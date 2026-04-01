#include "vtkWindowCube.h"
#include "ui_vtkWindowCube.h"

#include "ColorMaps.h"
#include "CubeViewController.h"
#include "LUTCustomizerDialog.h"
#include "MomentMapComputeTask.h"
#include "ProfileWidget.h"
#include "app/BackendClient.h"
#include "vtkFITSReader.h"
#include "vtkInteractorStyleProfile.h"
#include "vtkLegendScaleActorWCS.h"
#include "vtkMomentMapFilter.h"
#include "wcs.h"

#include <QVTKInteractor.h>
#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkColorTransferFunction.h>
#include <vtkCoordinate.h>
#include <vtkExtractVOI.h>
#include <vtkFloatArray.h>
#include <vtkFlyingEdges2D.h>
#include <vtkFlyingEdges3D.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkImageProperty.h>
#include <vtkImageReslice.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageThreshold.h>
#include <vtkInteractorStyleImage.h>
#include <vtkLookupTable.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkOutlineFilter.h>
#include <vtkPlaneSource.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkScalarBarActor.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkIdTypeArray.h>
#include <vtkTrivialProducer.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

#include <QActionGroup>
#include <QApplication>
#include <QColorDialog>
#include <QDebug>
#include <QDoubleValidator>
#include <QElapsedTimer>
#include <QInputDialog>
#include <QLabel>
#include <QList>
#include <QSignalBlocker>
#include <QtConcurrentRun>

#include <cmath>
#include <cstring>
#include <limits>
#include <sstream>

using namespace Qt::StringLiterals;

namespace {
constexpr double pi = 3.14159265358979323846;

vtkSmartPointer<vtkImageData> createPlaceholderImageData()
{
    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
    image->SetExtent(0, 0, 0, 0, 0, 0);
    image->AllocateScalars(VTK_FLOAT, 1);
    image->SetScalarComponentFromFloat(0, 0, 0, 0, 0.f);
    return image;
}

bool validBounds(const double bounds[6])
{
    return std::isfinite(bounds[0]) && std::isfinite(bounds[1]) && std::isfinite(bounds[2])
            && std::isfinite(bounds[3]) && std::isfinite(bounds[4]) && std::isfinite(bounds[5])
            && bounds[0] <= bounds[1] && bounds[2] <= bounds[3] && bounds[4] <= bounds[5];
}

void refitParallelSliceCamera(vtkRenderer *renderer, vtkImageData *sliceImage, vtkRenderWindow *win)
{
    if (!renderer || !sliceImage || !win) {
        return;
    }

    double bounds[6];
    sliceImage->GetBounds(bounds);
    if (!validBounds(bounds)) {
        return;
    }

    auto *camera = renderer->GetActiveCamera();
    double position[3];
    double focalPoint[3];
    camera->GetPosition(position);
    camera->GetFocalPoint(focalPoint);

    const double center[3] = { 0.5 * (bounds[0] + bounds[1]), 0.5 * (bounds[2] + bounds[3]),
                               0.5 * (bounds[4] + bounds[5]) };
    const double offset[3] = { position[0] - focalPoint[0], position[1] - focalPoint[1],
                               position[2] - focalPoint[2] };

    camera->SetFocalPoint(center);
    camera->SetPosition(center[0] + offset[0], center[1] + offset[1], center[2] + offset[2]);

    const int *size = win->GetSize();
    const double aspect = (size && size[1] > 0) ? static_cast<double>(size[0]) / size[1] : 1.0;
    const double width = std::max(1e-6, bounds[1] - bounds[0]);
    const double height = std::max(1e-6, bounds[3] - bounds[2]);
    constexpr double margin = 1.1;
    camera->SetParallelScale(
            margin * std::max(height * 0.5, width * 0.5 / std::max(1.0, aspect)));
}

void refitCubeCamera(vtkRenderer *renderer, vtkImageData *cubeImage)
{
    if (!renderer || !cubeImage) {
        return;
    }

    double bounds[6];
    cubeImage->GetBounds(bounds);
    if (!validBounds(bounds)) {
        return;
    }

    auto *camera = renderer->GetActiveCamera();
    double position[3];
    double focalPoint[3];
    camera->GetPosition(position);
    camera->GetFocalPoint(focalPoint);

    double direction[3] = { position[0] - focalPoint[0], position[1] - focalPoint[1],
                            position[2] - focalPoint[2] };
    double distance = std::sqrt(direction[0] * direction[0] + direction[1] * direction[1]
                                + direction[2] * direction[2]);
    if (distance <= 1e-6) {
        direction[0] = 0.;
        direction[1] = 0.;
        direction[2] = 1.;
        distance = 1.;
    } else {
        direction[0] /= distance;
        direction[1] /= distance;
        direction[2] /= distance;
    }

    const double center[3] = { 0.5 * (bounds[0] + bounds[1]), 0.5 * (bounds[2] + bounds[3]),
                               0.5 * (bounds[4] + bounds[5]) };
    const double dx = bounds[1] - bounds[0];
    const double dy = bounds[3] - bounds[2];
    const double dz = bounds[5] - bounds[4];
    const double radius = std::max(1e-6, 0.5 * std::sqrt(dx * dx + dy * dy + dz * dz));
    const double viewAngleRad = std::max(1e-3, camera->GetViewAngle() * pi / 180.);
    const double fitDistance = radius / std::sin(viewAngleRad * 0.5);
    const double finalDistance = std::max(distance, fitDistance);

    camera->SetFocalPoint(center);
    camera->SetPosition(center[0] + direction[0] * finalDistance,
                        center[1] + direction[1] * finalDistance,
                        center[2] + direction[2] * finalDistance);
}

AsyncIsosurfaceResult computeIsosurface(vtkSmartPointer<vtkImageData> image, double isoValue,
                                        int requestId)
{
    QElapsedTimer timer;
    timer.start();
    AsyncIsosurfaceResult result;
    result.requestId = requestId;

    if (!image) {
        return result;
    }

    vtkNew<vtkFlyingEdges3D> filter;
    filter->SetInputData(image);
    filter->SetValue(0, isoValue);
    filter->ComputeNormalsOff();
    filter->ComputeGradientsOff();
    filter->Update();

    vtkSmartPointer<vtkPolyData> mesh = vtkSmartPointer<vtkPolyData>::New();
    mesh->ShallowCopy(filter->GetOutput());
    result.mesh = mesh;
    qDebug().noquote()
            << QStringLiteral("[perf][isosurface] compute: %1 ms").arg(timer.elapsed());
    return result;
}

vtkSmartPointer<vtkImageData> decodeRemoteVolume(const QByteArray &data, int width, int height, int depth)
{
    const qsizetype expectedBytes = static_cast<qsizetype>(width) * height
            * static_cast<qsizetype>(depth) * static_cast<qsizetype>(sizeof(float));
    if (width <= 0 || height <= 0 || depth <= 0 || data.size() != expectedBytes) {
        return nullptr;
    }

    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
    image->SetExtent(0, width - 1, 0, height - 1, 0, depth - 1);
    image->SetOrigin(0., 0., 0.);
    image->SetSpacing(1., 1., 1.);
    image->AllocateScalars(VTK_FLOAT, 1);
    std::memcpy(image->GetScalarPointer(), data.constData(), static_cast<std::size_t>(expectedBytes));
    return image;
}

vtkSmartPointer<vtkImageData> decodeRemoteSlice(const QByteArray &data, int width, int height)
{
    const qsizetype expectedBytes =
            static_cast<qsizetype>(width) * height * static_cast<qsizetype>(sizeof(float));
    if (width <= 0 || height <= 0 || data.size() != expectedBytes) {
        return nullptr;
    }

    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
    image->SetExtent(0, width - 1, 0, height - 1, 0, 0);
    image->SetOrigin(0., 0., 0.);
    image->SetSpacing(1., 1., 1.);
    image->AllocateScalars(VTK_FLOAT, 1);
    std::memcpy(image->GetScalarPointer(), data.constData(), static_cast<std::size_t>(expectedBytes));
    return image;
}

void computeVolumeStats(vtkImageData *image, std::array<double, 2> &range, double &mean, double &rms)
{
    range = { 0., 0. };
    mean = 0.;
    rms = 0.;
    if (!image) {
        return;
    }

    double scalarRange[2];
    image->GetScalarRange(scalarRange);
    range = { scalarRange[0], scalarRange[1] };

    int extent[6];
    image->GetExtent(extent);
    const auto voxelCount = static_cast<qsizetype>(extent[1] - extent[0] + 1)
            * static_cast<qsizetype>(extent[3] - extent[2] + 1)
            * static_cast<qsizetype>(extent[5] - extent[4] + 1);
    if (voxelCount <= 0) {
        return;
    }

    const auto *values = static_cast<const float *>(image->GetScalarPointer());
    double sum = 0.;
    double sumSq = 0.;
    qsizetype finiteCount = 0;
    for (qsizetype i = 0; i < voxelCount; ++i) {
        const double value = values[i];
        if (!std::isfinite(value)) {
            continue;
        }
        sum += value;
        sumSq += value * value;
        ++finiteCount;
    }

    if (finiteCount <= 0) {
        return;
    }

    mean = sum / static_cast<double>(finiteCount);
    rms = std::sqrt(sumSq / static_cast<double>(finiteCount));
}

vtkSmartPointer<vtkPolyData> decodeRemoteIsosurface(const QByteArray &pointsData,
                                                    const QByteArray &polysData, int numPoints,
                                                    int numPolys)
{
    if (numPoints <= 0 || numPolys <= 0) {
        return vtkSmartPointer<vtkPolyData>::New();
    }

    const qsizetype expectedPointsBytes =
            static_cast<qsizetype>(numPoints) * 3 * static_cast<qsizetype>(sizeof(float));
    const qsizetype expectedPolysBytes =
            static_cast<qsizetype>(numPolys) * 4 * static_cast<qsizetype>(sizeof(qint32));
    if (pointsData.size() != expectedPointsBytes || polysData.size() != expectedPolysBytes) {
        return nullptr;
    }

    vtkNew<vtkFloatArray> pointArray;
    pointArray->SetNumberOfComponents(3);
    pointArray->SetNumberOfTuples(numPoints);
    std::memcpy(pointArray->GetVoidPointer(0), pointsData.constData(),
                static_cast<std::size_t>(expectedPointsBytes));

    vtkNew<vtkPoints> points;
    points->SetData(pointArray);

    const auto *rawPolys = reinterpret_cast<const qint32 *>(polysData.constData());
    vtkNew<vtkIdTypeArray> legacyCells;
    legacyCells->SetNumberOfValues(static_cast<vtkIdType>(numPolys) * 4);
    for (vtkIdType i = 0; i < legacyCells->GetNumberOfValues(); ++i) {
        legacyCells->SetValue(i, static_cast<vtkIdType>(rawPolys[i]));
    }

    vtkNew<vtkCellArray> cells;
    cells->ImportLegacyFormat(legacyCells);

    vtkSmartPointer<vtkPolyData> mesh = vtkSmartPointer<vtkPolyData>::New();
    mesh->SetPoints(points);
    mesh->SetPolys(cells);
    mesh->BuildCells();
    mesh->BuildLinks();
    return mesh;
}

RemoteCubePreviewResult fetchRemotePreview(const QString &backendUrl, const QString &datasetId,
                                           int downsample)
{
    RemoteCubePreviewResult result;
    BackendClient client(backendUrl);
    const auto response = client.requestPreview(datasetId, downsample);
    if (!response.valid) {
        result.errorMessage = response.error.isEmpty() ? u"Remote preview request failed."_s
                                                       : response.error;
        return result;
    }

    if (response.scalarType != u"float32"_s) {
        result.errorMessage = u"Unsupported remote preview scalar type."_s;
        return result;
    }

    result.cubeImageData = decodeRemoteVolume(response.data, response.width, response.height,
                                              response.depth);
    if (!result.cubeImageData) {
        result.errorMessage = u"Invalid remote preview payload."_s;
        return result;
    }

    result.valid = true;
    result.cubeRange = { response.rangeMin, response.rangeMax };
    result.dataExtent = { 0, response.width - 1, 0, response.height - 1, 0, response.depth - 1 };
    return result;
}

RemoteCubeSliceResult fetchRemoteSlice(const QString &backendUrl, const QString &datasetId,
                                       int index)
{
    RemoteCubeSliceResult result;
    result.index = index;

    BackendClient client(backendUrl);
    const auto response = client.requestSlice(datasetId, QStringLiteral("z"), index);
    if (!response.valid) {
        result.errorMessage = response.error.isEmpty() ? u"Remote slice request failed."_s
                                                       : response.error;
        return result;
    }

    if (response.scalarType != u"float32"_s) {
        result.errorMessage = u"Unsupported remote slice scalar type."_s;
        return result;
    }

    result.imageData = decodeRemoteSlice(response.data, response.width, response.height);
    if (!result.imageData) {
        result.errorMessage = u"Invalid remote slice payload."_s;
        return result;
    }

    result.valid = true;
    result.imageRange = { response.rangeMin, response.rangeMax };
    return result;
}

RemoteCubeSubvolumeResult fetchRemoteSubvolume(const QString &backendUrl, const QString &datasetId,
                                               const std::array<int, 6> &roi)
{
    RemoteCubeSubvolumeResult result;

    BackendClient client(backendUrl);
    const auto response = client.requestSubvolume(datasetId, roi[0], roi[1], roi[2], roi[3], roi[4],
                                                  roi[5]);
    if (!response.valid) {
        result.errorMessage =
                response.error.isEmpty() ? u"Remote subvolume request failed."_s : response.error;
        return result;
    }

    if (response.scalarType != u"float32"_s) {
        result.errorMessage = u"Unsupported remote subvolume scalar type."_s;
        return result;
    }

    result.cubeImageData = decodeRemoteVolume(response.data, response.width, response.height,
                                              response.depth);
    if (!result.cubeImageData) {
        result.errorMessage = u"Invalid remote subvolume payload."_s;
        return result;
    }

    result.valid = true;
    result.dataExtent = { 0, response.width - 1, 0, response.height - 1, 0, response.depth - 1 };
    computeVolumeStats(result.cubeImageData, result.cubeRange, result.cubeMean, result.cubeRms);
    return result;
}

AsyncIsosurfaceResult fetchRemoteIsosurface(const QString &backendUrl, const QString &datasetId,
                                            double isoValue, int requestId)
{
    AsyncIsosurfaceResult result;
    result.requestId = requestId;

    BackendClient client(backendUrl);
    const auto response = client.requestIsosurface(datasetId, isoValue);
    qDebug().noquote()
            << QStringLiteral("[remote-iso] response valid=%1 error=%2 num_points=%3 num_polys=%4")
                       .arg(response.valid)
                       .arg(response.error)
                       .arg(response.numPoints)
                       .arg(response.numPolys);
    qDebug().noquote()
            << QStringLiteral("[remote-iso] decode points bytes=%1 polys bytes=%2")
                       .arg(response.pointsData.size())
                       .arg(response.polysData.size());
    if (!response.valid) {
        result.errorMessage =
                response.error.isEmpty() ? u"Remote isocontour request failed."_s : response.error;
        return result;
    }

    result.mesh = decodeRemoteIsosurface(response.pointsData, response.polysData, response.numPoints,
                                         response.numPolys);
    if (!result.mesh) {
        result.errorMessage = u"Invalid remote isocontour payload."_s;
        return result;
    }
    qDebug().noquote()
            << QStringLiteral("[remote-iso] mesh points=%1 polys=%2")
                       .arg(result.mesh->GetNumberOfPoints())
                       .arg(result.mesh->GetNumberOfPolys());
    return result;
}
}

