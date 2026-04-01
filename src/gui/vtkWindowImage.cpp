#include "vtkWindowImage.h"
#include "ui_vtkWindowImage.h"

#include "AstroUtils.h"
#include "app/BackendClient.h"
#include "ColorMaps.h"
#include "ImageLayerController.h"
#include "ImageLayerImportService.h"
#include "ImageLayerLoadTask.h"
#include "LUTCustomizerDialog.h"
#include "LayerListModel.h"
#include "ProfileWidget.h"
#include "vtkInteractorStyleProfile.h"
#include "vtkLegendScaleActorWCS.h"
#include "wcs.h"

#include <vtkCamera.h>
#include <vtkAxisActor2D.h>
#include <vtkCoordinate.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkImageStack.h>
#include <vtkInteractorStyleImage.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkScalarBarActor.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include <QActionGroup>
#include <QAction>
#include <QButtonGroup>
#include <QCheckBox>
#include <QDebug>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QMessageBox>
#include <QtConcurrentRun>

#include <cmath>
#include <cstring>
#include <sstream>

using namespace Qt::StringLiterals;

namespace {
constexpr int overlayTickCount = 5;

struct VisibleImageBounds2D
{
    bool valid{ false };
    double xmin{ 0. };
    double xmax{ 0. };
    double ymin{ 0. };
    double ymax{ 0. };
};

VisibleImageBounds2D computeVisibleImageBounds2D(vtkRenderer *renderer, vtkImageData *imageData)
{
    VisibleImageBounds2D result;
    if (!renderer || !imageData || !renderer->GetActiveCamera()) {
        return result;
    }

    const int *size = renderer->GetSize();
    if (!size || size[0] <= 0 || size[1] <= 0) {
        return result;
    }

    double bounds[6];
    imageData->GetBounds(bounds);
    auto *camera = renderer->GetActiveCamera();
    const double *focalPoint = camera->GetFocalPoint();
    const double halfHeight = camera->GetParallelScale();
    const double halfWidth = halfHeight * static_cast<double>(size[0]) / static_cast<double>(size[1]);

    result.xmin = std::max(bounds[0], focalPoint[0] - halfWidth);
    result.xmax = std::min(bounds[1], focalPoint[0] + halfWidth);
    result.ymin = std::max(bounds[2], focalPoint[1] - halfHeight);
    result.ymax = std::min(bounds[3], focalPoint[1] + halfHeight);
    result.valid = result.xmin <= result.xmax && result.ymin <= result.ymax;
    return result;
}

void configureAxisActor(vtkAxisActor2D *axis, double x1, double y1, double x2, double y2)
{
    axis->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
    axis->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
    axis->GetPoint1Coordinate()->SetValue(x1, y1);
    axis->GetPoint2Coordinate()->SetValue(x2, y2);
    axis->SetNumberOfLabels(5);
    axis->AdjustLabelsOn();
    axis->SetLabelFormat("%-#6.4g");
    axis->SetFontFactor(0.6);
    axis->SetTickLength(6);
    axis->AxisVisibilityOn();
    axis->TickVisibilityOn();
    axis->LabelVisibilityOn();
    axis->TitleVisibilityOn();
    axis->GetProperty()->SetColor(1., 1., 1.);
    axis->GetLabelTextProperty()->SetColor(1., 1., 1.);
    axis->GetTitleTextProperty()->SetColor(1., 1., 1.);
    axis->GetLabelTextProperty()->SetFontSize(14);
    axis->GetTitleTextProperty()->SetFontSize(16);
}

void configureVerticalAxisTitle(vtkAxisActor2D *axis)
{
    auto *titleProp = axis->GetTitleTextProperty();
    titleProp->SetOrientation(90.);
    titleProp->SetJustificationToCentered();
    titleProp->SetVerticalJustificationToCentered();
    titleProp->SetColor(1., 1., 1.);
    titleProp->SetFontSize(14);
    axis->SetTitlePosition(0.5);
}

QString upperCtype(const QString &value)
{
    return value.trimmed().toUpper();
}

int inferCelestialFrameFromCtypePair(const std::array<QString, 3> &ctype)
{
    const QString c1 = upperCtype(ctype[0]);
    const QString c2 = upperCtype(ctype[1]);
    if (c1.startsWith(u"GLON"_s) && c2.startsWith(u"GLAT"_s)) {
        return WCS_GALACTIC;
    }
    if (c1.startsWith(u"ELON"_s) && c2.startsWith(u"ELAT"_s)) {
        return WCS_ECLIPTIC;
    }
    if (c1.startsWith(u"RA"_s) && c2.startsWith(u"DEC"_s)) {
        return WCS_J2000;
    }
    return -1;
}

QString formatCelestialCoordinate(int frame, int axis, double value)
{
    char buffer[64] = { 0 };
    if (frame == WCS_J2000 && axis == 0) {
        ra2str(buffer, sizeof(buffer), value, 1);
    } else {
        dec2str(buffer, sizeof(buffer), value, axis == 0 ? 1 : 0);
    }
    return QString::fromLatin1(buffer).trimmed();
}

void configureTickLabelActor(vtkTextActor *actor, bool rightAligned)
{
    actor->GetTextProperty()->SetColor(1., 1., 1.);
    actor->GetTextProperty()->SetFontSize(12);
    actor->GetTextProperty()->SetVerticalJustificationToCentered();
    if (rightAligned) {
        actor->GetTextProperty()->SetJustificationToRight();
    } else {
        actor->GetTextProperty()->SetJustificationToCentered();
    }
}

vtkSmartPointer<vtkImageData> createPlaceholderImageData()
{
    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
    image->SetExtent(0, 0, 0, 0, 0, 0);
    image->SetOrigin(0., 0., 0.);
    image->SetSpacing(1., 1., 1.);
    image->AllocateScalars(VTK_FLOAT, 1);
    image->SetScalarComponentFromFloat(0, 0, 0, 0, 0.f);
    return image;
}

ImageLayerLoadResult createPlaceholderRemoteLayerResult(const QString &filepath)
{
    ImageLayerLoadResult result;
    result.valid = true;
    result.filepath = filepath.toStdString();
    result.imageData = createPlaceholderImageData();
    result.scalarRange = { 0., 0. };
    return result;
}

ImageLayerLoadResult fetchRemoteImageLayer(const QString &backendUrl, const QString &datasetId,
                                           const QString &datasetPath)
{
    ImageLayerLoadResult result;
    result.filepath = datasetPath.toStdString();

    BackendClient client(backendUrl);
    const auto response = client.requestImage(datasetId);
    if (!response.valid) {
        result.errorMessage = response.error.isEmpty() ? "Remote image request failed."
                                                       : response.error.toStdString();
        return result;
    }

    if (response.scalarType != u"float32"_s) {
        result.errorMessage = "Unsupported remote image scalar type.";
        return result;
    }

    const qsizetype expectedBytes =
            static_cast<qsizetype>(response.width) * response.height * static_cast<qsizetype>(sizeof(float));
    if (response.width <= 0 || response.height <= 0 || response.data.size() != expectedBytes) {
        result.errorMessage = "Invalid remote image payload.";
        return result;
    }

    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
    image->SetExtent(0, response.width - 1, 0, response.height - 1, 0, 0);
    image->SetOrigin(0., 0., 0.);
    image->SetSpacing(1., 1., 1.);
    image->AllocateScalars(VTK_FLOAT, 1);
    std::memcpy(image->GetScalarPointer(), response.data.constData(),
                static_cast<std::size_t>(expectedBytes));

    const double *range = image->GetScalarRange();
    result.imageData = image;
    result.scalarRange = { range[0], range[1] };
    result.valid = true;
    return result;
}
}

