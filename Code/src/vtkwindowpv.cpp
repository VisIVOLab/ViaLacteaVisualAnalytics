#include "vtkwindowpv.h"
#include "ui_vtkwindowpv.h"

#include "luteditor.h"
#include "vtkfitsreader2.h"

#include <vtkAxisActor2D.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageMapToColors.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkInteractorStyleImage.h>
#include <vtkLegendScaleActor.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkWindowToImageFilter.h>

#include <QFileDialog>
#include <QMessageBox>

vtkWindowPV::vtkWindowPV(const QString &filepath, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::vtkWindowPV), filepath(filepath)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(filepath);

    vtkNew<vtkGenericOpenGLRenderWindow> renderWin;
    ui->qVtk->setRenderWindow(renderWin);

    vtkNew<vtkRenderer> renderer;
    renderer->SetBackground(0.21, 0.23, 0.25);
    renderWin->AddRenderer(renderer);

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