vtkWindowCube::vtkWindowCube(const QString &filepath, QWidget *parent)
    : vtkWindowCube(filepath, {}, {}, 0, 0, 0, { 1.0, 1.0, 1.0 }, { 0.0, 0.0, 0.0 }, parent)
{
}

vtkWindowCube::vtkWindowCube(const QString &filepath, const QString &backendUrl,
                             const QString &datasetId, int remoteWidth, int remoteHeight,
                             int remoteDepth, const std::array<double, 3> &remoteSpacing,
                             const std::array<double, 3> &remoteOrigin, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::vtkWindowCube),
      filepath(filepath),
      isRemoteMode(!datasetId.isEmpty()),
      remoteBackendUrl(backendUrl),
      remoteDatasetId(datasetId),
      remoteDatasetWidth(remoteWidth),
      remoteDatasetHeight(remoteHeight),
      remoteDatasetDepth(remoteDepth),
      remoteDatasetSpacing(remoteSpacing),
      remoteDatasetOrigin(remoteOrigin),
      astro(this->isRemoteMode ? nullptr : std::make_unique<AstroUtils>(filepath.toStdString())),
      lutCustomizer(nullptr),
      profileWidget(nullptr),
      level(15)
{
    ui->setupUi(this);
    this->setWindowTitle(this->filepath);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->cubeOpenStateLabel = new QLabel(this);
    this->cubeOpenStateLabel->setStyleSheet(u"QLabel { font-weight: 600; padding-left: 8px; }"_s);
    this->cubeOpenStateLabel->hide();
    this->statusBar()->addPermanentWidget(this->cubeOpenStateLabel);
    this->statusMessageClearTimer.setSingleShot(true);
    QObject::connect(&this->statusMessageClearTimer, &QTimer::timeout, this, [this]() {
        this->persistentStatusActive = false;
        this->statusBar()->clearMessage();
    });
    this->remoteSliceDebounceTimer.setSingleShot(true);
    QObject::connect(&this->remoteSliceDebounceTimer, &QTimer::timeout, this, [this]() {
        if (!this->isRemoteMode) {
            return;
        }

        this->requestRemoteSlice(this->pendingRemoteSliceIndex);
    });
    this->remoteFullResolutionStateTimer.setSingleShot(true);
    QObject::connect(&this->remoteFullResolutionStateTimer, &QTimer::timeout, this, [this]() {
        if (!this->isRemoteMode || this->remoteCubeDisplayState != RemoteCubeDisplayState::LoadingFullResolution
            || !this->remoteHighResCubeWatcher.isRunning()) {
            return;
        }

        this->setCubeOpenStateLabel(u"Loading full resolution..."_s);
    });
    this->isosurfaceDebounceTimer.setSingleShot(true);
    QObject::connect(&this->isosurfaceDebounceTimer, &QTimer::timeout, this, [this]() {
        qDebug().noquote()
                << QStringLiteral("[remote-iso] debounce fired checked=%1 remote=%2")
                           .arg(ui->actionIsosurface->isChecked())
                           .arg(this->isRemoteMode);
        if (this->cubeOpenWatcher.isRunning() || !ui->actionIsosurface->isChecked()) {
            return;
        }

        qDebug().noquote()
                << QStringLiteral("[remote-iso] launching remote compute threshold=%1")
                           .arg(ui->lineThreshold->text());
        this->startAsyncIsosurface(ui->lineThreshold->text().toDouble());
    });

    const CubeOpenStageResult preview = this->isRemoteMode ? CubeOpenStageResult { }
                                                           : loadCubeOpenPreview(this->filepath);
    const bool usingPreview = !this->isRemoteMode && preview.valid && preview.cubeImageData
            && preview.momentImageData;

    if (this->isRemoteMode) {
        vtkNew<vtkImageData> placeholder;
        placeholder->SetExtent(0, 0, 0, 0, 0, 0);
        placeholder->AllocateScalars(VTK_FLOAT, 1);
        static_cast<float *>(placeholder->GetScalarPointer())[0] = 0.f;
        this->cubeDisplaySource->SetOutput(placeholder);
        this->lowerBound = 0.f;
        this->upperBound = 1.f;
    } else if (usingPreview) {
        this->cubeDisplaySource->SetOutput(preview.cubeImageData);
        this->momentDisplaySource->SetOutput(preview.momentImageData);
        this->lowerBound = 3.f * preview.cubeRms;
        this->upperBound = preview.cubeRange[1];
    } else {
        this->reader->SetFileName(this->filepath.toUtf8());
        this->reader->Update();
        this->cubeDisplaySource->SetOutput(this->reader->GetOutput());
        this->lowerBound = 3.f * this->reader->GetRMS();
        this->upperBound = this->reader->GetMax();
    }

    // Setup Renderers
    this->setupCubeRenderer();
    this->setupSliceRenderer();
    this->setupMomentRenderer();
    QTimer::singleShot(0, this, [this]() {
        if (!this->isVisible()) {
            return;
        }

        QElapsedTimer timer;
        timer.start();
        ui->vtkCube->renderWindow()->Render();
        qDebug().noquote()
                << QStringLiteral("[perf][cube] warm-up render: %1 ms").arg(timer.elapsed());
    });
    if (usingPreview) {
        this->applyCubeOpenResult(preview);
    } else if (!this->isRemoteMode) {
        this->applyCubeOpenResult({ true,
                                    { },
                                    this->reader->GetOutput(),
                                    this->moment->GetOutput(),
                                    { this->reader->GetMin(), this->reader->GetMax() },
                                    { this->moment->GetOutput()->GetScalarRange()[0],
                                      this->moment->GetOutput()->GetScalarRange()[1] },
                                    { this->reader->GetDataExtent()[0], this->reader->GetDataExtent()[1],
                                      this->reader->GetDataExtent()[2], this->reader->GetDataExtent()[3],
                                      this->reader->GetDataExtent()[4], this->reader->GetDataExtent()[5] },
                                    this->reader->GetMean(),
                                    this->reader->GetRMS() });
    }
    if (!this->isRemoteMode) {
        this->viewController = std::make_unique<CubeViewController>(CubeViewContext {
            this->cubeDisplaySource,
            *this->astro,
            ui->vtkCube->renderWindow()->GetRenderers()->GetFirstRenderer(),
            this->isosurface,
            this->volume,
            this->isosurfaceFilter,
            this->volumeOpacity,
            this->slice,
            this->sliceOnCube,
            this->lutSlice,
            this->lutSliceOnCube,
            this->contours,
            this->contoursActor,
            this->moment,
            this->lutMoment,
            this->legendSlice,
            this->legendMoment
        });
    }
    QObject::connect(&this->cubeOpenWatcher, &QFutureWatcher<CubeOpenStageResult>::finished, this,
                     [this]() {
                         this->setCubeOpenActionsEnabled(true);

                         const auto result = this->cubeOpenWatcher.result();
                         if (!result.valid || !result.cubeImageData) {
                             this->setCubeOpenStateLabel(u"Preview"_s);
                             if (!result.errorMessage.isEmpty()) {
                                 this->persistentStatusActive = false;
                                 this->statusMessageClearTimer.stop();
                                 this->statusBar()->showMessage(result.errorMessage);
                             } else {
                                 this->clearPersistentStatusMessage();
                             }
                             return;
                         }

                         this->setCubeOpenStateLabel(u"Applying full resolution..."_s);
                         this->showPersistentStatusMessage(u"Applying full resolution..."_s);
                         QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                         QElapsedTimer applyTimer;
                         applyTimer.start();
                         this->applyCubeOpenResult(result);
                         qDebug().noquote()
                                 << QStringLiteral("[perf][cube] apply full result: %1 ms")
                                            .arg(applyTimer.elapsed());
                         ++this->currentFullCubeGeneration;
                         this->scheduleIsosurfacePrewarm();
                         this->setCubeOpenStateLabel({ });
                         this->clearPersistentStatusMessage();
                     });
    QObject::connect(&this->remotePreviewWatcher, &QFutureWatcher<RemoteCubePreviewResult>::finished, this,
                     [this]() {
                         const int requestId =
                                 this->remotePreviewWatcher.property("requestId").toInt();
                         if (requestId != this->currentRemotePreviewRequestId) {
                             return;
                         }

                         this->setCubeOpenActionsEnabled(true);

                         const auto result = this->remotePreviewWatcher.result();
                         if (!result.valid || !result.cubeImageData) {
                             this->persistentStatusActive = false;
                             this->statusMessageClearTimer.stop();
                             this->statusBar()->showMessage(result.errorMessage.isEmpty()
                                                                    ? u"Could not load remote preview."_s
                                                                    : result.errorMessage);
                             return;
                         }

                         const double span = result.cubeRange[1] - result.cubeRange[0];
                         this->lowerBound = static_cast<float>(result.cubeRange[0] + 0.1 * span);
                         this->upperBound = static_cast<float>(result.cubeRange[1]);
                         this->applyCubeOpenResult({ true,
                                                     { },
                                                     result.cubeImageData,
                                                     nullptr,
                                                     result.cubeRange,
                                                     { 0., 0. },
                                                     result.dataExtent,
                                                     0.,
                                                     0. });
                         this->setRemoteCubeDisplayState(RemoteCubeDisplayState::Preview);
                         qDebug().noquote() << QStringLiteral("[remote-plane] preview valid");
                         if (!this->requestHighResCube()) {
                             this->setRemoteCubeDisplayState(this->usingHighResCube
                                                                     ? RemoteCubeDisplayState::FullResolution
                                                                     : RemoteCubeDisplayState::Preview);
                             this->clearPersistentStatusMessage();
                         }
                     });
    QObject::connect(&this->remoteHighResCubeWatcher,
                     &QFutureWatcher<RemoteCubeSubvolumeResult>::finished, this, [this]() {
                         const int requestId =
                                 this->remoteHighResCubeWatcher.property("requestId").toInt();
                         if (requestId != this->currentRemoteHighResRequestId) {
                             return;
                         }

                         const auto result = this->remoteHighResCubeWatcher.result();
                         if (!result.valid || !result.cubeImageData) {
                             this->persistentStatusActive = false;
                             this->statusMessageClearTimer.stop();
                             this->setRemoteCubeDisplayState(RemoteCubeDisplayState::Preview);
                             this->statusBar()->showMessage(result.errorMessage.isEmpty()
                                                                    ? u"Could not load remote high-resolution cube."_s
                                                                    : result.errorMessage);
                             return;
                         }

                         this->usingHighResCube = true;
                         this->applyCubeOpenResult({ true,
                                                     { },
                                                     result.cubeImageData,
                                                     nullptr,
                                                     result.cubeRange,
                                                     { 0., 0. },
                                                     result.dataExtent,
                                                     result.cubeMean,
                                                     result.cubeRms });
                         if (auto *cubeImage = vtkImageData::SafeDownCast(
                                     this->cubeDisplaySource->GetOutputDataObject(0))) {
                             double bounds[6];
                             cubeImage->GetBounds(bounds);
                             qDebug().noquote()
                                     << QStringLiteral("[remote-roi] applied bounds=%1,%2,%3,%4,%5,%6")
                                                .arg(bounds[0], 0, 'g', 12)
                                                .arg(bounds[1], 0, 'g', 12)
                                                .arg(bounds[2], 0, 'g', 12)
                                                .arg(bounds[3], 0, 'g', 12)
                                                .arg(bounds[4], 0, 'g', 12)
                                                .arg(bounds[5], 0, 'g', 12);
                         }
                         this->setRemoteCubeDisplayState(RemoteCubeDisplayState::FullResolution);
                         this->clearPersistentStatusMessage();
                     });
    QObject::connect(&this->momentComputeWatcher, &QFutureWatcher<MomentMapComputeResult>::finished,
                     this, [this]() {
                         const int requestId =
                                 this->momentComputeWatcher.property("requestId").toInt();
                         if (requestId != this->currentMomentRequestId) {
                             return;
                         }

                         this->setMomentActionsEnabled(true);

                         const auto result = this->momentComputeWatcher.result();
                         if (!result.valid || !result.imageData) {
                             if (!result.errorMessage.isEmpty()) {
                                 this->persistentStatusActive = false;
                                 this->statusMessageClearTimer.stop();
                                 this->statusBar()->showMessage(result.errorMessage);
                             } else {
                                 this->clearPersistentStatusMessage();
                             }
                             return;
                         }

                         QElapsedTimer applyTimer;
                         applyTimer.start();
                         this->applyMomentMapResult(
                                 { result.imageData, { result.imageRange[0], result.imageRange[1] } });
                         qDebug().noquote()
                                 << QStringLiteral("[perf][moment] UI apply: %1 ms")
                                            .arg(applyTimer.elapsed());
                         this->clearPersistentStatusMessage();
                     });
    QObject::connect(&this->isosurfaceWatcher, &QFutureWatcher<AsyncIsosurfaceResult>::finished,
                     this, [this]() {
                         const auto result = this->isosurfaceWatcher.result();
                         if (result.requestId != this->currentIsosurfaceRequestId) {
                             return;
                         }

                         if (!result.mesh || result.mesh->GetNumberOfPoints() == 0) {
                             if (!result.errorMessage.isEmpty()) {
                                 this->persistentStatusActive = false;
                                 this->statusMessageClearTimer.stop();
                                 this->statusBar()->showMessage(result.errorMessage);
                             }
                             this->remoteIsosurfaceReady = false;
                             this->setCubeRenderModeLocally(false);
                             ui->actionVolume->setChecked(true);
                             ui->vtkCube->renderWindow()->Render();
                             return;
                         }
                         this->applyIsosurfaceResult(result);
                         this->clearPersistentStatusMessage();
                     });

    // Setup menu Camera
    ui->actionCameraFront->setIcon(QIcon(u":/icons/PIC_FRONT.png"_s));
    ui->actionCameraBack->setIcon(QIcon(u":/icons/PIC_BACK.png"_s));
    ui->actionCameraTop->setIcon(QIcon(u":/icons/PIC_TOP.png"_s));
    ui->actionCameraRight->setIcon(QIcon(u":/icons/PIC_RIGHT.png"_s));
    ui->actionCameraBottom->setIcon(QIcon(u":/icons/PIC_BOTTOM.png"_s));
    ui->actionCameraLeft->setIcon(QIcon(u":/icons/PIC_LEFT.png"_s));
    QObject::connect(ui->actionCameraFront, &QAction::triggered, this,
                     &vtkWindowCube::resetCameraFront);
    QObject::connect(ui->actionCameraBack, &QAction::triggered, this,
                     &vtkWindowCube::resetCameraBack);
    QObject::connect(ui->actionCameraTop, &QAction::triggered, this,
                     &vtkWindowCube::resetCameraTop);
    QObject::connect(ui->actionCameraRight, &QAction::triggered, this,
                     &vtkWindowCube::resetCameraRight);
    QObject::connect(ui->actionCameraBottom, &QAction::triggered, this,
                     &vtkWindowCube::resetCameraBottom);
    QObject::connect(ui->actionCameraLeft, &QAction::triggered, this,
                     &vtkWindowCube::resetCameraLeft);

    // Setup menu Edit
    QObject::connect(ui->actionEditLUT, &QAction::triggered, this,
                     &vtkWindowCube::showLUTCustomizer);

    // Setup menu View
    auto imageGroup = new QActionGroup(this);
    imageGroup->addAction(ui->actionSlice);
    imageGroup->addAction(ui->actionMomentMap);
    QObject::connect(imageGroup, &QActionGroup::triggered, this,
                     &vtkWindowCube::changeImageRenderer);
    QObject::connect(imageGroup, &QActionGroup::triggered, this,
                     &vtkWindowCube::updateLUTCustomizer);
    auto cubeGroup = new QActionGroup(this);
    cubeGroup->addAction(ui->actionIsosurface);
    cubeGroup->addAction(ui->actionVolume);
    QObject::connect(cubeGroup, &QActionGroup::triggered, this, &vtkWindowCube::changeCubeRender);

    // Setup menu Moment
    QObject::connect(ui->actionMoment0, &QAction::triggered, this,
                     [this]() { this->setMomentOrder(0); });
    QObject::connect(ui->actionMoment1, &QAction::triggered, this,
                     [this]() { this->setMomentOrder(1); });
    QObject::connect(ui->actionMoment2, &QAction::triggered, this,
                     [this]() { this->setMomentOrder(2); });
    QObject::connect(ui->actionMoment6, &QAction::triggered, this,
                     [this]() { this->setMomentOrder(6); });
    QObject::connect(ui->actionMoment8, &QAction::triggered, this,
                     [this]() { this->setMomentOrder(8); });
    QObject::connect(ui->actionMoment10, &QAction::triggered, this,
                     [this]() { this->setMomentOrder(10); });

    // Setup menu WCS
    auto groupWCS = new QActionGroup(this);
    groupWCS->addAction(ui->actionGalactic);
    groupWCS->addAction(ui->actionFK5);
    groupWCS->addAction(ui->actionEcliptic);
    ui->menuWCS->setEnabled(this->astro && !this->astro->isSimulation());
    QObject::connect(groupWCS, &QActionGroup::triggered, this, &vtkWindowCube::changeLegendWCS);

    // Setup menu Tools
    QObject::connect(ui->actionExtractSpectrum, &QAction::triggered, this,
                     &vtkWindowCube::setInteractorStyleProfile);

    // Setup Threshold UI
    const std::string bunit = this->astro ? this->astro->getPhysicalUnit() : std::string {};
    if (!bunit.empty()) {
        ui->groupThreshold->setTitle(u"Threshold (%1)"_s.arg(QString::fromStdString(bunit)));
    }
    ui->lineThreshold->setText(QString::number(this->lowerBound));
    ui->lineThreshold->setValidator(new QDoubleValidator(ui->lineThreshold));
    ui->btnCubeColor->setIcon(QIcon(u":/icons/COLORIZE.png"_s));
    QObject::connect(ui->lineThreshold, &QLineEdit::editingFinished, this,
                     &vtkWindowCube::thresholdLineChanged);
    QObject::connect(ui->sliderThreshold, &QSlider::actionTriggered, this,
                     &vtkWindowCube::thresholdSliderChanged);
    QObject::connect(ui->btnCubeColor, &QPushButton::clicked, this,
                     &vtkWindowCube::changeCubeColor);

    // Setup Slice UI
    const std::string unit = this->astro ? this->astro->getAxisUnit(2) : std::string {};
    if (!unit.empty()) {
        ui->groupSlice->setTitle(u"Cutting plane (%1)"_s.arg(QString::fromStdString(unit)));
    }
    ui->lineSpectral->setText(this->astro ? QString::number(this->astro->getInitialSpectralValue())
                                          : QString::number(this->remoteSliceCoordinate(0)));
    QObject::connect(ui->sliderSlice, &QSlider::actionTriggered, this,
                     &vtkWindowCube::sliceSliderChanged);
    QObject::connect(ui->spinSlice, &QSpinBox::valueChanged, this,
                     &vtkWindowCube::sliceSpinChanged);
    if (this->isRemoteMode) {
        const int maxSliceValue = std::max(1, this->remoteSliceCount());
        ui->sliderSlice->setMinimum(1);
        ui->spinSlice->setMinimum(1);
        ui->sliderSlice->setMaximum(maxSliceValue);
        ui->spinSlice->setMaximum(maxSliceValue);
        ui->sliderSlice->setValue(1);
        ui->spinSlice->setValue(1);
    }

    // Setup Contours UI
    ui->lineLevel->setText(QString::number(this->level));
    ui->lineLevel->setValidator(new QIntValidator(ui->lineLevel));
    ui->lineLowerBound->setText(QString::number(this->lowerBound));
    ui->lineLowerBound->setValidator(new QDoubleValidator(ui->lineLowerBound));
    ui->lineUpperBound->setText(QString::number(this->upperBound));
    ui->lineUpperBound->setValidator(new QDoubleValidator(ui->lineUpperBound));
    QObject::connect(ui->checkContours, &QCheckBox::checkStateChanged, this,
                     &vtkWindowCube::updateContoursVisibility);
    QObject::connect(ui->lineLevel, &QLineEdit::editingFinished, this,
                     &vtkWindowCube::updateContours);
    QObject::connect(ui->lineLowerBound, &QLineEdit::editingFinished, this,
                     &vtkWindowCube::updateContours);
    QObject::connect(ui->lineUpperBound, &QLineEdit::editingFinished, this,
                     &vtkWindowCube::updateContours);

    // Setup Statistics UI
    if (this->isRemoteMode) {
        ui->lineCubeMin->clear();
        ui->lineCubeMax->clear();
        ui->lineCubeMean->clear();
        ui->lineCubeRms->clear();
        this->setCubeOpenActionsEnabled(false);
        ui->actionIsosurface->setEnabled(true);
        ui->actionIsosurface->setChecked(false);
        ui->actionVolume->setChecked(true);
        this->setCubeRenderModeLocally(false);
        this->setRemoteCubeDisplayState(RemoteCubeDisplayState::Preview);
        this->showPersistentStatusMessage(u"Loading remote preview..."_s);
        this->remotePreviewWatcher.setProperty("requestId", ++this->currentRemotePreviewRequestId);
        this->remotePreviewWatcher.setFuture(
                QtConcurrent::run(&fetchRemotePreview, this->remoteBackendUrl, this->remoteDatasetId, 4));
    } else if (usingPreview) {
        ui->lineCubeMin->setText(QString::number(preview.cubeRange[0]));
        ui->lineCubeMax->setText(QString::number(preview.cubeRange[1]));
        ui->lineCubeMean->setText(QString::number(preview.cubeMean));
        ui->lineCubeRms->setText(QString::number(preview.cubeRms));
        this->setCubeOpenActionsEnabled(false);
        this->setCubeOpenStateLabel(u"Preview"_s);
        this->showPersistentStatusMessage(u"Loading full resolution..."_s);
        this->cubeOpenWatcher.setFuture(QtConcurrent::run(&loadCubeOpenFull, this->filepath));
    } else {
        ui->lineCubeMin->setText(QString::number(this->reader->GetMin()));
        ui->lineCubeMax->setText(QString::number(this->reader->GetMax()));
        ui->lineCubeMean->setText(QString::number(this->reader->GetMean()));
        ui->lineCubeRms->setText(QString::number(this->reader->GetRMS()));
    }
}