vtkWindowImage::vtkWindowImage(const QString &filepath, QWidget *parent)
    : vtkWindowImage(filepath,
                     {},
                     {},
                     { QString(), QString(), QString() },
                     { QString(), QString(), QString() },
                     { 0.0, 0.0, 0.0 },
                     { 1.0, 1.0, 1.0 },
                     { 1.0, 1.0, 1.0 },
                     parent)
{
}

vtkWindowImage::vtkWindowImage(const QString &filepath, const QString &backendUrl,
                               const QString &datasetId,
                               const std::array<QString, 3> &remoteCtype,
                               const std::array<QString, 3> &remoteCunit,
                               const std::array<double, 3> &remoteCrval,
                               const std::array<double, 3> &remoteCrpix,
                               const std::array<double, 3> &remoteCdelt, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::vtkWindowImage),
      filepath(filepath),
      isRemoteMode(!datasetId.isEmpty()),
      remoteBackendUrl(backendUrl),
      remoteDatasetId(datasetId),
      remoteDatasetCtype(remoteCtype),
      remoteDatasetCunit(remoteCunit),
      remoteDatasetCrval(remoteCrval),
      remoteDatasetCrpix(remoteCrpix),
      remoteDatasetCdelt(remoteCdelt),
      astro(this->isRemoteMode ? nullptr : std::make_unique<AstroUtils>(filepath.toStdString())),
      lutCustomizer(nullptr),
      profileWidget(nullptr),
      layers(nullptr),
      importService(std::make_unique<ImageLayerImportService>())
{
    ui->setupUi(this);
    if (this->isRemoteMode) {
        qDebug().noquote()
                << QStringLiteral("[wcs] remote metadata loaded ctype=%1,%2 cdelt=%3,%4")
                           .arg(this->remoteDatasetCtype[0], this->remoteDatasetCtype[1])
                           .arg(this->remoteDatasetCdelt[0], 0, 'g', 12)
                           .arg(this->remoteDatasetCdelt[1], 0, 'g', 12);
    }
    this->setWindowTitle(this->isRemoteMode ? u"%1 [remote image]"_s.arg(this->filepath)
                                            : this->filepath);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->wcsAxesCheck = new QCheckBox(u"Show WCS Axes"_s, this);
    this->wcsAxesCheck->setChecked(this->showWcsAxes);
    this->statusBar()->addPermanentWidget(this->wcsAxesCheck);
    QObject::connect(this->wcsAxesCheck, &QCheckBox::toggled, this, [this](bool checked) {
        this->showWcsAxes = checked;
        this->lastOverlayVisibleBounds = { std::numeric_limits<double>::quiet_NaN(),
                                           std::numeric_limits<double>::quiet_NaN(),
                                           std::numeric_limits<double>::quiet_NaN(),
                                           std::numeric_limits<double>::quiet_NaN() };
        this->lastOverlayViewportSize = { -1, -1 };
        this->setWcsOverlayVisible(checked);
        this->updateWcsOverlay();
        this->vtkRender();
    });
    this->statusMessageClearTimer.setSingleShot(true);
    QObject::connect(&this->statusMessageClearTimer, &QTimer::timeout, this, [this]() {
        this->persistentStatusActive = false;
        this->statusBar()->clearMessage();
    });
    QObject::connect(&this->layerLoadWatcher, &QFutureWatcher<ImageLayerLoadResult>::finished, this,
                     [this]() {
                         this->setLayerImportEnabled(true);

                         const auto result = this->layerLoadWatcher.result();
                         if (!result.valid) {
                             if (!result.errorMessage.empty()) {
                                 this->persistentStatusActive = false;
                                 this->statusMessageClearTimer.stop();
                                 QMessageBox::warning(this, u"Import FITS file"_s,
                                                      QString::fromStdString(result.errorMessage));
                             }
                             this->clearPersistentStatusMessage();
                             return;
                         }

                         QElapsedTimer applyTimer;
                         applyTimer.start();
                         this->applyLoadedLayer(result);
                         qDebug().noquote()
                                 << QStringLiteral("[perf][layer] UI apply: %1 ms")
                                            .arg(applyTimer.elapsed());
                         this->clearPersistentStatusMessage();
                     });
    QObject::connect(&this->remoteImageWatcher, &QFutureWatcher<ImageLayerLoadResult>::finished, this,
                     [this]() {
                         const auto result = this->remoteImageWatcher.result();
                         if (!result.valid || !result.imageData) {
                             this->persistentStatusActive = false;
                             this->statusMessageClearTimer.stop();
                             this->statusBar()->showMessage(result.errorMessage.empty()
                                                                    ? u"Could not load remote image."_s
                                                                    : QString::fromStdString(result.errorMessage));
                             return;
                         }

                         this->applyRemoteMasterLayer(result);
                         this->clearPersistentStatusMessage();
                     });

    this->layers = this->isRemoteMode
            ? new LayerListModel(createPlaceholderRemoteLayerResult(this->filepath), this)
            : new LayerListModel(this->filepath.toStdString(), this);

    this->setupRenderer();

    // Setup Menu File
    QObject::connect(ui->actionAddFITS, &QAction::triggered, this, &vtkWindowImage::addLocalFile);

    // Setup Menu WCS
    auto groupWCS = new QActionGroup(this);
    groupWCS->addAction(ui->actionGalactic);
    groupWCS->addAction(ui->actionFK5);
    groupWCS->addAction(ui->actionEcliptic);
    QObject::connect(groupWCS, &QActionGroup::triggered, this, &vtkWindowImage::changeLegendWCS);
    ui->menuWCS->addSeparator();
    auto *formatGroup = new QActionGroup(this);
    this->actionWcsSexagesimal = ui->menuWCS->addAction(u"Sexagesimal"_s);
    this->actionWcsDecimal = ui->menuWCS->addAction(u"Decimal"_s);
    this->actionWcsSexagesimal->setCheckable(true);
    this->actionWcsDecimal->setCheckable(true);
    formatGroup->addAction(this->actionWcsSexagesimal);
    formatGroup->addAction(this->actionWcsDecimal);
    QObject::connect(formatGroup, &QActionGroup::triggered, this, [this](QAction *action) {
        this->wcsFormatExplicitlyChosen = true;
        this->useSexagesimalWcsFormat = action == this->actionWcsSexagesimal;
        this->invalidateWcsOverlayCache();
        ui->vtk->renderWindow()->Render();
    });
    this->applyDefaultWcsFormatForSelectedFrame();

    // Setup Menu Tools
    QObject::connect(ui->actionProfile, &QAction::triggered, this,
                     &vtkWindowImage::setInteractorStyleProfile);

    // Setup Buttons
    ui->btnSources->setIcon(QIcon(u":/icons/RECT_SELECT.png"_s));
    ui->btnFilaments->setIcon(QIcon(u":/icons/RECT_SELECT.png"_s));
    ui->btn3D->setIcon(QIcon(u":/icons/RECT_SELECT.png"_s));

    // Color Maps Combobox
    auto cmaps = ColorMaps::GetColorMapNames();
    std::for_each(cmaps.cbegin(), cmaps.cend(), [this](const std::string &name) {
        ui->comboLut->addItem(QString::fromStdString(name));
    });
    ui->comboLut->setCurrentText(QString::fromStdString(ColorMaps::DefaultColorMap));
    QObject::connect(ui->comboLut, &QComboBox::textActivated, this,
                     &vtkWindowImage::changeCurrentColorMap);

    // LUT Customizer
    QObject::connect(ui->btnLutEdit, &QPushButton::clicked, this,
                     &vtkWindowImage::showLUTCustomizer);

    // Scale Radio Buttons
    auto group = new QButtonGroup(this);
    group->addButton(ui->radioLinear);
    group->addButton(ui->radioLog);
    QObject::connect(group, &QButtonGroup::buttonClicked, this,
                     &vtkWindowImage::changeCurrentColorScale);

    // Layer Opacity
    QObject::connect(ui->sliderOpacity, &QSlider::actionTriggered, this,
                     &vtkWindowImage::changeCurrentLayerOpacity);

    // Setup Layer List View
    ui->listLayer->setAcceptDrops(true);
    ui->listLayer->setModel(this->layers);
    ui->listLayer->setCurrentIndex(this->layers->index(0, 0));
    QObject::connect(this->layers, &LayerListModel::dataChanged, this, &vtkWindowImage::vtkRender);
    QObject::connect(ui->listLayer->selectionModel(), &QItemSelectionModel::currentChanged, this,
                     &vtkWindowImage::showCurrentLayerSettings);
    QObject::connect(ui->listLayer->selectionModel(), &QItemSelectionModel::currentChanged, this,
                     &vtkWindowImage::updateLUTCustomizer);

    if (this->isRemoteMode) {
        ui->actionAddFITS->setEnabled(false);
        ui->actionProfile->setEnabled(false);
        this->showPersistentStatusMessage(u"Loading remote image..."_s);
        this->remoteImageWatcher.setFuture(
                QtConcurrent::run(&fetchRemoteImageLayer, this->remoteBackendUrl,
                                  this->remoteDatasetId, this->filepath));
    }
}

