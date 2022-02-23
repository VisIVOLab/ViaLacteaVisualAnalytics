#include "vtkwindowcube.h"
#include "ui_vtkwindowcube.h"

#include "luteditor.h"
#include "vtkfitsreader.h"
#include "vtklegendscaleactor.h"

#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCaptionActor2D.h>
#include <vtkFlyingEdges3D.h>
#include <vtkFrustumSource.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageActor.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkLODActor.h>
#include <vtkLookupTable.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkOutlineFilter.h>
#include <vtkPlanes.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkResliceImageViewer.h>
#include <vtkTextActor.h>

vtkWindowCube::vtkWindowCube(QWidget *parent, vtkSmartPointer<vtkFitsReader> fitsReader,
                             QString velocityUnit)
    : QMainWindow(parent),
      ui(new Ui::vtkWindowCube),
      fitsReader(fitsReader),
      currentSlice(0),
      velocityUnit(velocityUnit)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(QString::fromStdString(fitsReader->GetFileName()));

    fitsReader->is3D = true;
    fitsReader->CalculateRMS();

    // Start datacube pipeline
    auto renWinCube = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->qVtkCube->setDefaultCursor(Qt::ArrowCursor);
    ui->qVtkCube->setRenderWindow(renWinCube);

    auto rendererCube = vtkSmartPointer<vtkRenderer>::New();
    rendererCube->SetBackground(0.21, 0.23, 0.25);
    rendererCube->GlobalWarningDisplayOff();
    renWinCube->AddRenderer(rendererCube);

    double threshold = 3 * fitsReader->GetRMS();

    ui->thresholdText->setText(QString::number(threshold, 'f', 4));
    ui->lowerBoundText->setText(QString::number(threshold, 'f', 4));
    ui->upperBoundText->setText(QString::number(fitsReader->GetMax(), 'f', 4));
    ui->minCubeText->setText(QString::number(fitsReader->GetMin(), 'f', 4));
    ui->maxCubeText->setText(QString::number(fitsReader->GetMax(), 'f', 4));
    ui->rmsCubeText->setText(QString::number(fitsReader->GetRMS(), 'f', 4));

    // Outline
    auto outlineFilter = vtkSmartPointer<vtkOutlineFilter>::New();
    outlineFilter->SetInputData(fitsReader->GetOutput());
    auto outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    outlineMapper->SetInputConnection(outlineFilter->GetOutputPort());
    outlineMapper->ScalarVisibilityOff();
    auto outlineActor = vtkSmartPointer<vtkActor>::New();
    outlineActor->SetMapper(outlineMapper);

    // Isosurface
    auto isosurface = vtkSmartPointer<vtkFlyingEdges3D>::New();
    isosurface->SetInputData(fitsReader->GetOutput());
    isosurface->ComputeNormalsOn();
    isosurface->SetValue(0, threshold);
    auto isosurfaceMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    isosurfaceMapper->SetInputConnection(isosurface->GetOutputPort());
    isosurfaceMapper->ScalarVisibilityOff();
    auto isosurfaceActor = vtkSmartPointer<vtkActor>::New();
    isosurfaceActor->SetMapper(isosurfaceMapper);
    isosurfaceActor->GetProperty()->SetColor(1.0, 0.5, 1.0);

    // Plane
    double *bounds = fitsReader->GetOutput()->GetBounds();
    auto planes = vtkSmartPointer<vtkPlanes>::New();
    planes->SetBounds(bounds[0], bounds[1], bounds[2], bounds[3], 0, 1);
    auto frustumSource = vtkSmartPointer<vtkFrustumSource>::New();
    frustumSource->ShowLinesOff();
    frustumSource->SetPlanes(planes);
    frustumSource->Update();
    auto planeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    planeMapper->SetInputData(frustumSource->GetOutput());
    planeActor = vtkSmartPointer<vtkActor>::New();
    planeActor->SetMapper(planeMapper);

    // Axes
    auto axesActor = vtkSmartPointer<vtkAxesActor>::New();
    axesActor->SetXAxisLabelText("X");
    axesActor->SetYAxisLabelText("Y");
    axesActor->SetZAxisLabelText("Z");
    axesActor->DragableOn();
    axesActor->GetXAxisCaptionActor2D()->GetTextActor()->SetTextScaleMode(0);
    axesActor->GetYAxisCaptionActor2D()->GetTextActor()->SetTextScaleMode(0);
    axesActor->GetZAxisCaptionActor2D()->GetTextActor()->SetTextScaleMode(0);
    axesWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    axesWidget->SetInteractor(renWinCube->GetInteractor());
    axesWidget->SetOrientationMarker(axesActor);
    axesWidget->SetOutlineColor(0.9300, 0.5700, 0.1300);
    axesWidget->SetViewport(0.0, 0.0, 0.2, 0.2);
    axesWidget->SetEnabled(1);
    axesWidget->InteractiveOff();

    // Legend
    auto legendActorCube = vtkSmartPointer<vtkLegendScaleActor>::New();
    legendActorCube->LegendVisibilityOff();
    legendActorCube->setFitsFile(fitsReader);

    rendererCube->AddActor(outlineActor);
    rendererCube->AddActor(isosurfaceActor);
    rendererCube->AddActor(planeActor);
    rendererCube->AddActor(legendActorCube);

    rendererCube->ResetCamera();
    renWinCube->GetInteractor()->Render();
    // End datacube pipeline

    // Start slice pipeline
    auto renWinSlice = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->qVtkSlice->setDefaultCursor(Qt::ArrowCursor);
    ui->qVtkSlice->setRenderWindow(renWinSlice);

    auto rendererSlice = vtkSmartPointer<vtkRenderer>::New();
    rendererSlice->SetBackground(0.21, 0.23, 0.25);
    rendererSlice->GlobalWarningDisplayOff();
    renWinSlice->AddRenderer(rendererSlice);

    auto lutSlice = vtkSmartPointer<vtkLookupTable>::New();
    lutSlice->SetTableRange(fitsReader->GetRangeSlice(0)[0], fitsReader->GetRangeSlice(0)[1]);
    SelectLookTable("Gray", lutSlice);

    /// \todo Interactor Image Contour

    sliceViewer = vtkSmartPointer<vtkResliceImageViewer>::New();
    sliceViewer->SetInputData(fitsReader->GetOutput());
    sliceViewer->GetWindowLevel()->SetOutputFormatToRGB();
    sliceViewer->GetWindowLevel()->SetLookupTable(lutSlice);
    sliceViewer->GetImageActor()->InterpolateOff();
    sliceViewer->SetRenderWindow(renWinSlice);
    sliceViewer->SetRenderer(rendererSlice);

    ui->sliceSlider->setRange(1, fitsReader->GetNaxes(2));
    ui->sliceSpinBox->setRange(1, fitsReader->GetNaxes(2));

    auto legendActorSlice = vtkSmartPointer<vtkLegendScaleActor>::New();
    legendActorSlice->LegendVisibilityOff();
    legendActorSlice->setFitsFile(fitsReader);
    rendererSlice->AddActor(legendActorSlice);

    rendererSlice->ResetCamera();
    renWinSlice->GetInteractor()->Render();
}