vtkWindowCube::~vtkWindowCube()
{
    delete ui;
}

void vtkWindowCube::closeEvent(QCloseEvent *event)
{
    if (this->isBusy()) {
        if (this->cubeOpenWatcher.isRunning() || this->remotePreviewWatcher.isRunning()
            || this->remoteHighResCubeWatcher.isRunning()) {
            this->showPersistentStatusMessage(this->isRemoteMode ? u"Loading remote preview..."_s
                                                                : u"Loading full resolution..."_s);
        } else if (this->activeRemoteSliceRequests > 0) {
            this->showPersistentStatusMessage(u"Loading remote slice..."_s);
        } else if (this->activeRemoteIsosurfaceRequests > 0 || this->isosurfaceWatcher.isRunning()) {
            this->showPersistentStatusMessage(u"Computing isocontour..."_s);
        } else {
            this->showPersistentStatusMessage(u"Computing moment..."_s);
        }
        event->ignore();
        return;
    }

    QMainWindow::closeEvent(event);
}

void vtkWindowCube::setupCubeRenderer()
{
    vtkNew<vtkRenderer> ren;
    ren->SetBackground(0.21, 0.23, 0.25);

    vtkNew<vtkGenericOpenGLRenderWindow> win;
    win->AddRenderer(ren);
    ui->vtkCube->setRenderWindow(win);
    ui->vtkCube->setEnableTouchEventProcessing(false);

    // Isosurface
    this->isosurfaceFilter->SetInputConnection(this->cubeDisplaySource->GetOutputPort());
    this->isosurfaceFilter->SetValue(0, this->lowerBound);
    vtkNew<vtkPolyDataMapper> isosurfaceMapper;
    isosurfaceMapper->SetInputConnection(this->isosurfaceFilter->GetOutputPort());
    isosurfaceMapper->ScalarVisibilityOff();
    this->isosurface->SetMapper(isosurfaceMapper);
    this->isosurface->GetProperty()->SetColor(1., 0.5, 1.);
    if (!this->isRemoteMode) {
        ren->AddViewProp(this->isosurface);
        this->currentIsosurfaceActor = this->isosurface;
    }

    // Volume
    const auto cubeImage = vtkImageData::SafeDownCast(this->cubeDisplaySource->GetOutputDataObject(0));
    double cubeRange[2] = { 0., 0. };
    if (cubeImage) {
        cubeImage->GetScalarRange(cubeRange);
    }
    vtkNew<vtkLookupTable> lutVolume;
    lutVolume->SetTableRange(cubeRange);
    ColorMaps::SetColorMap(lutVolume);
    ColorMaps::SetColorTransferFunction(lutVolume, this->volumeColorTransferFunction);
    this->volumeColorTransferFunction->SetObjectName(lutVolume->GetObjectName());
    this->volumeOpacity->AddPoint(cubeRange[0], 0.0);
    this->volumeOpacity->AddPoint(this->lowerBound, 0.05);
    this->volumeOpacity->AddPoint(cubeRange[1], 0.3);
    vtkNew<vtkVolumeProperty> volumeProperty;
    volumeProperty->SetColor(this->volumeColorTransferFunction);
    volumeProperty->SetScalarOpacity(this->volumeOpacity);
    volumeProperty->SetInterpolationTypeToLinear();
    vtkNew<vtkImageThreshold> nanMask;
    nanMask->SetInputConnection(this->cubeDisplaySource->GetOutputPort());
    nanMask->ThresholdBetween(cubeRange[0], cubeRange[1]);
    nanMask->SetOutValue(0.0);
    nanMask->SetInValue(255.0);
    nanMask->SetOutputScalarTypeToUnsignedChar();
    nanMask->Update();
    vtkNew<vtkGPUVolumeRayCastMapper> volumeMapper;
    volumeMapper->SetInputConnection(this->cubeDisplaySource->GetOutputPort());
    volumeMapper->SetMaskInput(nanMask->GetOutput());
    volumeMapper->SetMaskTypeToBinary();
    this->volume->SetMapper(volumeMapper);
    this->volume->SetProperty(volumeProperty);
    // By default, we show the isosurface

    // Outline
    vtkNew<vtkOutlineFilter> outline;
    outline->SetInputConnection(this->cubeDisplaySource->GetOutputPort());
    vtkNew<vtkPolyDataMapper> outlineMapper;
    outlineMapper->SetInputConnection(outline->GetOutputPort());
    vtkNew<vtkActor> outlineActor;
    outlineActor->SetMapper(outlineMapper);
    ren->AddViewProp(outlineActor);

    if (this->isRemoteMode) {
        vtkNew<vtkPolyDataMapper> planeMapper;
        planeMapper->SetInputConnection(this->remoteCuttingPlaneSource->GetOutputPort());
        planeMapper->ScalarVisibilityOff();
        this->remoteCuttingPlaneActor->SetMapper(planeMapper);
        this->remoteCuttingPlaneActor->GetProperty()->SetColor(0.1, 1.0, 0.1);
        this->remoteCuttingPlaneActor->GetProperty()->SetOpacity(0.55);
        this->remoteCuttingPlaneActor->GetProperty()->EdgeVisibilityOn();
        this->remoteCuttingPlaneActor->GetProperty()->SetEdgeColor(1.0, 0.0, 0.0);
        this->remoteCuttingPlaneActor->GetProperty()->SetLineWidth(3.0);
        this->remoteCuttingPlaneActor->GetProperty()->LightingOff();
        this->remoteCuttingPlaneActor->GetProperty()->BackfaceCullingOff();
        this->remoteCuttingPlaneActor->GetProperty()->FrontfaceCullingOff();
        this->remoteCuttingPlaneActor->VisibilityOn();
        ren->AddViewProp(this->remoteCuttingPlaneActor);
        qDebug().noquote()
                << QStringLiteral("[remote-plane] actor added renderer=%1 actor=%2 visible=%3")
                           .arg(reinterpret_cast<quintptr>(ren.GetPointer()), 0, 16)
                           .arg(reinterpret_cast<quintptr>(this->remoteCuttingPlaneActor.GetPointer()),
                                0, 16)
                           .arg(this->remoteCuttingPlaneActor->GetVisibility());
    }

    // Slice overlay in cube renderer is local-only.
    if (!this->isRemoteMode) {
        int extent[6] = { 0, -1, 0, -1, 0, -1 };
        if (cubeImage) {
            cubeImage->GetExtent(extent);
        }
        extent[4] = extent[5] = 0;
        this->sliceOnCube->SetInputConnection(this->cubeDisplaySource->GetOutputPort());
        this->sliceOnCube->SetVOI(extent);
        this->sliceOnCube->Update();
        this->lutSliceOnCube->SetTableRange(this->sliceOnCube->GetOutput()->GetScalarRange());
        this->lutSliceOnCube->SetNanColor(1., 1., 1., 1.);
        ColorMaps::SetColorMap(this->lutSliceOnCube);
        vtkNew<vtkImageMapToColors> colors;
        colors->SetInputConnection(this->sliceOnCube->GetOutputPort());
        colors->SetLookupTable(this->lutSliceOnCube);
        vtkNew<vtkImageSliceMapper> sliceMapper;
        sliceMapper->SetInputConnection(colors->GetOutputPort());
        vtkNew<vtkImageSlice> sliceActor;
        sliceActor->SetMapper(sliceMapper);
        sliceActor->GetProperty()->SetInterpolationTypeToNearest();
        ren->AddViewProp(sliceActor);
    }

    // Axes
    vtkNew<vtkAxesActor> axes;
    this->axesWidget->SetOrientationMarker(axes);
    this->axesWidget->SetInteractor(win->GetInteractor());
    this->axesWidget->SetEnabled(1);
    this->axesWidget->InteractiveOff();

    ren->ResetCamera();
    win->Render();

    ren->GetActiveCamera()->GetPosition(this->initialCameraPosition);
    ren->GetActiveCamera()->GetFocalPoint(this->initialCameraFocalPoint);
}