vtkWindowImage::~vtkWindowImage()
{
    delete ui;
}

void vtkWindowImage::closeEvent(QCloseEvent *event)
{
    if (this->isBusy()) {
        this->showPersistentStatusMessage(this->isRemoteMode ? u"Loading remote image..."_s
                                                             : u"Loading layer..."_s);
        event->ignore();
        return;
    }

    QMainWindow::closeEvent(event);
}

void vtkWindowImage::showLUTCustomizer()
{
    const int index = this->currentLayerIndex();
    if (!this->lutCustomizer) {
        this->lutCustomizer = new LUTCustomizerDialog(this);
        QObject::connect(this->lutCustomizer, &LUTCustomizerDialog::lutUpdated, this,
                         &vtkWindowImage::showCurrentLayerSettings);
    }
    this->lutCustomizer->init(this->layers->getImageData(index),
                              this->layers->getLookupTable(index));
    this->lutCustomizer->show();
    this->lutCustomizer->raise();
    this->lutCustomizer->activateWindow();
}

void vtkWindowImage::updateLUTCustomizer()
{
    if (this->lutCustomizer) {
        const int index = this->currentLayerIndex();
        this->lutCustomizer->init(this->layers->getImageData(index),
                                  this->layers->getLookupTable(index));
    }
}

