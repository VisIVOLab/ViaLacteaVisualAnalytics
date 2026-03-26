#include "vtkWindowImage.h"
#include "ui_vtkWindowImage.h"

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
#include <vtkCoordinate.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageStack.h>
#include <vtkInteractorStyleImage.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>

#include <QActionGroup>
#include <QButtonGroup>
#include <QFileDialog>
#include <QMessageBox>
#include <QtConcurrentRun>

#include <sstream>

using namespace Qt::StringLiterals;

vtkWindowImage::vtkWindowImage(const QString &filepath, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::vtkWindowImage),
      filepath(filepath),
      astro(filepath.toStdString()),
      lutCustomizer(nullptr),
      profileWidget(nullptr),
      importService(std::make_unique<ImageLayerImportService>())
{
    ui->setupUi(this);
    this->setWindowTitle(this->filepath);
    this->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(&this->layerLoadWatcher, &QFutureWatcher<ImageLayerLoadResult>::finished, this,
                     [this]() {
                         this->setLayerImportEnabled(true);

                         const auto result = this->layerLoadWatcher.result();
                         if (!result.valid) {
                             if (!result.errorMessage.empty()) {
                                 QMessageBox::warning(this, u"Import FITS file"_s,
                                                      QString::fromStdString(result.errorMessage));
                             }
                             this->statusBar()->clearMessage();
                             return;
                         }

                         this->applyLoadedLayer(result);
                         this->statusBar()->clearMessage();
                     });

    this->setupRenderer();

    // Setup Menu File
    QObject::connect(ui->actionAddFITS, &QAction::triggered, this, &vtkWindowImage::addLocalFile);

    // Setup Menu WCS
    auto groupWCS = new QActionGroup(this);
    groupWCS->addAction(ui->actionGalactic);
    groupWCS->addAction(ui->actionFK5);
    groupWCS->addAction(ui->actionEcliptic);
    QObject::connect(groupWCS, &QActionGroup::triggered, this, &vtkWindowImage::changeLegendWCS);

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
}

vtkWindowImage::~vtkWindowImage()
{
    delete ui;
}

void vtkWindowImage::closeEvent(QCloseEvent *event)
{
    if (this->isBusy()) {
        this->statusBar()->showMessage(u"Loading layer..."_s);
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
    this->legendWCS->Init(this->filepath.toStdString());
    this->legendWCS->SetWCS(WCS_GALACTIC);
    ren->AddViewProp(this->legendWCS);

    ren->ResetCamera();
    win->Render();
}

void vtkWindowImage::mouseCallback()
{
    const int *position = ui->vtk->renderWindow()->GetInteractor()->GetEventPosition();
    this->coordinate->SetValue(position[0], position[1]);
    const double *worldCoord = this->coordinate->GetComputedWorldValue(nullptr);
    const long imageCoord[2] = { std::lround(worldCoord[0]), std::lround(worldCoord[1]) };

    std::ostringstream ss;
    ss << "<value> "
       << this->layers->getPixelValue(this->layers->getMasterIndex(), imageCoord[0], imageCoord[1]);
    ss << "  <image> X: " << worldCoord[0] << " Y: " << worldCoord[1];

    if (!this->astro.isSimulation()) {
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
    this->statusBar()->showMessage(u"Loading layer..."_s);
    this->layerLoadWatcher.setFuture(QtConcurrent::run(
            &loadImageLayer, ImageLayerLoadRequest { this->filepath.toStdString(), filepath }));
}

void vtkWindowImage::applyLoadedLayer(const ImageLayerLoadResult &result)
{
    this->stack->AddImage(this->layers->addLayer(result));
    this->vtkRender();
}

bool vtkWindowImage::isBusy() const
{
    return this->layerLoadWatcher.isRunning();
}

void vtkWindowImage::setLayerImportEnabled(bool enabled)
{
    ui->actionAddFITS->setEnabled(enabled);
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
    this->legendWCS->SetWCS(wcs);
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