void vtkWindowCube::setupSliceRenderer()
{
    vtkNew<vtkRenderer> ren;
    ren->SetBackground(0.21, 0.23, 0.25);
    ren->GetActiveCamera()->ParallelProjectionOn();

    this->sliceWin->AddRenderer(ren);
    ui->vtkImage->setRenderWindow(this->sliceWin);
    ui->vtkImage->setEnableTouchEventProcessing(false);

    vtkNew<vtkInteractorStyleImage> style;
    this->sliceWin->GetInteractor()->SetInteractorStyle(style);
    this->sliceWin->GetInteractor()->AddObserver(vtkCommand::MouseMoveEvent, this,
                                                 &vtkWindowCube::mouseCallback);
    this->coordinate->SetCoordinateSystemToDisplay();
    this->coordinate->SetViewport(ren);

    // Slice
    if (this->isRemoteMode) {
        this->lutSlice->SetTableRange(0., 0.);
    } else {
        this->slice->SetInputConnection(this->cubeDisplaySource->GetOutputPort());
        this->slice->SetResliceAxesOrigin(0., 0., 0.);
        this->slice->SetOutputDimensionality(2);
        this->slice->Update();
        this->lutSlice->SetTableRange(this->slice->GetOutput()->GetScalarRange());
    }
    this->lutSlice->SetNanColor(1., 1., 1., 1.);
    ColorMaps::SetColorMap(this->lutSlice);
    if (this->isRemoteMode) {
        auto *img = vtkImageData::SafeDownCast(this->remoteSliceDisplaySource->GetOutputDataObject(0));
        if (img) {
            this->sliceColors->SetInputData(img);
        } else {
            this->sliceColors->SetInputData(createPlaceholderImageData());
        }
    } else {
        auto *img = this->slice->GetOutput();
        if (img) {
            this->sliceColors->SetInputData(img);
        } else {
            this->sliceColors->SetInputData(createPlaceholderImageData());
        }
    }
    this->sliceColors->SetLookupTable(this->lutSlice);
    vtkNew<vtkImageSliceMapper> sliceMapper;
    sliceMapper->SetInputConnection(this->sliceColors->GetOutputPort());
    sliceMapper->BorderOn();
    vtkNew<vtkImageSlice> sliceActor;
    sliceActor->SetMapper(sliceMapper);
    sliceActor->GetProperty()->SetInterpolationTypeToNearest();
    ren->AddViewProp(sliceActor);

    // Color bar
    vtkNew<vtkScalarBarActor> colorbar;
    colorbar->SetLookupTable(this->lutSlice);
    colorbar->SetMaximumWidthInPixels(100);
    colorbar->SetPosition(0.9, 0.1);
    ren->AddViewProp(colorbar);

    // Contours
    this->contours->SetInputConnection(this->isRemoteMode ? this->remoteSliceDisplaySource->GetOutputPort()
                                                          : this->slice->GetOutputPort());
    this->contours->GenerateValues(this->level, this->lowerBound, this->upperBound);
    vtkNew<vtkPolyDataMapper> contoursMapper;
    contoursMapper->SetInputConnection(this->contours->GetOutputPort());
    contoursMapper->SetScalarRange(this->lowerBound, this->upperBound);
    this->contoursActor->SetMapper(contoursMapper);
    this->contoursActor->VisibilityOff();
    ren->AddViewProp(this->contoursActor);

    // Legend
    if (this->astro) {
        this->legendSlice->Init(this->filepath.toStdString());
        this->legendSlice->SetWCS(WCS_GALACTIC);
        ren->AddViewProp(this->legendSlice);
    }

    ren->ResetCamera();
    this->sliceWin->Render();
}

void vtkWindowCube::setupMomentRenderer()
{
    vtkNew<vtkRenderer> ren;
    ren->SetBackground(0.21, 0.23, 0.25);
    ren->GetActiveCamera()->ParallelProjectionOn();
    this->momentWin->AddRenderer(ren);

    vtkNew<QVTKInteractor> iren;
    this->momentWin->SetInteractor(iren);
    iren->Initialize();
    iren->AddObserver(vtkCommand::MouseMoveEvent, this, &vtkWindowCube::mouseCallback);

    vtkNew<vtkInteractorStyleImage> style;
    iren->SetInteractorStyle(style);

    // Moment
    if (!vtkImageData::SafeDownCast(this->momentDisplaySource->GetOutputDataObject(0))
        && !this->isRemoteMode) {
        this->moment->SetInputConnection(this->reader->GetOutputPort());
        this->moment->Init(this->filepath.toStdString());
        this->moment->Update();
        this->momentDisplaySource->SetOutput(this->moment->GetOutput());
    }

    const auto momentImage =
            vtkImageData::SafeDownCast(this->momentDisplaySource->GetOutputDataObject(0));
    if (momentImage) {
        this->lutMoment->SetTableRange(momentImage->GetScalarRange());
    } else {
        this->lutMoment->SetTableRange(0., 0.);
    }
    this->lutMoment->SetNanColor(1., 1., 1., 1.);
    ColorMaps::SetColorMap(this->lutMoment);
    if (momentImage) {
        this->momentColors->SetInputData(momentImage);
    } else {
        this->momentColors->SetInputData(createPlaceholderImageData());
    }
    this->momentColors->SetLookupTable(this->lutMoment);
    vtkNew<vtkImageSliceMapper> momentMapper;
    momentMapper->SetInputConnection(this->momentColors->GetOutputPort());
    momentMapper->BorderOn();
    vtkNew<vtkImageSlice> momentActor;
    momentActor->SetMapper(momentMapper);
    momentActor->GetProperty()->SetInterpolationTypeToNearest();
    ren->AddViewProp(momentActor);

    // Color bar
    vtkNew<vtkScalarBarActor> colorbar;
    colorbar->SetLookupTable(this->lutMoment);
    colorbar->SetMaximumWidthInPixels(100);
    colorbar->SetPosition(0.9, 0.1);
    ren->AddViewProp(colorbar);

    // Legend
    if (this->astro) {
        this->legendMoment->Init(this->filepath.toStdString());
        this->legendMoment->SetWCS(WCS_GALACTIC);
        ren->AddViewProp(this->legendMoment);
    }

    ren->ResetCamera();
}

void vtkWindowCube::resetCubeCamera()
{
    auto ren = ui->vtkCube->renderWindow()->GetRenderers()->GetFirstRenderer();
    auto camera = ren->GetActiveCamera();
    camera->SetPosition(this->initialCameraPosition);
    camera->SetFocalPoint(this->initialCameraFocalPoint);
    camera->SetViewUp(0., 1., 0.);
    ren->ResetCamera();
}

void vtkWindowCube::setCameraAzimuth(double az)
{
    this->resetCubeCamera();
    ui->vtkCube->renderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Azimuth(az);
    ui->vtkCube->renderWindow()->Render();
}

void vtkWindowCube::setCameraElevation(double el)
{
    this->resetCubeCamera();
    ui->vtkCube->renderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Elevation(
            el);
    ui->vtkCube->renderWindow()->Render();
}