void vtkWindowImage::addLocalFile()
{
    if (this->isBusy()) {
        return;
    }

    const QString filepath = QFileDialog::getOpenFileName(this, u"Import FITS file"_s, QString(),
                                                          u"FITS files (*.fits *.fit)"_s);

    if (filepath.isEmpty()) {
        return;
    }

    const ImageLayerImportResult result =
            this->importService->inspect(ImageLayerImportRequest { this->filepath, filepath });
    if (!result.accepted) {
        QMessageBox::warning(this, u"Import FITS file"_s, result.errorMessage);
        return;
    }

    this->addLayerImage(filepath.toStdString());
}

void vtkWindowImage::setupRenderer()
{
    vtkNew<vtkRenderer> ren;
    ren->SetBackground(0.21, 0.23, 0.25);
    ren->GetActiveCamera()->ParallelProjectionOn();

    vtkNew<vtkGenericOpenGLRenderWindow> win;
    win->AddRenderer(ren);
    win->AddObserver(vtkCommand::EndEvent, this, &vtkWindowImage::updateWcsOverlay);
    ui->vtk->setRenderWindow(win);
    ui->vtk->setEnableTouchEventProcessing(false);

    vtkNew<vtkInteractorStyleImage> style;
    win->GetInteractor()->SetInteractorStyle(style);
    win->GetInteractor()->AddObserver(vtkCommand::MouseMoveEvent, this,
                                      &vtkWindowImage::mouseCallback);

    this->coordinate->SetCoordinateSystemToDisplay();
    this->coordinate->SetViewport(ren);

    // Stack
    this->layers = new LayerListModel(this->filepath.toStdString(), this);
    this->stack->AddImage(this->layers->getMasterLayerActor());
    this->stack->SetActiveLayer(0);
    ren->AddViewProp(this->stack);

    // Color bar
    this->colorbar->SetMaximumWidthInPixels(120);
    this->colorbar->SetPosition(0.9, 0.1);
    this->colorbar->SetLookupTable(this->layers->getLookupTable(this->layers->getMasterIndex()));
    ren->AddViewProp(this->colorbar);

    this->layerController =
            std::make_unique<ImageLayerController>(*(this->layers), this->stack, this->colorbar);

    // Legend
    if (this->astro) {
        this->legendWCS->Init(this->filepath.toStdString());
        this->legendWCS->SetWCS(WCS_GALACTIC);
        ren->AddViewProp(this->legendWCS);
    }
    ren->AddViewProp(this->overlayXAxis);
    ren->AddViewProp(this->overlayYAxis);
    ren->AddViewProp(this->overlayXTitleActor);
    ren->AddViewProp(this->overlayYTitleActor);
    this->overlayXAxis->GetTitleTextProperty()->SetFontSize(16);
    this->overlayXAxis->GetTitleTextProperty()->SetBold(false);
    this->overlayYAxis->GetTitleTextProperty()->SetFontSize(16);
    this->overlayYAxis->GetTitleTextProperty()->SetOrientation(90.);
    this->overlayYAxis->GetTitleTextProperty()->SetBold(false);
    this->overlayXTitleActor->GetTextProperty()->SetColor(1., 1., 1.);
    this->overlayXTitleActor->GetTextProperty()->SetFontSize(14);
    this->overlayXTitleActor->GetTextProperty()->SetJustificationToCentered();
    this->overlayXTitleActor->GetTextProperty()->SetVerticalJustificationToCentered();
    this->overlayYTitleActor->GetTextProperty()->SetColor(1., 1., 1.);
    this->overlayYTitleActor->GetTextProperty()->SetFontSize(14);
    this->overlayYTitleActor->GetTextProperty()->SetOrientation(90.);
    this->overlayYTitleActor->GetTextProperty()->SetJustificationToCentered();
    this->overlayYTitleActor->GetTextProperty()->SetVerticalJustificationToCentered();
    this->ensureOverlayTickActors(ren);
    this->setWcsOverlayVisible(this->showWcsAxes);

    ren->ResetCamera();
    win->Render();
}

