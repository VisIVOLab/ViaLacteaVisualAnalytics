#include "vtkwindowpv.h"
#include "ui_vtkwindowpv.h"

#include "astroutils.h"
#include "luteditor.h"

#include <vtkCoordinate.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageMapToColors.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkInteractorStyleImage.h>
#include <vtkLegendScaleActor.h>
#include <vtkLookupTable.h>
#include <vtkPNGWriter.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkScalarBarActor.h>
#include <vtkWindowToImageFilter.h>

#include <QFileDialog>
#include <QMessageBox>

#include <sstream>

vtkWindowPV::vtkWindowPV(const QString &filepath, const QString &cubepath, float x1, float y1,
                         float x2, float y2, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::vtkWindowPV),
      filepath(filepath),
      cubepath(cubepath),
      x1(x1),
      x2(x2),
      m((y2 - y1) / (x2 - x1)),
      q(y1 - m * x1)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(filepath);

    vtkNew<vtkGenericOpenGLRenderWindow> renderWin;
    ui->qVtk->setRenderWindow(renderWin);

    vtkNew<vtkRenderer> renderer;
    renderer->SetBackground(0.21, 0.23, 0.25);
    renderWin->AddRenderer(renderer);

    coordinate->SetCoordinateSystemToDisplay();

    reader->SetFileName(filepath.toStdString().c_str());
    reader->Update();

    float range[2];
    reader->GetValueRange(range);

    lut->SetTableRange(range[0], range[1]);
    lut->SetScaleToLinear();
    SelectLookTable(ui->comboLut->currentText(), lut);

    vtkNew<vtkScalarBarActor> colorBar;
    colorBar->SetLookupTable(lut);
    colorBar->SetBarRatio(0.1);
    colorBar->UnconstrainedFontSizeOn();

    vtkNew<vtkImageMapToColors> scalarsToColors;
    scalarsToColors->SetInputConnection(reader->GetOutputPort());
    scalarsToColors->SetLookupTable(lut);

    vtkNew<vtkImageSliceMapper> imageMapper;
    imageMapper->SetInputConnection(scalarsToColors->GetOutputPort());

    image->SetMapper(imageMapper);
    image->GetProperty()->SetInterpolationTypeToNearest();

    vtkNew<vtkLegendScaleActor> legend;
    legend->SetLabelModeToXYCoordinates();
    legend->RightAxisVisibilityOff();
    legend->LeftAxisVisibilityOff();
    legend->TopAxisVisibilityOff();
    legend->LegendVisibilityOff();

    vtkNew<vtkInteractorStyleImage> interactor;
    renderWin->GetInteractor()->SetInteractorStyle(interactor);
    renderWin->GetInteractor()->AddObserver(vtkCommand::MouseMoveEvent, this,
                                            &vtkWindowPV::coordsCallback);

    renderer->AddActor2D(colorBar);
    renderer->AddViewProp(image);
    renderer->AddActor(legend);
    renderer->ResetCamera();
    renderWin->Render();

    connect(ui->radioLinear, &QRadioButton::toggled, this, &vtkWindowPV::changeLutScale);
    connect(ui->sliderOpacity, &QSlider::valueChanged, this, &vtkWindowPV::changeOpacity);
    connect(ui->comboLut, &QComboBox::currentTextChanged, this, &vtkWindowPV::changeLut);
    connect(ui->actionSaveImage, &QAction::triggered, this, &vtkWindowPV::saveAsImage);
}

vtkWindowPV::~vtkWindowPV()
{
    delete ui;
}

void vtkWindowPV::coordsCallback()
{
    int *pos = ui->qVtk->renderWindow()->GetInteractor()->GetEventPosition();
    this->coordinate->SetValue(pos[0], pos[1]);
    double *world_coord = this->coordinate->GetComputedWorldValue(
            ui->qVtk->renderWindow()->GetRenderers()->GetFirstRenderer());

    std::ostringstream oss;
    // Pixel value and coords
    float *pixel = this->reader->GetPixelValue(world_coord[0], world_coord[1], 0);
    oss << "<value> ";
    if (pixel != NULL)
        oss << pixel[0];
    else
        oss << "NaN";
    oss << " <image> X: " << world_coord[0] << " Y: " << world_coord[1];

    // Original coordinates in parent cube
    world_coord[0] = (x1 < x2) ? world_coord[0] + x1 : x1 - world_coord[0];
    world_coord[1] = world_coord[0] * m + q;

    double sky_coord[2];
    double sky_coord_gal[2];
    double sky_coord_fk5[2];

    std::string filepath(cubepath.toStdString());
    // Galactic coords
    AstroUtils::xy2sky(filepath, world_coord[0], world_coord[1], sky_coord_gal, WCS_GALACTIC);
    oss << " <galactic> GLON: " << sky_coord_gal[0] << " GLAT: " << sky_coord_gal[1];

    // fk5 coords
    AstroUtils::xy2sky(filepath, world_coord[0], world_coord[1], sky_coord_fk5, WCS_J2000);
    oss << " <fk5> RA: " << sky_coord_fk5[0] << " DEC: " << sky_coord_fk5[1];

    // Ecliptic coords
    AstroUtils::xy2sky(filepath, world_coord[0], world_coord[1], sky_coord);
    oss << " <ecliptic> RA: " << sky_coord[0] << " DEC: " << sky_coord[1];

    ui->statusbar->showMessage(QString::fromStdString(oss.str()));
}

void vtkWindowPV::changeLutScale()
{
    float range[2];
    reader->GetValueRange(range);

    if (ui->radioLinear->isChecked()) {
        lut->SetScaleToLinear();
        lut->SetTableRange(range[0], range[1]);
    } else {
        lut->SetTableRange(1e-1, range[1]);
        lut->SetScaleToLog10();
    }

    ui->qVtk->renderWindow()->Render();
}

void vtkWindowPV::changeOpacity(int opacity)
{
    image->GetProperty()->SetOpacity(opacity / 100.0);
    ui->qVtk->renderWindow()->Render();
}

void vtkWindowPV::changeLut(const QString &lutName)
{
    SelectLookTable(lutName, lut);
    ui->qVtk->renderWindow()->Render();
}

void vtkWindowPV::saveAsImage()
{
    QString filepath =
            QFileDialog::getSaveFileName(this, "Save as PNG...", QString(), "PNG image (*.png)");
    if (filepath.isEmpty()) {
        return;
    }

    vtkNew<vtkWindowToImageFilter> filter;
    filter->SetInput(ui->qVtk->renderWindow());
    filter->SetScale(2);
    filter->Update();

    vtkNew<vtkPNGWriter> writer;
    writer->SetFileName(filepath.toStdString().c_str());
    writer->SetInputConnection(filter->GetOutputPort());
    writer->Write();

    ui->qVtk->renderWindow()->Render();

    QMessageBox::information(this, "Image saved", "Image saved: " + filepath);
}