void vtkWindowCube::mouseCallback()
{
    if (this->isBusy()) {
        return;
    }

    const int *position = ui->vtkImage->renderWindow()->GetInteractor()->GetEventPosition();
    this->coordinate->SetValue(position[0], position[1]);
    const double *worldCoord = this->coordinate->GetComputedWorldValue(nullptr);
    const long imageCoord[2] = { std::lround(worldCoord[0]), std::lround(worldCoord[1]) };

    const auto imageData = this->viewingSlice()
            ? (this->isRemoteMode
                       ? vtkImageData::SafeDownCast(this->remoteSliceDisplaySource->GetOutputDataObject(0))
                       : this->slice->GetOutput())
            : vtkImageData::SafeDownCast(this->momentDisplaySource->GetOutputDataObject(0));
    if (!imageData) {
        this->clearPersistentStatusMessage();
        return;
    }

    int extent[6];
    imageData->GetExtent(extent);
    if (imageCoord[0] < extent[0] || imageCoord[0] > extent[1] || imageCoord[1] < extent[2]
        || imageCoord[1] > extent[3]) {
        this->clearPersistentStatusMessage();
        return;
    }

    std::ostringstream ss;
    ss << "<value> ";
    if (this->viewingSlice()) {
        const float val = imageData->GetScalarComponentAsFloat(imageCoord[0], imageCoord[1], 0, 0);
        ss << val;
    } else {
        const float val = imageData->GetScalarComponentAsFloat(imageCoord[0], imageCoord[1], 0, 0);
        ss << val;
    }

    ss << "  <image> X: " << worldCoord[0] << " Y: " << worldCoord[1];

    if (this->astro && !this->astro->isSimulation()) {
        double wcs[2];
        this->astro->xy2sky(worldCoord, wcs, WCS_GALACTIC);
        ss << "  <galactic> GLON: " << wcs[0] << " GLAT: " << wcs[1];

        this->astro->xy2sky(worldCoord, wcs, WCS_J2000);
        ss << "  <fk5> RA: " << wcs[0] << " Dec: " << wcs[1];

        this->astro->xy2sky(worldCoord, wcs, WCS_ECLIPTIC);
        ss << "  <ecliptic> ELON: " << wcs[0] << " ELAT: " << wcs[1];
    }

    this->statusBar()->showMessage(QString::fromStdString(ss.str()));
}

bool vtkWindowCube::viewingIsosurface() const
{
    if (!this->currentIsosurfaceActor) {
        return false;
    }

    return ui->vtkCube->renderWindow()->GetRenderers()->GetFirstRenderer()->HasViewProp(
            this->currentIsosurfaceActor);
}

bool vtkWindowCube::viewingSlice() const
{
    return ui->vtkImage->renderWindow() == this->sliceWin;
}

void vtkWindowCube::updateCube()
{
    const double threshold = ui->lineThreshold->text().toDouble();
    if (this->isRemoteMode) {
        auto *cubeData = vtkImageData::SafeDownCast(this->cubeDisplaySource->GetOutputDataObject(0));
        if (cubeData) {
            double cubeRange[2];
            cubeData->GetScalarRange(cubeRange);
            this->isosurfaceFilter->SetValue(0, threshold);
            this->volumeOpacity->RemoveAllPoints();
            this->volumeOpacity->AddPoint(cubeRange[0], 0.0);
            this->volumeOpacity->AddPoint(threshold, 0.05);
            this->volumeOpacity->AddPoint(cubeRange[1], 0.3);
        }
    } else {
        this->viewController->updateCube(threshold);
    }
    if (!this->cubeOpenWatcher.isRunning() && ui->actionIsosurface->isChecked()) {
        this->scheduleIsosurfaceRecompute();
    }
    ui->vtkCube->renderWindow()->Render();
}

std::array<int, 6> vtkWindowCube::computeVisibleROI() const
{
    const auto computeAxisRoi = [](int size) -> std::array<int, 2> {
        const int maxIndex = std::max(0, size - 1);
        if (size <= 1) {
            return { 0, maxIndex };
        }

        const int minKeep = std::min(size, std::max(8, size / 2));
        const int keep = std::clamp(static_cast<int>(std::lround(size * 0.6)), minKeep, size);
        const int start = std::max(0, (size - keep) / 2);
        const int end = std::min(maxIndex, start + keep - 1);
        return { start, end };
    };

    const auto x = computeAxisRoi(this->remoteDatasetWidth);
    const auto y = computeAxisRoi(this->remoteDatasetHeight);
    const auto z = computeAxisRoi(this->remoteDatasetDepth);
    return { x[0], x[1], y[0], y[1], z[0], z[1] };
}

bool vtkWindowCube::requestHighResCube()
{
    if (!this->isRemoteMode || this->usingHighResCube || this->remoteHighResCubeWatcher.isRunning()) {
        return false;
    }

    const auto roi = this->computeVisibleROI();
    qDebug().noquote()
            << QStringLiteral("[remote-roi] request x=%1..%2 y=%3..%4 z=%5..%6")
                       .arg(roi[0])
                       .arg(roi[1])
                       .arg(roi[2])
                       .arg(roi[3])
                       .arg(roi[4])
                       .arg(roi[5]);
    this->setRemoteCubeDisplayState(RemoteCubeDisplayState::LoadingFullResolution);
    this->showPersistentStatusMessage(u"Loading full resolution..."_s);
    this->remoteHighResCubeWatcher.setProperty("requestId", ++this->currentRemoteHighResRequestId);
    this->remoteHighResCubeWatcher.setFuture(
            QtConcurrent::run(&fetchRemoteSubvolume, this->remoteBackendUrl, this->remoteDatasetId, roi));
    return true;
}

void vtkWindowCube::updateRemoteCuttingPlane(int sliceIndex)
{
    if (!this->isRemoteMode) {
        return;
    }

    auto *cubeImage = vtkImageData::SafeDownCast(this->cubeDisplaySource->GetOutputDataObject(0));
    if (!cubeImage) {
        qDebug().noquote() << QStringLiteral("[remote-plane] preview valid=0");
        this->remoteCuttingPlaneActor->VisibilityOff();
        return;
    }

    int extent[6];
    cubeImage->GetExtent(extent);
    if (extent[0] > extent[1] || extent[2] > extent[3] || extent[4] > extent[5]) {
        qDebug().noquote()
                << QStringLiteral("[remote-plane] invalid extent=%1,%2,%3,%4,%5,%6")
                           .arg(extent[0])
                           .arg(extent[1])
                           .arg(extent[2])
                           .arg(extent[3])
                           .arg(extent[4])
                           .arg(extent[5]);
        this->remoteCuttingPlaneActor->VisibilityOff();
        return;
    }

    const int clampedSlice = this->clampRemoteSliceIndex(sliceIndex);
    double bounds[6];
    cubeImage->GetBounds(bounds);

    double z = 0.5 * (bounds[4] + bounds[5]);
    const int realDepth = this->remoteSliceCount();
    if (realDepth > 1) {
        const double fraction =
                static_cast<double>(clampedSlice) / static_cast<double>(realDepth - 1);
        z = bounds[4] + fraction * (bounds[5] - bounds[4]);
    }

    this->remoteCuttingPlaneSource->SetOrigin(bounds[0], bounds[2], z);
    this->remoteCuttingPlaneSource->SetPoint1(bounds[1], bounds[2], z);
    this->remoteCuttingPlaneSource->SetPoint2(bounds[0], bounds[3], z);
    this->remoteCuttingPlaneSource->Modified();
    this->remoteCuttingPlaneSource->Update();
    if (auto *mapper = this->remoteCuttingPlaneActor->GetMapper()) {
        mapper->Modified();
    }
    this->remoteCuttingPlaneActor->Modified();
    this->remoteCuttingPlaneActor->VisibilityOn();
    qDebug().noquote()
            << QStringLiteral("[remote-plane] update z=%1 bounds=%2,%3,%4,%5,%6,%7 visible=%8")
                       .arg(z, 0, 'g', 12)
                       .arg(bounds[0], 0, 'g', 12)
                       .arg(bounds[1], 0, 'g', 12)
                       .arg(bounds[2], 0, 'g', 12)
                       .arg(bounds[3], 0, 'g', 12)
                       .arg(bounds[4], 0, 'g', 12)
                       .arg(bounds[5], 0, 'g', 12)
                       .arg(this->remoteCuttingPlaneActor->GetVisibility());
    ui->vtkCube->renderWindow()->Render();
    qDebug().noquote() << QStringLiteral("[remote-plane] render triggered");
}

void vtkWindowCube::applyCubeOpenResult(const CubeOpenStageResult &result)
{
    if (!result.cubeImageData) {
        return;
    }

    QElapsedTimer totalTimer;
    totalTimer.start();

    this->cubeDisplaySource->SetOutput(result.cubeImageData);
    this->cubeDisplaySource->Modified();
    result.cubeImageData->Modified();
    if (result.momentImageData) {
        this->momentDisplaySource->SetOutput(result.momentImageData);
        this->momentDisplaySource->Modified();
        this->lutMoment->SetTableRange(result.momentRange[0], result.momentRange[1]);
    }
    if (auto *mapper = this->volume->GetMapper()) {
        mapper->Modified();
    }
    if (this->currentIsosurfaceActor && this->currentIsosurfaceActor->GetMapper()) {
        this->currentIsosurfaceActor->GetMapper()->Modified();
    }

    const int clampedSlice = this->isRemoteMode
            ? this->clampRemoteSliceIndex(ui->spinSlice->value() - 1)
            : std::clamp(ui->spinSlice->value() - 1, result.dataExtent[4], result.dataExtent[5]);
    qDebug().noquote()
            << QStringLiteral("[perf][cube] apply clamp slice index: %1 ms").arg(
                       totalTimer.elapsed());

    QElapsedTimer sliderSyncTimer;
    sliderSyncTimer.start();
    {
        const QSignalBlocker blockSlider(ui->sliderSlice);
        const QSignalBlocker blockSpin(ui->spinSlice);
        const int maxSliceValue = this->isRemoteMode ? std::max(1, this->remoteSliceCount())
                                                     : (result.dataExtent[5] + 1);
        ui->sliderSlice->setMaximum(maxSliceValue);
        ui->spinSlice->setMaximum(maxSliceValue);
        ui->sliderSlice->setValue(clampedSlice + 1);
        ui->spinSlice->setValue(clampedSlice + 1);
    }
    qDebug().noquote()
            << QStringLiteral("[perf][cube] apply slider/spin sync: %1 ms").arg(
                       sliderSyncTimer.elapsed());

    double sliceRange[2] = { 0., 0. };
    if (!this->isRemoteMode) {
        int sliceExtent[6] = { result.dataExtent[0], result.dataExtent[1], result.dataExtent[2],
                               result.dataExtent[3], clampedSlice, clampedSlice };
        QElapsedTimer sliceOnCubeSetTimer;
        sliceOnCubeSetTimer.start();
        this->sliceOnCube->SetVOI(sliceExtent);
        qDebug().noquote()
                << QStringLiteral("[perf][cube] apply sliceOnCube SetVOI: %1 ms").arg(
                           sliceOnCubeSetTimer.elapsed());

        QElapsedTimer sliceOnCubeUpdateTimer;
        sliceOnCubeUpdateTimer.start();
        this->sliceOnCube->Update();
        qDebug().noquote()
                << QStringLiteral("[perf][cube] apply sliceOnCube update: %1 ms").arg(
                           sliceOnCubeUpdateTimer.elapsed());

        QElapsedTimer sliceSetTimer;
        sliceSetTimer.start();
        this->slice->SetResliceAxesOrigin(0., 0., clampedSlice);
        qDebug().noquote()
                << QStringLiteral("[perf][cube] apply slice origin set: %1 ms").arg(
                           sliceSetTimer.elapsed());

        QElapsedTimer sliceUpdateTimer;
        sliceUpdateTimer.start();
        this->slice->Update();
        qDebug().noquote()
                << QStringLiteral("[perf][cube] apply slice update: %1 ms").arg(
                           sliceUpdateTimer.elapsed());

        QElapsedTimer lutSyncTimer;
        lutSyncTimer.start();
        const double *localSliceRange = this->slice->GetOutput()->GetScalarRange();
        sliceRange[0] = localSliceRange[0];
        sliceRange[1] = localSliceRange[1];
        this->lutSlice->SetTableRange(localSliceRange);

        const double *sliceOnCubeRange = this->sliceOnCube->GetOutput()->GetScalarRange();
        this->lutSliceOnCube->SetTableRange(sliceOnCubeRange);
        qDebug().noquote()
                << QStringLiteral("[perf][cube] apply LUT sync: %1 ms").arg(lutSyncTimer.elapsed());
    }

    QElapsedTimer cubeFieldsTimer;
    cubeFieldsTimer.start();
    ui->lineCubeMin->setText(QString::number(result.cubeRange[0]));
    ui->lineCubeMax->setText(QString::number(result.cubeRange[1]));
    ui->lineCubeMean->setText(QString::number(result.cubeMean));
    ui->lineCubeRms->setText(QString::number(result.cubeRms));
    ui->lineSpectral->setText(this->astro
                                      ? QString::number(this->astro->getInitialSpectralValue()
                                                        + this->astro->getIncrements()[2] * clampedSlice)
                                      : QString::number(this->remoteSliceCoordinate(clampedSlice)));
    qDebug().noquote()
            << QStringLiteral("[perf][cube] apply cube UI fields: %1 ms").arg(
                       cubeFieldsTimer.elapsed());
    qDebug().noquote()
            << QStringLiteral("[perf][cube] apply data+ui sync: %1 ms").arg(totalTimer.elapsed());

    QElapsedTimer cameraTimer;
    cameraTimer.start();
    auto cubeRenderer = ui->vtkCube->renderWindow()->GetRenderers()->GetFirstRenderer();
    QElapsedTimer cubeFitTimer;
    cubeFitTimer.start();
    refitCubeCamera(cubeRenderer, result.cubeImageData);
    cubeRenderer->ResetCameraClippingRange();
    qDebug().noquote()
            << QStringLiteral("[perf][cube] apply cube clipping sync: %1 ms").arg(
                       cubeFitTimer.elapsed());

    auto sliceRenderer = this->sliceWin->GetRenderers()->GetFirstRenderer();
    if (!this->isRemoteMode) {
        QElapsedTimer sliceFitTimer;
        sliceFitTimer.start();
        refitParallelSliceCamera(sliceRenderer, this->slice->GetOutput(), this->sliceWin);
        qDebug().noquote()
                << QStringLiteral("[perf][cube] apply slice fit: %1 ms").arg(sliceFitTimer.elapsed());
    }

    QElapsedTimer sliceClipTimer;
    sliceClipTimer.start();
    sliceRenderer->ResetCameraClippingRange();
    qDebug().noquote()
            << QStringLiteral("[perf][cube] apply slice clipping sync: %1 ms").arg(
                       sliceClipTimer.elapsed());

    auto momentRenderer = this->momentWin->GetRenderers()->GetFirstRenderer();
    QElapsedTimer momentClipTimer;
    momentClipTimer.start();
    momentRenderer->ResetCameraClippingRange();
    qDebug().noquote()
            << QStringLiteral("[perf][cube] apply moment clipping sync: %1 ms").arg(
                       momentClipTimer.elapsed());
    qDebug().noquote()
            << QStringLiteral("[perf][cube] apply camera sync: %1 ms").arg(cameraTimer.elapsed());

    QElapsedTimer imgFieldsTimer;
    imgFieldsTimer.start();
    if (this->viewingSlice() && !this->isRemoteMode) {
        ui->lineImgMin->setText(QString::number(sliceRange[0]));
        ui->lineImgMax->setText(QString::number(sliceRange[1]));
        this->updateLUTCustomizer();
    } else if (result.momentImageData) {
        ui->lineImgMin->setText(QString::number(result.momentRange[0]));
        ui->lineImgMax->setText(QString::number(result.momentRange[1]));
        this->updateLUTCustomizer();
    }
    qDebug().noquote()
            << QStringLiteral("[perf][cube] apply image UI fields: %1 ms").arg(
                       imgFieldsTimer.elapsed());

    this->setCubeRenderModeLocally(ui->actionIsosurface->isChecked());
    if (this->isRemoteMode) {
        this->updateRemoteCuttingPlane(clampedSlice);
    }
    cubeRenderer->Modified();
    ui->vtkCube->renderWindow()->Modified();

    QElapsedTimer renderTimer;
    renderTimer.start();
    ui->vtkCube->renderWindow()->Render();
    ui->vtkImage->renderWindow()->Render();
    QTimer::singleShot(0, this, [this]() { ui->vtkCube->renderWindow()->Render(); });
    if (this->isRemoteMode) {
        this->requestRemoteSlice(clampedSlice);
    }
    qDebug().noquote()
            << QStringLiteral("[perf][cube] apply render: %1 ms").arg(renderTimer.elapsed());
    qDebug().noquote() << QStringLiteral("[perf][cube] apply total: %1 ms").arg(
            totalTimer.elapsed());
}