void vtkWindowImage::mouseCallback()
{
    if (this->isBusy()) {
        return;
    }

    const int *position = ui->vtk->renderWindow()->GetInteractor()->GetEventPosition();
    this->coordinate->SetValue(position[0], position[1]);
    const double *worldCoord = this->coordinate->GetComputedWorldValue(nullptr);
    const long imageCoord[2] = { std::lround(worldCoord[0]), std::lround(worldCoord[1]) };

    std::ostringstream ss;
    ss << "<value> "
       << this->layers->getPixelValue(this->layers->getMasterIndex(), imageCoord[0], imageCoord[1]);
    ss << "  <image> X: " << worldCoord[0] << " Y: " << worldCoord[1];

    if (this->astro && !this->astro->isSimulation()) {
        double wcs[2];
        this->astro->xy2sky(worldCoord, wcs, WCS_GALACTIC);
        ss << "  <galactic> GLON: " << wcs[0] << " GLAT: " << wcs[1];

        this->astro->xy2sky(worldCoord, wcs, WCS_J2000);
        ss << "  <fk5> RA: " << wcs[0] << " Dec: " << wcs[1];

        this->astro->xy2sky(worldCoord, wcs, WCS_ECLIPTIC);
        ss << "  <ecliptic> ELON: " << wcs[0] << " ELAT: " << wcs[1];
    } else if (this->isRemoteMode) {
        const QString axis1Label = this->remoteDatasetCtype[0].isEmpty() ? u"AXIS1"_s
                                                                          : this->remoteDatasetCtype[0];
        const QString axis2Label = this->remoteDatasetCtype[1].isEmpty() ? u"AXIS2"_s
                                                                          : this->remoteDatasetCtype[1];
        ss << "  <wcs> " << axis1Label.toStdString() << ": "
           << this->remoteFormatAxisCoordinate(0, static_cast<double>(imageCoord[0])).toStdString();
        ss << "  " << axis2Label.toStdString() << ": "
           << this->remoteFormatAxisCoordinate(1, static_cast<double>(imageCoord[1])).toStdString();
    }

    this->statusBar()->showMessage(QString::fromStdString(ss.str()));
}

void vtkWindowImage::setInteractorStyleImage()
{
    vtkNew<vtkInteractorStyleImage> style;
    ui->vtk->renderWindow()->GetInteractor()->SetInteractorStyle(style);
    this->vtkRender();
}

void vtkWindowImage::setInteractorStyleProfile()
{
    if (!this->profileWidget) {
        vtkNew<vtkInteractorStyleProfile> style;
        ui->vtk->renderWindow()->GetInteractor()->SetInteractorStyle(style);
        this->vtkRender();

        this->profileWidget =
                new ProfileWidget(style, this->layers->getImageData(this->layers->getMasterIndex()),
                                  this->filepath.toStdString(), this);
        this->profileWidget->setupImagePlots();
        QObject::connect(this->profileWidget, &ProfileWidget::destroyed, this,
                         &vtkWindowImage::setInteractorStyleImage, Qt::QueuedConnection);
    }
}

int vtkWindowImage::currentLayerIndex() const
{
    return ui->listLayer->currentIndex().row();
}

void vtkWindowImage::addLayerImage(const std::string &filepath)
{
    this->setLayerImportEnabled(false);
    this->showPersistentStatusMessage(u"Loading layer..."_s);
    this->layerLoadWatcher.setFuture(QtConcurrent::run(
            &loadImageLayer, ImageLayerLoadRequest { this->filepath.toStdString(), filepath }));
}

void vtkWindowImage::applyLoadedLayer(const ImageLayerLoadResult &result)
{
    QElapsedTimer timer;
    timer.start();
    this->stack->AddImage(this->layers->addLayer(result));
    this->vtkRender();
    qDebug().noquote()
            << QStringLiteral("[perf][layer] render after apply: %1 ms").arg(timer.elapsed());
}

void vtkWindowImage::applyRemoteMasterLayer(const ImageLayerLoadResult &result)
{
    auto *oldActor = this->layers->getMasterLayerActor();
    auto *newActor = this->layers->replaceMasterLayer(result);
    if (!newActor) {
        return;
    }

    if (oldActor) {
        this->stack->RemoveImage(oldActor);
    }
    this->stack->AddImage(newActor);
    this->stack->SetActiveLayer(this->layers->getMasterIndex());
    this->layerController->activateLayer(this->layers->getMasterIndex());
    auto *renderer = ui->vtk->renderWindow()->GetRenderers()->GetFirstRenderer();
    if (renderer) {
        renderer->ResetCamera();
    }
    this->vtkRender();
}

