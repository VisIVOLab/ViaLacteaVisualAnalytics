#include "vtkWindowImage.h"
#include "ui_vtkWindowImage.h"

#include "ColorMaps.h"
#include "LayerListModel.h"
#include "wcs.h"

#include <vtkCoordinate.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageStack.h>
#include <vtkInteractorStyleImage.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>

#include <QButtonGroup>
#include <QFileDialog>

#include <sstream>

using namespace Qt::StringLiterals;

vtkWindowImage::vtkWindowImage(const QString &filepath, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::vtkWindowImage),
      filepath(filepath),
      astro(filepath.toStdString())
{
    ui->setupUi(this);
    this->setWindowTitle(this->filepath);
    this->setAttribute(Qt::WA_DeleteOnClose);

    this->setupRenderer();

    // Color Maps Combobox
    auto cmaps = ColorMaps::GetColorMapNames();
    std::for_each(cmaps.cbegin(), cmaps.cend(), [this](const std::string &name) {
        ui->comboLut->addItem(QString::fromStdString(name));
    });
    ui->comboLut->setCurrentText(QString::fromStdString(ColorMaps::DefaultColorMap));
    QObject::connect(ui->comboLut, &QComboBox::textActivated, this,
                     &vtkWindowImage::changeCurrentColorMap);

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
}

vtkWindowImage::~vtkWindowImage()
{
    delete ui;
}

void vtkWindowImage::setupRenderer()
{
    vtkNew<vtkRenderer> ren;
    ren->SetBackground(0.21, 0.23, 0.25);

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

int vtkWindowImage::currentLayerIndex() const
{
    return ui->listLayer->currentIndex().row();
}

void vtkWindowImage::addLayerImage(const std::string &filepath)
{
    this->stack->AddImage(this->layers->addLayer(filepath));
}

void vtkWindowImage::vtkRender()
{
    ui->vtk->renderWindow()->Render();
}

void vtkWindowImage::changeCurrentColorMap()
{
    this->layers->setColorMap(this->currentLayerIndex(), ui->comboLut->currentText().toStdString());
}

void vtkWindowImage::changeCurrentColorScale()
{
    this->layers->setLogScale(this->currentLayerIndex(), ui->radioLog->isChecked());
}

void vtkWindowImage::changeCurrentLayerOpacity()
{
    const double opacity = ui->sliderOpacity->sliderPosition() / 100.;
    this->layers->setLayerOpacity(ui->listLayer->currentIndex().row(), opacity);
}

void vtkWindowImage::showCurrentLayerSettings()
{
    const int index = this->currentLayerIndex();
    const int opacity = this->layers->getLayerOpacity(index) * 100;
    ui->comboLut->setCurrentText(QString::fromStdString(this->layers->getColorMapName(index)));
    if (this->layers->usingLogScale(index)) {
        ui->radioLog->setChecked(true);
    } else {
        ui->radioLinear->setChecked(true);
    }
    ui->sliderOpacity->setValue(opacity);

    this->stack->SetActiveLayer(index);
    this->colorbar->SetLookupTable(this->layers->getLookupTable(index));
    this->vtkRender();
}