void vtkWindowCube::updateSlice()
{
    const int slice = ui->spinSlice->value() - 1;
    if (this->isRemoteMode) {
        this->remoteSliceDebounceTimer.stop();
        this->requestRemoteSlice(slice);
        return;
    }

    const auto result = this->viewController->updateSlice(slice);
    if (!result.valid) {
        return;
    }

    ui->lineSpectral->setText(QString::number(result.spectralValue));

    ui->vtkCube->renderWindow()->Render();
    this->sliceWin->GetRenderers()->GetFirstRenderer()->ResetCamera();
    this->sliceWin->Render();

    if (this->viewingSlice()) {
        ui->lineImgMin->setText(QString::number(result.imageRange[0]));
        ui->lineImgMax->setText(QString::number(result.imageRange[1]));
        this->updateLUTCustomizer();
    }
}

void vtkWindowCube::requestRemoteSlice(int sliceIndex)
{
    sliceIndex = this->clampRemoteSliceIndex(sliceIndex);
    this->currentRequestedRemoteSliceIndex = sliceIndex;
    if (this->tryApplyCachedRemoteSlice(sliceIndex)) {
        return;
    }

    const QString key = this->remoteSliceCacheKey(sliceIndex);
    if (this->remoteSliceFetchesInFlight.contains(key)) {
        this->statusBar()->showMessage(u"Loading remote slice..."_s);
        return;
    }

    const int requestId = ++this->currentRemoteSliceRequestId;
    ++this->activeRemoteSliceRequests;
    this->statusBar()->showMessage(u"Loading remote slice..."_s);
    this->startRemoteSliceFetch(sliceIndex, false, requestId);
}

void vtkWindowCube::applyRemoteSliceResult(const RemoteCubeSliceResult &result)
{
    this->cacheRemoteSliceResult(result);
    this->remoteSliceDisplaySource->SetOutput(result.imageData);
    this->remoteSliceDisplaySource->Modified();
    auto *img = vtkImageData::SafeDownCast(this->remoteSliceDisplaySource->GetOutputDataObject(0));
    if (img) {
        this->sliceColors->SetInputData(img);
    } else {
        qWarning() << "[vtk] Expected vtkImageData for remote slice colors but got null";
        return;
    }
    this->lutSlice->SetTableRange(result.imageRange[0], result.imageRange[1]);

    const QSignalBlocker blockSlider(ui->sliderSlice);
    const QSignalBlocker blockSpin(ui->spinSlice);
    ui->sliderSlice->setValue(result.index + 1);
    ui->spinSlice->setValue(result.index + 1);
    ui->lineSpectral->setText(this->astro
                                      ? QString::number(this->astro->getInitialSpectralValue()
                                                        + this->astro->getIncrements()[2] * result.index)
                                      : QString::number(this->remoteSliceCoordinate(result.index)));
    this->updateRemoteCuttingPlane(result.index);
    ui->vtkCube->renderWindow()->Render();
    qDebug().noquote() << QStringLiteral("[remote-plane] render triggered");

    if (this->viewingSlice()) {
        ui->lineImgMin->setText(QString::number(result.imageRange[0]));
        ui->lineImgMax->setText(QString::number(result.imageRange[1]));
        this->updateLUTCustomizer();
    }

    auto *sliceRenderer = this->sliceWin->GetRenderers()->GetFirstRenderer();
    refitParallelSliceCamera(sliceRenderer, result.imageData, this->sliceWin);
    sliceRenderer->ResetCameraClippingRange();
    this->sliceWin->Render();
    this->prefetchNeighborRemoteSlices(result.index);
}

void vtkWindowCube::updateRemoteSliceDragFeedback(int sliceIndex)
{
    const int clampedSlice = this->clampRemoteSliceIndex(sliceIndex);
    this->currentRequestedRemoteSliceIndex = clampedSlice;
    ui->lineSpectral->setText(this->astro
                                      ? QString::number(this->astro->getInitialSpectralValue()
                                                        + this->astro->getIncrements()[2] * clampedSlice)
                                      : QString::number(this->remoteSliceCoordinate(clampedSlice)));
    this->updateRemoteCuttingPlane(clampedSlice);
    ui->vtkCube->renderWindow()->Render();
}

int vtkWindowCube::remoteSliceCount() const
{
    return std::max(1, this->remoteDatasetDepth);
}

int vtkWindowCube::clampRemoteSliceIndex(int sliceIndex) const
{
    return std::clamp(sliceIndex, 0, this->remoteSliceCount() - 1);
}

double vtkWindowCube::remoteSliceCoordinate(int sliceIndex) const
{
    const int clampedSlice = this->clampRemoteSliceIndex(sliceIndex);
    return this->remoteDatasetOrigin[2]
            + this->remoteDatasetSpacing[2] * static_cast<double>(clampedSlice);
}

QString vtkWindowCube::remoteSliceCacheKey(int sliceIndex) const
{
    return QStringLiteral("%1|z|%2")
            .arg(this->remoteDatasetId)
            .arg(this->clampRemoteSliceIndex(sliceIndex));
}

void vtkWindowCube::touchRemoteSliceCacheKey(const QString &key)
{
    this->remoteSliceCacheLru.removeAll(key);
    this->remoteSliceCacheLru.append(key);
}

void vtkWindowCube::cacheRemoteSliceResult(const RemoteCubeSliceResult &result)
{
    if (!result.valid || !result.imageData) {
        return;
    }

    const QString key = this->remoteSliceCacheKey(result.index);
    this->remoteSliceCache.insert(key, result);
    this->touchRemoteSliceCacheKey(key);
    while (this->remoteSliceCacheLru.size() > remoteSliceCacheCapacity) {
        const QString evictedKey = this->remoteSliceCacheLru.takeFirst();
        this->remoteSliceCache.remove(evictedKey);
    }
}

bool vtkWindowCube::tryApplyCachedRemoteSlice(int sliceIndex)
{
    const QString key = this->remoteSliceCacheKey(sliceIndex);
    const auto it = this->remoteSliceCache.constFind(key);
    if (it == this->remoteSliceCache.cend()) {
        return false;
    }

    this->touchRemoteSliceCacheKey(key);
    this->applyRemoteSliceResult(it.value());
    this->clearPersistentStatusMessage();
    return true;
}

void vtkWindowCube::startRemoteSliceFetch(int sliceIndex, bool isPrefetch, int requestId)
{
    sliceIndex = this->clampRemoteSliceIndex(sliceIndex);
    const QString key = this->remoteSliceCacheKey(sliceIndex);
    if (this->remoteSliceFetchesInFlight.contains(key)) {
        return;
    }

    this->remoteSliceFetchesInFlight.insert(key);
    auto *watcher = new QFutureWatcher<RemoteCubeSliceResult>(this);
    watcher->setProperty("requestId", requestId);
    watcher->setProperty("sliceKey", key);
    watcher->setProperty("isPrefetch", isPrefetch);
    QObject::connect(watcher, &QFutureWatcher<RemoteCubeSliceResult>::finished, this,
                     [this, watcher]() {
                         const auto result = watcher->result();
                         const int requestId = watcher->property("requestId").toInt();
                         const QString key = watcher->property("sliceKey").toString();
                         const bool isPrefetch = watcher->property("isPrefetch").toBool();
                         this->remoteSliceFetchesInFlight.remove(key);
                         if (!isPrefetch) {
                             --this->activeRemoteSliceRequests;
                         }
                         watcher->deleteLater();

                         if (!result.valid || !result.imageData) {
                             if (!isPrefetch && requestId == this->currentRemoteSliceRequestId) {
                                 this->persistentStatusActive = false;
                                 this->statusMessageClearTimer.stop();
                                 this->statusBar()->showMessage(result.errorMessage.isEmpty()
                                                                        ? u"Could not load remote slice."_s
                                                                        : result.errorMessage);
                             }
                             return;
                         }

                         this->cacheRemoteSliceResult(result);

                         const bool requestedSliceMatches = this->clampRemoteSliceIndex(result.index)
                                 == this->currentRequestedRemoteSliceIndex;
                         if (!isPrefetch && requestId != this->currentRemoteSliceRequestId) {
                             if (this->activeRemoteSliceRequests == 0) {
                                 this->clearPersistentStatusMessage();
                             }
                             return;
                         }

                         if (requestedSliceMatches) {
                             this->applyRemoteSliceResult(result);
                             if (!isPrefetch && this->activeRemoteSliceRequests == 0) {
                                 this->clearPersistentStatusMessage();
                             }
                         } else if (!isPrefetch && this->activeRemoteSliceRequests == 0) {
                             this->clearPersistentStatusMessage();
                         }
                     });
    watcher->setFuture(QtConcurrent::run(&fetchRemoteSlice, this->remoteBackendUrl,
                                         this->remoteDatasetId, sliceIndex));
}

void vtkWindowCube::prefetchNeighborRemoteSlices(int sliceIndex)
{
    if (!this->isRemoteMode) {
        return;
    }

    for (const int neighbor : { sliceIndex - 1, sliceIndex + 1 }) {
        if (neighbor < 0 || neighbor >= this->remoteSliceCount()) {
            continue;
        }

        const QString key = this->remoteSliceCacheKey(neighbor);
        if (this->remoteSliceCache.contains(key) || this->remoteSliceFetchesInFlight.contains(key)) {
            continue;
        }

        this->startRemoteSliceFetch(neighbor, true);
    }
}

void vtkWindowCube::updateContoursVisibility()
{
    if (this->isRemoteMode) {
        this->contoursActor->SetVisibility(ui->checkContours->isChecked());
    } else {
        this->viewController->setContoursVisible(ui->checkContours->isChecked());
    }
    this->sliceWin->Render();
}

void vtkWindowCube::setMomentOrder(int order)
{
    if (this->isBusy()) {
        return;
    }

    const int requestId = ++this->currentMomentRequestId;
    this->momentComputeWatcher.setProperty("requestId", requestId);
    this->setMomentActionsEnabled(false);
    this->showPersistentStatusMessage(u"Computing moment..."_s);
    this->momentComputeWatcher.setFuture(QtConcurrent::run(
            &computeMomentMap, MomentMapComputeRequest { this->filepath,
                                                        this->isRemoteMode ? this->remoteDatasetId : QString {},
                                                        this->isRemoteMode ? this->remoteBackendUrl : QString {},
                                                        order }));
}

void vtkWindowCube::showPersistentStatusMessage(const QString &text, int minDurationMs)
{
    this->statusMessageClearTimer.stop();
    this->persistentStatusActive = true;
    this->statusMessageMinDurationMs = minDurationMs;
    this->statusMessageElapsed.restart();
    this->statusBar()->showMessage(text);
}

void vtkWindowCube::clearPersistentStatusMessage()
{
    if (!this->persistentStatusActive) {
        this->statusBar()->clearMessage();
        return;
    }

    const int remaining = this->statusMessageMinDurationMs - this->statusMessageElapsed.elapsed();
    if (remaining <= 0) {
        this->persistentStatusActive = false;
        this->statusBar()->clearMessage();
        return;
    }

    this->statusMessageClearTimer.start(remaining);
}