bool vtkWindowImage::isBusy() const
{
    return this->layerLoadWatcher.isRunning() || this->remoteImageWatcher.isRunning();
}

void vtkWindowImage::showPersistentStatusMessage(const QString &text, int minDurationMs)
{
    this->statusMessageClearTimer.stop();
    this->persistentStatusActive = true;
    this->statusMessageMinDurationMs = minDurationMs;
    this->statusMessageElapsed.restart();
    this->statusBar()->showMessage(text);
}

void vtkWindowImage::clearPersistentStatusMessage()
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

void vtkWindowImage::setLayerImportEnabled(bool enabled)
{
    ui->actionAddFITS->setEnabled(enabled && !this->isRemoteMode);
}

void vtkWindowImage::updateWcsOverlay()
{
    auto *renderer = ui->vtk->renderWindow() ? ui->vtk->renderWindow()->GetRenderers()->GetFirstRenderer()
                                             : nullptr;
    auto *imageData = this->layers ? this->layers->getImageData(this->layers->getMasterIndex()) : nullptr;
    const bool useLegend = this->astro && !this->astro->isSimulation();
    this->setWcsOverlayVisible(this->showWcsAxes);
    if (!this->showWcsAxes || useLegend || !renderer || !imageData) {
        return;
    }
    this->ensureOverlayTickActors(renderer);

    const auto visible = computeVisibleImageBounds2D(renderer, imageData);
    if (!visible.valid) {
        this->overlayXAxis->VisibilityOff();
        this->overlayYAxis->VisibilityOff();
        return;
    }

    const int *size = renderer->GetSize();
    if (!size || size[0] <= 0 || size[1] <= 0) {
        return;
    }

    const std::array<double, 4> visibleBounds = { visible.xmin, visible.xmax, visible.ymin, visible.ymax };
    const std::array<int, 2> viewportSize = { size[0], size[1] };
    if (visibleBounds == this->lastOverlayVisibleBounds && viewportSize == this->lastOverlayViewportSize) {
        return;
    }
    this->lastOverlayVisibleBounds = visibleBounds;
    this->lastOverlayViewportSize = viewportSize;

    constexpr double leftMargin = 168.;
    constexpr double axisX = 136.;
    constexpr double bottomMargin = 58.;
    constexpr double rightMargin = 34.;
    constexpr double topMargin = 28.;
    configureAxisActor(this->overlayXAxis, axisX, bottomMargin, size[0] - rightMargin, bottomMargin);
    configureAxisActor(this->overlayYAxis, axisX, size[1] - topMargin, axisX, bottomMargin);
    this->overlayXAxis->SetTitle("");
    this->overlayXTitleActor->SetInput(this->remoteOverlayAxisTitle(0).toStdString().c_str());
    this->overlayXTitleActor->SetDisplayPosition((axisX + (size[0] - rightMargin)) / 2,
                                                 static_cast<int>(bottomMargin / 2.0) - 2);
    this->overlayYAxis->SetTitle("");
    this->overlayYTitleActor->SetInput(this->remoteOverlayAxisTitle(1).toStdString().c_str());
    this->overlayYTitleActor->SetDisplayPosition(static_cast<int>(leftMargin / 3.0), size[1] / 2);

    bool xOk = false;
    const double xMin = this->remoteVoxelToWcs(0, visible.xmin, &xOk);
    const double xMax = this->remoteVoxelToWcs(0, visible.xmax, &xOk);
    bool yOk = false;
    const double yMin = this->remoteVoxelToWcs(1, visible.ymin, &yOk);
    const double yMax = this->remoteVoxelToWcs(1, visible.ymax, &yOk);
    this->overlayXAxis->SetRange(xOk ? xMin : visible.xmin, xOk ? xMax : visible.xmax);
    this->overlayYAxis->SetRange(yOk ? yMax : visible.ymax, yOk ? yMin : visible.ymin);
    this->overlayXAxis->LabelVisibilityOff();
    this->overlayYAxis->LabelVisibilityOff();
    this->overlayXAxis->VisibilityOn();
    this->overlayYAxis->VisibilityOn();
    this->overlayXTitleActor->VisibilityOn();
    this->overlayYTitleActor->VisibilityOn();
    for (int i = 0; i < overlayTickCount; ++i) {
        const double t = overlayTickCount == 1 ? 0.0
                                               : static_cast<double>(i)
                        / static_cast<double>(overlayTickCount - 1);
        const double voxelX = visible.xmin + t * (visible.xmax - visible.xmin);
        const double voxelY = visible.ymin + t * (visible.ymax - visible.ymin);
        bool tickXOk = false;
        bool tickYOk = false;
        double tickX = this->remoteVoxelToWcs(0, voxelX, &tickXOk);
        double tickY = this->remoteVoxelToWcs(1, voxelY, &tickYOk);
        double frameX = tickX;
        double frameY = tickY;
        if (tickXOk && tickYOk && this->remoteHasCelestialAxes()) {
            this->convertRemoteCelestialCoordinates(tickX, tickY, frameX, frameY);
        }

        this->overlayXTickActors[static_cast<std::size_t>(i)]->SetInput(
                this->formatRemoteOverlayCoordinate(0, frameX).toStdString().c_str());
        this->overlayXTickActors[static_cast<std::size_t>(i)]->SetDisplayPosition(
                static_cast<int>(axisX + t * ((size[0] - rightMargin) - axisX)),
                static_cast<int>(bottomMargin - 18));
        this->overlayXTickActors[static_cast<std::size_t>(i)]->VisibilityOn();

        this->overlayYTickActors[static_cast<std::size_t>(i)]->SetInput(
                this->formatRemoteOverlayCoordinate(1, frameY).toStdString().c_str());
        this->overlayYTickActors[static_cast<std::size_t>(i)]->SetDisplayPosition(
                static_cast<int>(axisX - 10),
                static_cast<int>(bottomMargin + t * ((size[1] - topMargin) - bottomMargin)));
        this->overlayYTickActors[static_cast<std::size_t>(i)]->VisibilityOn();
    }
    qDebug().noquote()
            << QStringLiteral("[wcs-overlay] updated ticks x=%1..%2 y=%3..%4 size=%5x%6 actor=%7 endpoints=(%8,%9)->(%10,%11) outer=%12")
                       .arg(visible.xmin, 0, 'g', 12)
                       .arg(visible.xmax, 0, 'g', 12)
                       .arg(visible.ymin, 0, 'g', 12)
                       .arg(visible.ymax, 0, 'g', 12)
                       .arg(size[0])
                       .arg(size[1])
                       .arg(this->overlayXAxis->GetVisibility())
                       .arg(axisX, 0, 'g', 12)
                       .arg(size[1] - topMargin, 0, 'g', 12)
                       .arg(axisX, 0, 'g', 12)
                       .arg(bottomMargin, 0, 'g', 12)
                       .arg(leftMargin, 0, 'g', 12);
}