vtkWindowCube::~vtkWindowCube()
{
    delete ui;
}

void vtkWindowCube::on_sliceSlider_valueChanged(int value)
{
    ui->sliceSpinBox->setValue(value);
    currentSlice = value - 1;
    updateVelocityText();
    updateSliceDatacube();
}

void vtkWindowCube::on_sliceSpinBox_valueChanged(int value)
{
    ui->sliceSlider->setValue(value);
}

void vtkWindowCube::updateSliceDatacube()
{
    planeActor->SetPosition(0, 0, currentSlice + 1);

    float *range = fitsReader->GetRangeSlice(currentSlice);
    ui->minSliceText->setText(QString::number(range[0], 'f', 4));
    ui->maxSliceText->setText(QString::number(range[1], 'f', 4));

    auto lutSlice = vtkSmartPointer<vtkLookupTable>::New();
    lutSlice->SetTableRange(range[0], range[1]);
    SelectLookTable("Gray", lutSlice);

    sliceViewer->GetWindowLevel()->SetLookupTable(lutSlice);
    sliceViewer->SetSlice(currentSlice);

    sliceViewer->GetRenderer()->ResetCamera();
    sliceViewer->Render();
    ui->qVtkCube->renderWindow()->GetInteractor()->Render();
}

void vtkWindowCube::updateVelocityText()
{
    double velocity = fitsReader->getInitSlice() + fitsReader->GetCdelt(2) * currentSlice;
    if (velocityUnit.startsWith("m")) {
        // Return value in km/s
        velocity /= 1000;
    }
    ui->velocityText->setText(QString::number(velocity).append(" Km/s"));
}
