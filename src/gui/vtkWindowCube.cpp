#include "vtkWindowCube.h"
#include "ui_vtkWindowCube.h"

#include "ColorMaps.h"
#include "CubeViewController.h"
#include "LUTCustomizerDialog.h"
#include "MomentMapComputeTask.h"
#include "ProfileWidget.h"
#include "vtkFITSReader.h"
#include "vtkInteractorStyleProfile.h"
#include "vtkLegendScaleActorWCS.h"
#include "vtkMomentMapFilter.h"
#include "wcs.h"

#include <QVTKInteractor.h>
#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkColorTransferFunction.h>
#include <vtkCoordinate.h>
#include <vtkExtractVOI.h>
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
#include <vtkPiecewiseFunction.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkScalarBarActor.h>
#include <vtkTrivialProducer.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

#include <QActionGroup>
#include <QApplication>
#include <QColorDialog>
#include <QDoubleValidator>
#include <QInputDialog>
#include <QLabel>
#include <QSignalBlocker>
#include <QtConcurrentRun>

#include <cmath>
#include <limits>
#include <sstream>

using namespace Qt::StringLiterals;

namespace {
AsyncIsosurfaceResult computeIsosurface(vtkSmartPointer<vtkImageData> image, double isoValue,
                                        int requestId)
{
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
    return result;
}
}