void vtkWindowImage::setWcsOverlayVisible(bool visible)
{
    const bool useLegend = this->astro && !this->astro->isSimulation();
    if (this->legendWCS) {
        this->legendWCS->SetVisibility(visible && useLegend);
    }
    this->overlayXAxis->SetVisibility(visible && !useLegend);
    this->overlayYAxis->SetVisibility(visible && !useLegend);
    this->overlayXTitleActor->SetVisibility(visible && !useLegend);
    this->overlayYTitleActor->SetVisibility(visible && !useLegend);
    for (const auto &actor : this->overlayXTickActors) {
        if (actor) {
            actor->SetVisibility(visible && !useLegend);
        }
    }
    for (const auto &actor : this->overlayYTickActors) {
        if (actor) {
            actor->SetVisibility(visible && !useLegend);
        }
    }
}

void vtkWindowImage::ensureOverlayTickActors(vtkRenderer *renderer)
{
    if (!renderer) {
        return;
    }
    if (this->overlayXTickActors.empty()) {
        for (int i = 0; i < overlayTickCount; ++i) {
            auto actor = vtkSmartPointer<vtkTextActor>::New();
            configureTickLabelActor(actor, false);
            renderer->AddViewProp(actor);
            this->overlayXTickActors.push_back(actor);
        }
    }
    if (this->overlayYTickActors.empty()) {
        for (int i = 0; i < overlayTickCount; ++i) {
            auto actor = vtkSmartPointer<vtkTextActor>::New();
            configureTickLabelActor(actor, true);
            renderer->AddViewProp(actor);
            this->overlayYTickActors.push_back(actor);
        }
    }
}

void vtkWindowImage::invalidateWcsOverlayCache()
{
    this->lastOverlayVisibleBounds = { std::numeric_limits<double>::quiet_NaN(),
                                       std::numeric_limits<double>::quiet_NaN(),
                                       std::numeric_limits<double>::quiet_NaN(),
                                       std::numeric_limits<double>::quiet_NaN() };
    this->lastOverlayViewportSize = { -1, -1 };
}

void vtkWindowImage::applyDefaultWcsFormatForSelectedFrame()
{
    if (this->wcsFormatExplicitlyChosen) {
        return;
    }

    this->useSexagesimalWcsFormat = this->selectedWcsFrame() == WCS_J2000;
    if (this->actionWcsSexagesimal) {
        this->actionWcsSexagesimal->setChecked(this->useSexagesimalWcsFormat);
    }
    if (this->actionWcsDecimal) {
        this->actionWcsDecimal->setChecked(!this->useSexagesimalWcsFormat);
    }
}

bool vtkWindowImage::remoteHasWcsAxis(int axis) const
{
    return axis >= 0 && axis < 3 && std::isfinite(this->remoteDatasetCrval[axis])
            && std::isfinite(this->remoteDatasetCrpix[axis])
            && std::isfinite(this->remoteDatasetCdelt[axis])
            && std::abs(this->remoteDatasetCdelt[axis]) > 1e-12;
}

double vtkWindowImage::remoteVoxelToWcs(int axis, double voxelIndex, bool *ok) const
{
    const bool valid = this->remoteHasWcsAxis(axis);
    if (ok) {
        *ok = valid;
    }
    if (!valid) {
        return voxelIndex;
    }

    return this->remoteDatasetCrval[axis]
            + ((voxelIndex + 1.0) - this->remoteDatasetCrpix[axis]) * this->remoteDatasetCdelt[axis];
}

QString vtkWindowImage::remoteFormatAxisCoordinate(int axis, double voxelIndex) const
{
    bool ok = false;
    const double world = this->remoteVoxelToWcs(axis, voxelIndex, &ok);
    if (!ok) {
        return QString::number(voxelIndex, 'g', 12);
    }

    const QString unit = (axis >= 0 && axis < 3) ? this->remoteDatasetCunit[axis].trimmed() : QString();
    if (unit.isEmpty()) {
        return QString::number(world, 'g', 12);
    }
    return u"%1 %2"_s.arg(QString::number(world, 'g', 12), unit);
}