void vtkWindowCube::updateContours()
{
    if (this->isRemoteMode) {
        this->contours->GenerateValues(ui->lineLevel->text().toInt(),
                                       ui->lineLowerBound->text().toDouble(),
                                       ui->lineUpperBound->text().toDouble());
    } else {
        this->viewController->updateContours(ui->lineLevel->text().toInt(),
                                             ui->lineLowerBound->text().toDouble(),
                                             ui->lineUpperBound->text().toDouble());
    }
    this->sliceWin->Render();
}

void vtkWindowCube::thresholdSliderChanged(int action)
{
    Q_UNUSED(action);
    const int p = ui->sliderThreshold->sliderPosition();
    const float threshold = 0.01 * p * (this->upperBound - this->lowerBound) + this->lowerBound;
    ui->lineThreshold->setText(QString::number(threshold));
    this->updateCube();
}

void vtkWindowCube::thresholdLineChanged()
{
    const float threshold =
            std::clamp(ui->lineThreshold->text().toFloat(), this->lowerBound, this->upperBound);
    ui->lineThreshold->setText(QString::number(threshold));
    const int p = 100 * (threshold - this->lowerBound) / (this->upperBound - this->lowerBound);
    ui->sliderThreshold->setValue(p);
    this->updateCube();
}

void vtkWindowCube::sliceSliderChanged(int action)
{
    Q_UNUSED(action);
    if (this->cubeOpenWatcher.isRunning() || this->remotePreviewWatcher.isRunning()) {
        const QSignalBlocker blockSpin(ui->spinSlice);
        ui->spinSlice->setValue(ui->sliderSlice->sliderPosition());
        return;
    }

    const int sliderValue = ui->sliderSlice->sliderPosition();
    if (this->isRemoteMode && ui->sliderSlice->isSliderDown()) {
        const QSignalBlocker blockSpin(ui->spinSlice);
        ui->spinSlice->setValue(sliderValue);
        const int sliceIndex = sliderValue - 1;
        this->updateRemoteSliceDragFeedback(sliceIndex);
        if (this->tryApplyCachedRemoteSlice(sliceIndex)) {
            this->remoteSliceDebounceTimer.stop();
            return;
        }

        this->pendingRemoteSliceIndex = sliceIndex;
        this->remoteSliceDebounceTimer.start(remoteSliceDebounceDelayMs);
        return;
    }

    ui->spinSlice->setValue(sliderValue);
    // updateSlice is called by spinSlice
}

void vtkWindowCube::sliceSpinChanged(int value)
{
    if (this->cubeOpenWatcher.isRunning() || this->remotePreviewWatcher.isRunning()) {
        const QSignalBlocker blockSlider(ui->sliderSlice);
        ui->sliderSlice->setValue(value);
        return;
    }

    ui->sliderSlice->setValue(value);
    this->remoteSliceDebounceTimer.stop();
    this->updateSlice();
}

void vtkWindowCube::changeLegendWCS()
{
    if (!this->astro) {
        return;
    }

    const int wcs = (ui->actionGalactic->isChecked()
                             ? WCS_GALACTIC
                             : (ui->actionFK5->isChecked() ? WCS_J2000 : WCS_ECLIPTIC));
    this->viewController->setLegendWcs(wcs);
    ui->vtkImage->renderWindow()->Render();
}

void vtkWindowCube::showLUTCustomizer()
{
    if (!this->lutCustomizer) {
        this->lutCustomizer = new LUTCustomizerDialog(this);
        QObject::connect(this->lutCustomizer, &LUTCustomizerDialog::lutUpdated, this,
                         &vtkWindowCube::renderImage);
        QObject::connect(this->lutCustomizer, &LUTCustomizerDialog::lutUpdated, this,
                         &vtkWindowCube::syncSlicesLUT);
    }

    this->updateLUTCustomizer();
    this->lutCustomizer->show();
    this->lutCustomizer->raise();
    this->lutCustomizer->activateWindow();
}

void vtkWindowCube::updateLUTCustomizer()
{
    if (!this->lutCustomizer) {
        return;
    }

    if (this->viewingSlice()) {
        this->lutCustomizer->init(this->isRemoteMode
                                          ? vtkImageData::SafeDownCast(
                                                    this->remoteSliceDisplaySource->GetOutputDataObject(0))
                                          : this->slice->GetOutput(),
                                  this->lutSlice);
    } else {
        this->lutCustomizer->init(
                vtkImageData::SafeDownCast(this->momentDisplaySource->GetOutputDataObject(0)),
                this->lutMoment);
    }
}

void vtkWindowCube::applyMomentMapResult(const MomentMapApplyResult &result)
{
    if (!result.imageData) {
        return;
    }

    QElapsedTimer totalTimer;
    totalTimer.start();

    this->momentDisplaySource->SetOutput(result.imageData);
    auto *img = vtkImageData::SafeDownCast(this->momentDisplaySource->GetOutputDataObject(0));
    if (img) {
        this->momentColors->SetInputData(img);
    } else {
        qWarning() << "[vtk] Expected vtkImageData for moment colors but got null";
        return;
    }
    double currentRange[2];
    this->lutMoment->GetTableRange(currentRange);
    if (std::fabs(currentRange[0] - result.imageRange[0]) >= 1e-6
        || std::fabs(currentRange[1] - result.imageRange[1]) >= 1e-6) {
        this->lutMoment->SetTableRange(result.imageRange[0], result.imageRange[1]);
    }

    {
        const QSignalBlocker blockSliceAction(ui->actionSlice);
        const QSignalBlocker blockMomentAction(ui->actionMomentMap);
        ui->actionSlice->setChecked(false);
        ui->actionMomentMap->setChecked(true);
    }

    ui->vtkImage->setRenderWindow(this->momentWin);
    ui->labelImg->setText(u"Moment:"_s);
    this->coordinate->SetViewport(ui->vtkImage->renderWindow()->GetRenderers()->GetFirstRenderer());

    {
        const QSignalBlocker blockMin(ui->lineImgMin);
        const QSignalBlocker blockMax(ui->lineImgMax);
        ui->lineImgMin->setText(QString::number(result.imageRange[0]));
        ui->lineImgMax->setText(QString::number(result.imageRange[1]));
    }

    this->updateLUTCustomizer();
    qDebug().noquote()
            << QStringLiteral("[perf][moment] apply data+ui sync: %1 ms").arg(totalTimer.elapsed());
    QElapsedTimer renderTimer;
    renderTimer.start();
    ui->vtkImage->renderWindow()->Render();
    qDebug().noquote() << QStringLiteral("[perf][moment] render after apply: %1 ms").arg(
            renderTimer.elapsed());
    qDebug().noquote() << QStringLiteral("[perf][moment] apply total: %1 ms").arg(
            totalTimer.elapsed());
}

bool vtkWindowCube::isBusy() const
{
    return this->cubeOpenWatcher.isRunning() || this->remotePreviewWatcher.isRunning()
            || this->remoteHighResCubeWatcher.isRunning()
            || this->momentComputeWatcher.isRunning() || this->activeRemoteSliceRequests > 0
            || this->activeRemoteIsosurfaceRequests > 0 || this->isosurfaceWatcher.isRunning();
}

void vtkWindowCube::setRemoteCubeDisplayState(RemoteCubeDisplayState state)
{
    this->remoteCubeDisplayState = state;
    if (!this->isRemoteMode) {
        return;
    }

    switch (state) {
    case RemoteCubeDisplayState::Preview:
        this->remoteFullResolutionStateTimer.stop();
        this->setCubeOpenStateLabel(u"Preview"_s);
        break;
    case RemoteCubeDisplayState::LoadingFullResolution:
        this->remoteFullResolutionStateTimer.start(remoteLoadingStateDelayMs);
        break;
    case RemoteCubeDisplayState::FullResolution:
        this->remoteFullResolutionStateTimer.stop();
        this->setCubeOpenStateLabel(u"Full resolution"_s);
        break;
    }
}

void vtkWindowCube::setMomentActionsEnabled(bool enabled)
{
    ui->actionMoment0->setEnabled(enabled);
    ui->actionMoment1->setEnabled(enabled);
    ui->actionMoment2->setEnabled(enabled);
    ui->actionMoment6->setEnabled(enabled);
    ui->actionMoment8->setEnabled(enabled);
    ui->actionMoment10->setEnabled(enabled);
}

void vtkWindowCube::setCubeOpenActionsEnabled(bool enabled)
{
    this->setMomentActionsEnabled(enabled);
    ui->actionExtractSpectrum->setEnabled(enabled && !this->isRemoteMode);
}

void vtkWindowCube::setCubeOpenStateLabel(const QString &text)
{
    if (!this->cubeOpenStateLabel) {
        return;
    }

    if (text.isEmpty()) {
        this->cubeOpenStateLabel->hide();
        this->cubeOpenStateLabel->clear();
        return;
    }

    this->cubeOpenStateLabel->setText(text);
    this->cubeOpenStateLabel->show();
}

void vtkWindowCube::setInteractorStyleImage()
{
    vtkNew<vtkInteractorStyleImage> style;
    ui->vtkImage->renderWindow()->GetInteractor()->SetInteractorStyle(style);
    ui->vtkImage->renderWindow()->Render();
}

void vtkWindowCube::setInteractorStyleProfile()
{
    if (this->isRemoteMode) {
        return;
    }

    if (!this->profileWidget) {
        vtkNew<vtkInteractorStyleProfile> style;
        ui->vtkImage->renderWindow()->GetInteractor()->SetInteractorStyle(style);
        ui->vtkImage->renderWindow()->Render();

        this->profileWidget = new ProfileWidget(
                style, vtkImageData::SafeDownCast(this->cubeDisplaySource->GetOutputDataObject(0)),
                                                this->filepath.toStdString(), this);
        this->profileWidget->setupSpectrumPlot();
        QObject::connect(this->profileWidget, &ProfileWidget::destroyed, this,
                         &vtkWindowCube::setInteractorStyleImage, Qt::QueuedConnection);
    }
}

void vtkWindowCube::changeImageRenderer()
{
    double imgRange[2];

    if (ui->actionSlice->isChecked()) {
        ui->vtkImage->setRenderWindow(this->sliceWin);
        ui->labelImg->setText(u"Slice:"_s);
        auto *sliceImage = this->isRemoteMode
                ? vtkImageData::SafeDownCast(this->remoteSliceDisplaySource->GetOutputDataObject(0))
                : this->sliceOnCube->GetOutput();
        if (sliceImage) {
            sliceImage->GetScalarRange(imgRange);
        } else {
            imgRange[0] = 0.;
            imgRange[1] = 0.;
        }
    } else {
        ui->vtkImage->setRenderWindow(this->momentWin);
        ui->labelImg->setText(u"Moment:"_s);
        auto *momentImage = vtkImageData::SafeDownCast(this->momentDisplaySource->GetOutputDataObject(0));
        if (momentImage) {
            momentImage->GetScalarRange(imgRange);
        } else {
            imgRange[0] = 0.;
            imgRange[1] = 0.;
        }
    }

    ui->lineImgMin->setText(QString::number(imgRange[0]));
    ui->lineImgMax->setText(QString::number(imgRange[1]));
    this->coordinate->SetViewport(ui->vtkImage->renderWindow()->GetRenderers()->GetFirstRenderer());
    ui->vtkImage->renderWindow()->Render();
}

void vtkWindowCube::changeCubeRender()
{
    qDebug().noquote()
            << QStringLiteral("[remote-iso] changeCubeRender triggered checked=%1 remote=%2")
                       .arg(ui->actionIsosurface->isChecked())
                       .arg(this->isRemoteMode);
    if (this->isRemoteMode && ui->actionIsosurface->isChecked()) {
        this->scheduleIsosurfaceRecompute();
        this->setCubeRenderModeLocally(false);
        qDebug().noquote()
                << QStringLiteral("[remote-iso] keeping volume visible while waiting for mesh");
        ui->vtkCube->renderWindow()->Render();
        return;
    }
    this->setCubeRenderModeLocally(ui->actionIsosurface->isChecked());
    ui->vtkCube->renderWindow()->Render();
}

void vtkWindowCube::changeCubeColor()
{
    if (this->viewingIsosurface()) {
        double rgb[3];
        this->currentIsosurfaceActor->GetProperty()->GetColor(rgb);

        QColor color;
        color.setRgbF(rgb[0], rgb[1], rgb[2]);
        QColorDialog dialog(color, this);
        dialog.setOption(QColorDialog::ShowAlphaChannel, false);
        if (dialog.exec() == QDialog::Accepted) {
            const QColor selected = dialog.selectedColor();
            this->currentIsosurfaceActor->GetProperty()->SetColor(selected.redF(),
                                                                  selected.greenF(),
                                                                  selected.blueF());
            ui->vtkCube->renderWindow()->Render();
        }
    } else {
        const auto names = ColorMaps::GetColorMapNames();
        QStringList items;
        items.reserve(names.size());
        std::transform(names.cbegin(), names.cend(), std::back_inserter(items),
                       [](const std::string &name) { return QString::fromStdString(name); });

        const int idxCurrent = items.indexOf(this->volumeColorTransferFunction->GetObjectName());
        bool ok{ };
        const QString palette =
                QInputDialog::getItem(this, u"Select color palette"_s, u"Color palette:"_s, items,
                                      idxCurrent, false, &ok);
        if (ok && !palette.isEmpty()) {
            const auto cubeImage =
                    vtkImageData::SafeDownCast(this->cubeDisplaySource->GetOutputDataObject(0));
            double cubeRange[2] = { 0., 0. };
            if (cubeImage) {
                cubeImage->GetScalarRange(cubeRange);
            }
            vtkNew<vtkLookupTable> lut;
            lut->SetTableRange(cubeRange);
            ColorMaps::SetColorMap(lut, palette.toStdString());
            ColorMaps::SetColorTransferFunction(lut, this->volumeColorTransferFunction);
            this->volumeColorTransferFunction->SetObjectName(palette.toStdString());
            ui->vtkCube->renderWindow()->Render();
        }
    }
}