vtkWindowCube::vtkWindowCube(const QString &filepath, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::vtkWindowCube),
      filepath(filepath),
      astro(filepath.toStdString()),
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
    this->isosurfaceDebounceTimer.setSingleShot(true);
    QObject::connect(&this->isosurfaceDebounceTimer, &QTimer::timeout, this, [this]() {
        if (this->cubeOpenWatcher.isRunning() || !ui->actionIsosurface->isChecked()) {
            return;
        }

        this->startAsyncIsosurface(ui->lineThreshold->text().toDouble());
    });

    this->reader->SetFileName(this->filepath.toUtf8());
    const CubeOpenStageResult preview = loadCubeOpenPreview(this->filepath);
    const bool usingPreview = preview.valid && preview.cubeImageData && preview.momentImageData;

    if (usingPreview) {
        this->cubeDisplaySource->SetOutput(preview.cubeImageData);
        this->momentDisplaySource->SetOutput(preview.momentImageData);
        this->lowerBound = 3.f * preview.cubeRms;
        this->upperBound = preview.cubeRange[1];
    } else {
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

        ui->vtkCube->renderWindow()->Render();
    });
    if (usingPreview) {
        this->applyCubeOpenResult(preview);
    } else {
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
    this->viewController = std::make_unique<CubeViewController>(CubeViewContext {
        this->cubeDisplaySource,
        this->astro,
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
    QObject::connect(&this->cubeOpenWatcher, &QFutureWatcher<CubeOpenStageResult>::finished, this,
                     [this]() {
                         this->setCubeOpenActionsEnabled(true);

                         const auto result = this->cubeOpenWatcher.result();
                         if (!result.valid || !result.cubeImageData || !result.momentImageData) {
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
                         this->applyCubeOpenResult(result);
                         ++this->currentFullCubeGeneration;
                         this->scheduleIsosurfacePrewarm();
                         this->setCubeOpenStateLabel({ });
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

                         this->applyMomentMapResult(
                                 { result.imageData, { result.imageRange[0], result.imageRange[1] } });
                         this->clearPersistentStatusMessage();
                     });
    QObject::connect(&this->isosurfaceWatcher, &QFutureWatcher<AsyncIsosurfaceResult>::finished,
                     this, [this]() {
                         const auto result = this->isosurfaceWatcher.result();
                         if (result.requestId != this->currentIsosurfaceRequestId) {
                             return;
                         }

                         if (!result.mesh || result.mesh->GetNumberOfPoints() == 0) {
                             return;
                         }

                         vtkNew<vtkPolyDataMapper> mapper;
                         mapper->SetInputData(result.mesh);
                         mapper->ScalarVisibilityOff();

                         vtkNew<vtkActor> newActor;
                         newActor->SetMapper(mapper);
                         if (this->currentIsosurfaceActor) {
                             newActor->GetProperty()->DeepCopy(
                                     this->currentIsosurfaceActor->GetProperty());
                         } else {
                             newActor->GetProperty()->SetColor(1., 0.5, 1.);
                         }

                         auto *renderer =
                                 ui->vtkCube->renderWindow()->GetRenderers()->GetFirstRenderer();
                         if (this->currentIsosurfaceActor
                             && renderer->HasViewProp(this->currentIsosurfaceActor)) {
                             renderer->RemoveActor(this->currentIsosurfaceActor);
                         }

                         this->currentIsosurfaceActor = newActor;
                         this->setCubeRenderModeLocally(ui->actionIsosurface->isChecked());
                         ui->vtkCube->renderWindow()->Render();
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
    ui->menuWCS->setEnabled(!this->astro.isSimulation());
    QObject::connect(groupWCS, &QActionGroup::triggered, this, &vtkWindowCube::changeLegendWCS);

    // Setup menu Tools
    QObject::connect(ui->actionExtractSpectrum, &QAction::triggered, this,
                     &vtkWindowCube::setInteractorStyleProfile);

    // Setup Threshold UI
    const std::string bunit = astro.getPhysicalUnit();
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
    const std::string unit = astro.getAxisUnit(2);
    if (!unit.empty()) {
        ui->groupSlice->setTitle(u"Cutting plane (%1)"_s.arg(QString::fromStdString(unit)));
    }
    ui->lineSpectral->setText(QString::number(this->astro.getInitialSpectralValue()));
    QObject::connect(ui->sliderSlice, &QSlider::actionTriggered, this,
                     &vtkWindowCube::sliceSliderChanged);
    QObject::connect(ui->spinSlice, &QSpinBox::valueChanged, this,
                     &vtkWindowCube::sliceSpinChanged);

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
    if (usingPreview) {
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
        if (this->cubeOpenWatcher.isRunning()) {
            this->showPersistentStatusMessage(u"Loading full resolution..."_s);
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
    ren->AddViewProp(this->isosurface);
    this->currentIsosurfaceActor = this->isosurface;

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

    // Slice
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
    this->slice->SetInputConnection(this->cubeDisplaySource->GetOutputPort());
    this->slice->SetResliceAxesOrigin(0., 0., 0.);
    this->slice->SetOutputDimensionality(2);
    this->slice->Update();
    this->lutSlice->SetTableRange(this->slice->GetOutput()->GetScalarRange());
    this->lutSlice->SetNanColor(1., 1., 1., 1.);
    ColorMaps::SetColorMap(this->lutSlice);
    vtkNew<vtkImageMapToColors> colors;
    colors->SetInputConnection(this->slice->GetOutputPort());
    colors->SetLookupTable(this->lutSlice);
    vtkNew<vtkImageSliceMapper> sliceMapper;
    sliceMapper->SetInputConnection(colors->GetOutputPort());
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
    this->contours->SetInputConnection(this->slice->GetOutputPort());
    this->contours->GenerateValues(this->level, this->lowerBound, this->upperBound);
    vtkNew<vtkPolyDataMapper> contoursMapper;
    contoursMapper->SetInputConnection(this->contours->GetOutputPort());
    contoursMapper->SetScalarRange(this->lowerBound, this->upperBound);
    this->contoursActor->SetMapper(contoursMapper);
    this->contoursActor->VisibilityOff();
    ren->AddViewProp(this->contoursActor);

    // Legend
    this->legendSlice->Init(this->filepath.toStdString());
    this->legendSlice->SetWCS(WCS_GALACTIC);
    ren->AddViewProp(this->legendSlice);

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
    if (!vtkImageData::SafeDownCast(this->momentDisplaySource->GetOutputDataObject(0))) {
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
    vtkNew<vtkImageMapToColors> colors;
    colors->SetInputConnection(this->momentDisplaySource->GetOutputPort());
    colors->SetLookupTable(this->lutMoment);
    vtkNew<vtkImageSliceMapper> momentMapper;
    momentMapper->SetInputConnection(colors->GetOutputPort());
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
    this->legendMoment->Init(this->filepath.toStdString());
    this->legendMoment->SetWCS(WCS_GALACTIC);
    ren->AddViewProp(this->legendMoment);

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
            ? this->slice->GetOutput()
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

    if (!astro.isSimulation()) {
        double wcs[2];
        astro.xy2sky(worldCoord, wcs, WCS_GALACTIC);
        ss << "  <galactic> GLON: " << wcs[0] << " GLAT: " << wcs[1];

        astro.xy2sky(worldCoord, wcs, WCS_J2000);
        ss << "  <fk5> RA: " << wcs[0] << " Dec: " << wcs[1];

        astro.xy2sky(worldCoord, wcs, WCS_ECLIPTIC);
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
    this->viewController->updateCube(threshold);
    if (!this->cubeOpenWatcher.isRunning() && ui->actionIsosurface->isChecked()) {
        this->scheduleIsosurfaceRecompute();
    }
    ui->vtkCube->renderWindow()->Render();
}

void vtkWindowCube::applyCubeOpenResult(const CubeOpenStageResult &result)
{
    if (!result.cubeImageData) {
        return;
    }

    this->cubeDisplaySource->SetOutput(result.cubeImageData);
    if (result.momentImageData) {
        this->momentDisplaySource->SetOutput(result.momentImageData);
        this->lutMoment->SetTableRange(result.momentRange[0], result.momentRange[1]);
    }

    const int clampedSlice = std::clamp(ui->spinSlice->value() - 1, result.dataExtent[4],
                                        result.dataExtent[5]);
    {
        const QSignalBlocker blockSlider(ui->sliderSlice);
        const QSignalBlocker blockSpin(ui->spinSlice);
        ui->sliderSlice->setMaximum(result.dataExtent[5] + 1);
        ui->spinSlice->setMaximum(result.dataExtent[5] + 1);
        ui->sliderSlice->setValue(clampedSlice + 1);
        ui->spinSlice->setValue(clampedSlice + 1);
    }

    int sliceExtent[6] = { result.dataExtent[0], result.dataExtent[1], result.dataExtent[2],
                           result.dataExtent[3], clampedSlice, clampedSlice };
    this->sliceOnCube->SetVOI(sliceExtent);
    this->sliceOnCube->Update();
    this->slice->SetResliceAxesOrigin(0., 0., clampedSlice);
    this->slice->Update();

    const double *sliceRange = this->slice->GetOutput()->GetScalarRange();
    this->lutSlice->SetTableRange(sliceRange);

    const double *sliceOnCubeRange = this->sliceOnCube->GetOutput()->GetScalarRange();
    this->lutSliceOnCube->SetTableRange(sliceOnCubeRange);

    ui->lineCubeMin->setText(QString::number(result.cubeRange[0]));
    ui->lineCubeMax->setText(QString::number(result.cubeRange[1]));
    ui->lineCubeMean->setText(QString::number(result.cubeMean));
    ui->lineCubeRms->setText(QString::number(result.cubeRms));
    ui->lineSpectral->setText(QString::number(this->astro.getInitialSpectralValue()
                                              + this->astro.getIncrements()[2] * clampedSlice));

    auto cubeRenderer = ui->vtkCube->renderWindow()->GetRenderers()->GetFirstRenderer();
    cubeRenderer->ResetCamera();
    cubeRenderer->ResetCameraClippingRange();

    auto sliceRenderer = this->sliceWin->GetRenderers()->GetFirstRenderer();
    sliceRenderer->ResetCamera();
    sliceRenderer->ResetCameraClippingRange();

    auto momentRenderer = this->momentWin->GetRenderers()->GetFirstRenderer();
    momentRenderer->ResetCamera();
    momentRenderer->ResetCameraClippingRange();

    if (this->viewingSlice()) {
        ui->lineImgMin->setText(QString::number(sliceRange[0]));
        ui->lineImgMax->setText(QString::number(sliceRange[1]));
        this->updateLUTCustomizer();
    } else if (result.momentImageData) {
        ui->lineImgMin->setText(QString::number(result.momentRange[0]));
        ui->lineImgMax->setText(QString::number(result.momentRange[1]));
        this->updateLUTCustomizer();
    }

    ui->vtkCube->renderWindow()->Render();
    ui->vtkImage->renderWindow()->Render();
}

void vtkWindowCube::updateSlice()
{
    const int slice = ui->spinSlice->value() - 1;
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

void vtkWindowCube::updateContoursVisibility()
{
    this->viewController->setContoursVisible(ui->checkContours->isChecked());
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
    this->momentComputeWatcher.setFuture(
            QtConcurrent::run(&computeMomentMap, MomentMapComputeRequest { this->filepath, order }));
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
    this->viewController->updateContours(ui->lineLevel->text().toInt(),
                                         ui->lineLowerBound->text().toDouble(),
                                         ui->lineUpperBound->text().toDouble());
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
    if (this->cubeOpenWatcher.isRunning()) {
        const QSignalBlocker blockSpin(ui->spinSlice);
        ui->spinSlice->setValue(ui->sliderSlice->sliderPosition());
        return;
    }

    ui->spinSlice->setValue(ui->sliderSlice->sliderPosition());
    // updateSlice is called by spinSlice
}

void vtkWindowCube::sliceSpinChanged(int value)
{
    if (this->cubeOpenWatcher.isRunning()) {
        const QSignalBlocker blockSlider(ui->sliderSlice);
        ui->sliderSlice->setValue(value);
        return;
    }

    ui->sliderSlice->setValue(value);
    this->updateSlice();
}

void vtkWindowCube::changeLegendWCS()
{
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
        this->lutCustomizer->init(this->slice->GetOutput(), this->lutSlice);
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

    this->momentDisplaySource->SetOutput(result.imageData);
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
    ui->vtkImage->renderWindow()->Render();
}

bool vtkWindowCube::isBusy() const
{
    return this->cubeOpenWatcher.isRunning() || this->momentComputeWatcher.isRunning();
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
    ui->actionExtractSpectrum->setEnabled(enabled);
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
        this->sliceOnCube->GetOutput()->GetScalarRange(imgRange);
    } else {
        ui->vtkImage->setRenderWindow(this->momentWin);
        ui->labelImg->setText(u"Moment:"_s);
        vtkImageData::SafeDownCast(this->momentDisplaySource->GetOutputDataObject(0))
                ->GetScalarRange(imgRange);
    }

    ui->lineImgMin->setText(QString::number(imgRange[0]));
    ui->lineImgMax->setText(QString::number(imgRange[1]));
    this->coordinate->SetViewport(ui->vtkImage->renderWindow()->GetRenderers()->GetFirstRenderer());
    ui->vtkImage->renderWindow()->Render();
}

void vtkWindowCube::changeCubeRender()
{
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
    this->viewController->syncSlicesLut();
    ui->vtkCube->renderWindow()->Render();
}

void vtkWindowCube::startAsyncIsosurface(double isoValue)
{
    if (this->isosurfaceWatcher.isRunning()) {
        return;
    }

    auto *source = vtkImageData::SafeDownCast(this->cubeDisplaySource->GetOutputDataObject(0));
    if (!source) {
        return;
    }

    vtkSmartPointer<vtkImageData> data = vtkSmartPointer<vtkImageData>::New();
    data->DeepCopy(source);

    const int requestId = ++this->currentIsosurfaceRequestId;
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

    if (isosurfaceMode) {
        if (this->currentIsosurfaceActor && !renderer->HasViewProp(this->currentIsosurfaceActor)) {
            renderer->AddActor(this->currentIsosurfaceActor);
        }
        if (renderer->HasViewProp(this->volume)) {
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