QString vtkWindowImage::remoteAxisTitle(int axis) const
{
    const QString base = axis == 0 ? u"X"_s : (axis == 1 ? u"Y"_s : u"Z"_s);
    const QString ctype = (axis >= 0 && axis < 3) ? this->remoteDatasetCtype[axis].trimmed() : QString();
    const QString cunit = (axis >= 0 && axis < 3) ? this->remoteDatasetCunit[axis].trimmed() : QString();
    const QString label = ctype.isEmpty() ? base : ctype;
    return cunit.isEmpty() ? label : u"%1 (%2)"_s.arg(label, cunit);
}

int vtkWindowImage::selectedWcsFrame() const
{
    return ui->actionGalactic->isChecked() ? WCS_GALACTIC
            : (ui->actionFK5->isChecked() ? WCS_J2000 : WCS_ECLIPTIC);
}

int vtkWindowImage::remoteNativeCelestialFrame() const
{
    return inferCelestialFrameFromCtypePair(this->remoteDatasetCtype);
}

bool vtkWindowImage::remoteHasCelestialAxes() const
{
    return this->remoteNativeCelestialFrame() >= 0 && this->remoteHasWcsAxis(0)
            && this->remoteHasWcsAxis(1);
}

bool vtkWindowImage::convertRemoteCelestialCoordinates(double nativeX, double nativeY, double &frameX,
                                                       double &frameY) const
{
    frameX = nativeX;
    frameY = nativeY;
    const int nativeFrame = this->remoteNativeCelestialFrame();
    const int targetFrame = this->selectedWcsFrame();
    if (nativeFrame < 0) {
        return false;
    }
    if (nativeFrame != targetFrame) {
        wcscon(nativeFrame, targetFrame, 2000.0, 2000.0, &frameX, &frameY, 2000.0);
    }
    return true;
}

QString vtkWindowImage::formatRemoteOverlayCoordinate(int axis, double value) const
{
    if (!this->remoteHasCelestialAxes()) {
        return QString::number(value, 'g', 8);
    }
    if (!this->useSexagesimalWcsFormat) {
        return this->formatDegreeCoordinate(value);
    }
    return formatCelestialCoordinate(this->selectedWcsFrame(), axis, value);
}

QString vtkWindowImage::remoteOverlayAxisTitle(int axis) const
{
    if (!this->remoteHasCelestialAxes()) {
        return this->remoteAxisTitle(axis);
    }
    const int frame = this->selectedWcsFrame();
    if (frame == WCS_J2000) {
        return axis == 0 ? u"Right Ascension"_s : u"Declination"_s;
    }
    if (frame == WCS_GALACTIC) {
        return axis == 0 ? u"Galactic Longitude"_s : u"Galactic Latitude"_s;
    }
    return axis == 0 ? u"Ecliptic Longitude"_s : u"Ecliptic Latitude"_s;
}

QString vtkWindowImage::formatDegreeCoordinate(double value) const
{
    return QString::number(value, 'f', 2);
}

void vtkWindowImage::vtkRender()
{
    ui->vtk->renderWindow()->Render();
}

void vtkWindowImage::changeLegendWCS()
{
    const int wcs = (ui->actionGalactic->isChecked()
                             ? WCS_GALACTIC
                             : (ui->actionFK5->isChecked() ? WCS_J2000 : WCS_ECLIPTIC));
    if (this->astro && !this->astro->isSimulation()) {
        this->legendWCS->SetWCS(wcs);
    }
    this->applyDefaultWcsFormatForSelectedFrame();
    this->invalidateWcsOverlayCache();
    qDebug().noquote()
            << QStringLiteral("[wcs] overlay using selected frame %1")
                       .arg(wcs == WCS_GALACTIC ? u"Galactic"_s
                                                : (wcs == WCS_J2000 ? u"FK5"_s : u"Ecliptic"_s));
    ui->vtk->renderWindow()->Render();
}

void vtkWindowImage::changeCurrentColorMap()
{
    this->layerController->setCurrentColorMap(this->currentLayerIndex(),
                                              ui->comboLut->currentText().toStdString());
    this->vtkRender();
}

void vtkWindowImage::changeCurrentColorScale()
{
    this->layerController->setCurrentLogScale(this->currentLayerIndex(), ui->radioLog->isChecked());
    this->vtkRender();
}

void vtkWindowImage::changeCurrentLayerOpacity()
{
    const double opacity = ui->sliderOpacity->sliderPosition() / 100.;
    this->layerController->setCurrentOpacity(this->currentLayerIndex(), opacity);
    this->vtkRender();
}

void vtkWindowImage::showCurrentLayerSettings()
{
    const int index = this->currentLayerIndex();
    const auto state = this->layerController->layerViewState(index);
    if (!state.valid) {
        return;
    }

    ui->comboLut->setCurrentText(QString::fromStdString(state.colorMapName));
    if (state.usingLogScale) {
        ui->radioLog->setChecked(true);
    } else {
        ui->radioLinear->setChecked(true);
    }
    ui->sliderOpacity->setValue(state.opacityPercent);

    this->layerController->activateLayer(index);
    this->vtkRender();
}