void vtkWindowCube::resetCameraFront()
{
    this->setCameraAzimuth(0.);
}

void vtkWindowCube::resetCameraBack()
{
    this->setCameraAzimuth(-180.);
}

void vtkWindowCube::resetCameraTop()
{
    this->setCameraElevation(90.);
}

void vtkWindowCube::resetCameraRight()
{
    this->setCameraAzimuth(90.);
}

void vtkWindowCube::resetCameraBottom()
{
    this->setCameraElevation(-90.);
}

void vtkWindowCube::resetCameraLeft()
{
    this->setCameraAzimuth(-90.);
}

void vtkWindowCube::renderImage()
{
    ui->vtkImage->renderWindow()->Render();
}

void vtkWindowCube::syncSlicesLUT()
{
    if (!this->isRemoteMode) {
        this->viewController->syncSlicesLut();
        ui->vtkCube->renderWindow()->Render();
    }
}

void vtkWindowCube::startAsyncIsosurface(double isoValue)
{
    if (this->isRemoteMode && this->remoteIsosurfaceRequestInFlight
        && std::fabs(this->inFlightRemoteIsosurfaceThreshold - isoValue) < 1e-9) {
        qDebug().noquote()
                << QStringLiteral("[remote-iso] skipping duplicate in-flight threshold=%1")
                           .arg(isoValue, 0, 'g', 12);
        return;
    }

    const int requestId = ++this->currentIsosurfaceRequestId;
    this->remoteIsosurfaceReady = false;
    qDebug().noquote()
            << QStringLiteral("[remote-iso] startAsyncIsosurface requestId=%1 threshold=%2 remote=%3")
                       .arg(requestId)
                       .arg(isoValue, 0, 'g', 12)
                       .arg(this->isRemoteMode);
    this->showPersistentStatusMessage(u"Computing isocontour..."_s);

    if (this->isRemoteMode) {
        ++this->activeRemoteIsosurfaceRequests;
        this->remoteIsosurfaceRequestInFlight = true;
        this->inFlightRemoteIsosurfaceThreshold = isoValue;
        auto *watcher = new QFutureWatcher<AsyncIsosurfaceResult>(this);
        watcher->setProperty("requestId", requestId);
        watcher->setProperty("isoValue", isoValue);
        QObject::connect(watcher, &QFutureWatcher<AsyncIsosurfaceResult>::finished, this,
                         [this, watcher]() {
                             --this->activeRemoteIsosurfaceRequests;
                             const auto result = watcher->result();
                             const int requestId = watcher->property("requestId").toInt();
                             const double isoValue = watcher->property("isoValue").toDouble();
                             if (std::fabs(this->inFlightRemoteIsosurfaceThreshold - isoValue) < 1e-9) {
                                 this->remoteIsosurfaceRequestInFlight = false;
                                 this->inFlightRemoteIsosurfaceThreshold =
                                         std::numeric_limits<double>::quiet_NaN();
                             }
                             watcher->deleteLater();

                             if (requestId != this->currentIsosurfaceRequestId) {
                                 if (this->activeRemoteIsosurfaceRequests == 0) {
                                     this->clearPersistentStatusMessage();
                                 }
                                 return;
                             }

                             if (!result.mesh || result.mesh->GetNumberOfPoints() == 0) {
                                 this->persistentStatusActive = false;
                                 this->statusMessageClearTimer.stop();
                                 this->statusBar()->showMessage(result.errorMessage.isEmpty()
                                                                        ? u"Remote isocontour is empty."_s
                                                                        : result.errorMessage);
                                 this->remoteIsosurfaceReady = false;
                                 this->setCubeRenderModeLocally(false);
                                 ui->actionVolume->setChecked(true);
                                 ui->vtkCube->renderWindow()->Render();
                                 return;
                             }

                             this->applyIsosurfaceResult(result);
                             this->clearPersistentStatusMessage();
                         });
        watcher->setFuture(QtConcurrent::run(&fetchRemoteIsosurface, this->remoteBackendUrl,
                                             this->remoteDatasetId, isoValue, requestId));
        return;
    }

    if (this->isosurfaceWatcher.isRunning()) {
        return;
    }

    auto *source = vtkImageData::SafeDownCast(this->cubeDisplaySource->GetOutputDataObject(0));
    if (!source) {
        return;
    }

    vtkSmartPointer<vtkImageData> data = vtkSmartPointer<vtkImageData>::New();
    QElapsedTimer deepCopyTimer;
    deepCopyTimer.start();
    data->DeepCopy(source);
    qDebug().noquote() << QStringLiteral("[perf][isosurface] DeepCopy before async launch: %1 ms")
                              .arg(deepCopyTimer.elapsed());

    this->isosurfaceWatcher.setFuture(
            QtConcurrent::run([data, isoValue, requestId]() {
                return computeIsosurface(data, isoValue, requestId);
            }));
}

void vtkWindowCube::scheduleIsosurfaceRecompute()
{
    this->isosurfaceDebounceTimer.start(150);
}

void vtkWindowCube::scheduleIsosurfacePrewarm()
{
    if (this->currentFullCubeGeneration <= 0
        || this->lastIsosurfacePrewarmGeneration == this->currentFullCubeGeneration) {
        return;
    }

    this->lastIsosurfacePrewarmGeneration = this->currentFullCubeGeneration;
    QTimer::singleShot(0, this, [this]() {
        if (this->isRemoteMode) {
            return;
        }
        if (this->isosurfaceWatcher.isRunning()) {
            return;
        }

        this->startAsyncIsosurface(ui->lineThreshold->text().toDouble());
    });
}

void vtkWindowCube::setCubeRenderModeLocally(bool isosurfaceMode)
{
    auto *renderer = ui->vtkCube->renderWindow()->GetRenderers()->GetFirstRenderer();
    if (!renderer) {
        return;
    }

    if (this->isRemoteMode && isosurfaceMode && !this->remoteIsosurfaceReady) {
        isosurfaceMode = false;
    }

    if (isosurfaceMode) {
        if (this->currentIsosurfaceActor && !renderer->HasViewProp(this->currentIsosurfaceActor)) {
            renderer->AddActor(this->currentIsosurfaceActor);
        }
        if (this->currentIsosurfaceActor && renderer->HasViewProp(this->volume)) {
            renderer->RemoveViewProp(this->volume);
        }
    } else {
        if (this->currentIsosurfaceActor && renderer->HasViewProp(this->currentIsosurfaceActor)) {
            renderer->RemoveActor(this->currentIsosurfaceActor);
        }
        if (!renderer->HasViewProp(this->volume)) {
            renderer->AddViewProp(this->volume);
        }
    }
}

void vtkWindowCube::applyIsosurfaceResult(const AsyncIsosurfaceResult &result)
{
    if (!result.mesh || result.mesh->GetNumberOfPoints() == 0 || result.mesh->GetNumberOfPolys() == 0) {
        return;
    }

    vtkSmartPointer<vtkPolyData> displayMesh = result.mesh;

    double meshBounds[6];
    result.mesh->GetBounds(meshBounds);
    auto *cubeImage = vtkImageData::SafeDownCast(this->cubeDisplaySource->GetOutputDataObject(0));
    double cubeBounds[6] = { 0., 0., 0., 0., 0., 0. };
    if (cubeImage) {
        cubeImage->GetBounds(cubeBounds);
    }
    qDebug().noquote()
            << QStringLiteral("[remote-iso] mesh bounds=%1,%2,%3,%4,%5,%6")
                       .arg(meshBounds[0], 0, 'g', 12)
                       .arg(meshBounds[1], 0, 'g', 12)
                       .arg(meshBounds[2], 0, 'g', 12)
                       .arg(meshBounds[3], 0, 'g', 12)
                       .arg(meshBounds[4], 0, 'g', 12)
                       .arg(meshBounds[5], 0, 'g', 12);
    qDebug().noquote()
            << QStringLiteral("[remote-iso] cube bounds=%1,%2,%3,%4,%5,%6")
                       .arg(cubeBounds[0], 0, 'g', 12)
                       .arg(cubeBounds[1], 0, 'g', 12)
                       .arg(cubeBounds[2], 0, 'g', 12)
                       .arg(cubeBounds[3], 0, 'g', 12)
                       .arg(cubeBounds[4], 0, 'g', 12)
                       .arg(cubeBounds[5], 0, 'g', 12);

    const bool degenerateBounds = !std::isfinite(meshBounds[0]) || !std::isfinite(meshBounds[1])
            || !std::isfinite(meshBounds[2]) || !std::isfinite(meshBounds[3])
            || !std::isfinite(meshBounds[4]) || !std::isfinite(meshBounds[5])
            || meshBounds[0] == meshBounds[1] || meshBounds[2] == meshBounds[3]
            || meshBounds[4] == meshBounds[5];
    if (degenerateBounds) {
        qDebug().noquote() << QStringLiteral("[remote-iso] degenerate mesh bounds");
        this->remoteIsosurfaceReady = false;
        this->persistentStatusActive = false;
        this->statusMessageClearTimer.stop();
        this->statusBar()->showMessage(u"Remote isocontour mesh has invalid bounds."_s);
        this->setCubeRenderModeLocally(false);
        ui->actionVolume->setChecked(true);
        ui->vtkCube->renderWindow()->Render();
        return;
    }

    if (this->isRemoteMode && cubeImage && validBounds(cubeBounds)) {
        const double fullBounds[6] = { 0.0,
                                       std::max(0, this->remoteDatasetWidth - 1) * 1.0,
                                       0.0,
                                       std::max(0, this->remoteDatasetHeight - 1) * 1.0,
                                       0.0,
                                       std::max(0, this->remoteDatasetDepth - 1) * 1.0 };
        const double fullSizeX = std::max(1e-9, fullBounds[1] - fullBounds[0]);
        const double fullSizeY = std::max(1e-9, fullBounds[3] - fullBounds[2]);
        const double fullSizeZ = std::max(1e-9, fullBounds[5] - fullBounds[4]);
        const double displaySizeX = cubeBounds[1] - cubeBounds[0];
        const double displaySizeY = cubeBounds[3] - cubeBounds[2];
        const double displaySizeZ = cubeBounds[5] - cubeBounds[4];

        vtkNew<vtkTransform> meshToDisplay;
        meshToDisplay->Scale(displaySizeX / fullSizeX, displaySizeY / fullSizeY,
                             displaySizeZ / fullSizeZ);
        meshToDisplay->Translate(cubeBounds[0], cubeBounds[2], cubeBounds[4]);

        vtkNew<vtkTransformPolyDataFilter> transformFilter;
        transformFilter->SetTransform(meshToDisplay);
        transformFilter->SetInputData(result.mesh);
        transformFilter->Update();

        displayMesh = vtkSmartPointer<vtkPolyData>::New();
        displayMesh->ShallowCopy(transformFilter->GetOutput());

        double transformedBounds[6];
        displayMesh->GetBounds(transformedBounds);
        qDebug().noquote()
                << QStringLiteral("[remote-iso] transformed mesh bounds=%1,%2,%3,%4,%5,%6")
                           .arg(transformedBounds[0], 0, 'g', 12)
                           .arg(transformedBounds[1], 0, 'g', 12)
                           .arg(transformedBounds[2], 0, 'g', 12)
                           .arg(transformedBounds[3], 0, 'g', 12)
                           .arg(transformedBounds[4], 0, 'g', 12)
                           .arg(transformedBounds[5], 0, 'g', 12);
    }

    QElapsedTimer actorTimer;
    actorTimer.start();
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(displayMesh);
    mapper->ScalarVisibilityOff();

    vtkNew<vtkActor> newActor;
    newActor->SetMapper(mapper);
    if (this->currentIsosurfaceActor) {
        newActor->GetProperty()->DeepCopy(this->currentIsosurfaceActor->GetProperty());
    } else {
        newActor->GetProperty()->SetColor(1., 0.5, 1.);
    }

    auto *renderer = ui->vtkCube->renderWindow()->GetRenderers()->GetFirstRenderer();
    if (this->currentIsosurfaceActor && renderer->HasViewProp(this->currentIsosurfaceActor)) {
        renderer->RemoveActor(this->currentIsosurfaceActor);
    }

    this->currentIsosurfaceActor = newActor;
    this->remoteIsosurfaceReady = true;
    qDebug().noquote()
            << QStringLiteral("[perf][isosurface] actor creation+swap: %1 ms")
                       .arg(actorTimer.elapsed());

    QElapsedTimer renderTimer;
    renderTimer.start();
    if (this->isRemoteMode) {
        ui->actionIsosurface->setChecked(true);
    }
    this->setCubeRenderModeLocally(true);
    ui->vtkCube->renderWindow()->Render();
    qDebug().noquote()
            << QStringLiteral("[perf][isosurface] render after apply: %1 ms")
                       .arg(renderTimer.elapsed());
}
