#include "vtkwindowcube.h"
#include "ui_vtkwindowcube.h"

#include "interactors/vtkinteractorstyleimagecustom.h"

#include "luteditor.h"
#include "vtkfitsreader.h"
#include "vtklegendscaleactor.h"

#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCaptionActor2D.h>
#include <vtkContourFilter.h>
#include <vtkCutter.h>
#include <vtkFlyingEdges3D.h>
#include <vtkFrustumSource.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageActor.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkLODActor.h>
#include <vtkLookupTable.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkOutlineFilter.h>
#include <vtkPlane.h>
#include <vtkPlanes.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkResliceImageViewer.h>
#include <vtkTextActor.h>

#include <cmath>

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

    lowerBound = 3 * fitsReader->GetRMS();
    upperBound = fitsReader->GetMax();

    // Start datacube pipeline
    auto renWinCube = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->qVtkCube->setDefaultCursor(Qt::ArrowCursor);
    ui->qVtkCube->setRenderWindow(renWinCube);

    auto rendererCube = vtkSmartPointer<vtkRenderer>::New();
    rendererCube->SetBackground(0.21, 0.23, 0.25);
    rendererCube->GlobalWarningDisplayOff();
    renWinCube->AddRenderer(rendererCube);

    ui->thresholdText->setText(QString::number(lowerBound, 'f', 4));
    ui->lowerBoundText->setText(QString::number(lowerBound, 'f', 4));
    ui->upperBoundText->setText(QString::number(upperBound, 'f', 4));
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
    isosurface = vtkSmartPointer<vtkFlyingEdges3D>::New();
    isosurface->SetInputData(fitsReader->GetOutput());
    isosurface->ComputeNormalsOn();
    isosurface->SetValue(0, lowerBound);
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

    rendererCube->GetActiveCamera()->GetPosition(initialCameraPosition);
    rendererCube->GetActiveCamera()->GetFocalPoint(initialCameraFocalPoint);

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

    sliceViewer = vtkSmartPointer<vtkResliceImageViewer>::New();
    sliceViewer->SetInputData(fitsReader->GetOutput());
    sliceViewer->GetWindowLevel()->SetOutputFormatToRGB();
    sliceViewer->GetWindowLevel()->SetLookupTable(lutSlice);
    sliceViewer->GetImageActor()->InterpolateOff();
    sliceViewer->SetRenderWindow(renWinSlice);
    sliceViewer->SetRenderer(rendererSlice);

    ui->sliceSlider->setRange(1, fitsReader->GetNaxes(2));
    ui->sliceSpinBox->setRange(1, fitsReader->GetNaxes(2));

    // Added to renderer when the contour checkbox is checked
    contoursActor = vtkSmartPointer<vtkLODActor>::New();
    contoursActor->GetProperty()->SetLineWidth(1);

    auto legendActorSlice = vtkSmartPointer<vtkLegendScaleActor>::New();
    legendActorSlice->LegendVisibilityOff();
    legendActorSlice->setFitsFile(fitsReader);
    rendererSlice->AddActor(legendActorSlice);

    auto interactorStyle = vtkSmartPointer<vtkInteractorStyleImageCustom>::New();
    interactorStyle->SetCoordsCallback([this](std::string str) { showStatusBarMessage(str); });
    interactorStyle->SetLayerFitsReaderFunc(
            [this]() -> vtkSmartPointer<vtkFitsReader> { return this->fitsReader; });
    interactorStyle->SetPixelZCompFunc([this]() -> int { return this->sliceViewer->GetSlice(); });
    ui->qVtkSlice->renderWindow()->GetInteractor()->SetInteractorStyle(interactorStyle);

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
    if (ui->contourCheckBox->isChecked()) {
        showContours();
    }
}

void vtkWindowCube::on_sliceSpinBox_valueChanged(int value)
{
    ui->sliceSlider->setValue(value);
}

void vtkWindowCube::showStatusBarMessage(const std::string &msg)
{
    ui->statusBar->showMessage(QString::fromStdString(msg));
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

void vtkWindowCube::setThreshold(double threshold)
{
    isosurface->SetValue(0, threshold);
    ui->qVtkCube->renderWindow()->GetInteractor()->Render();
}

void vtkWindowCube::showContours()
{
    auto plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(0, 0, sliceViewer->GetSlice());
    plane->SetNormal(0, 0, 1);

    auto cutter = vtkSmartPointer<vtkCutter>::New();
    cutter->SetCutFunction(plane);
    cutter->SetInputData(fitsReader->GetOutput());
    cutter->Update();

    int level = ui->levelText->text().toInt();
    double min = ui->lowerBoundText->text().toDouble();
    double max = ui->upperBoundText->text().toDouble();

    auto contoursFilter = vtkSmartPointer<vtkContourFilter>::New();
    contoursFilter->GenerateValues(level, min, max);
    contoursFilter->SetInputConnection(cutter->GetOutputPort());

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(contoursFilter->GetOutputPort());
    mapper->SetScalarRange(min, max);
    mapper->ScalarVisibilityOn();
    mapper->SetScalarModeToUsePointData();
    mapper->SetColorModeToMapScalars();

    contoursActor->SetMapper(mapper);
    ui->qVtkSlice->renderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(contoursActor);
    ui->qVtkSlice->renderWindow()->GetInteractor()->Render();
}

void vtkWindowCube::removeContours()
{
    ui->qVtkSlice->renderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(contoursActor);
    ui->qVtkSlice->renderWindow()->GetInteractor()->Render();
}

void vtkWindowCube::resetCamera()
{
    auto renderer = ui->qVtkCube->renderWindow()->GetRenderers()->GetFirstRenderer();
    auto camera = renderer->GetActiveCamera();
    camera->SetViewUp(0, 1, 0);
    camera->SetPosition(initialCameraPosition);
    camera->SetFocalPoint(initialCameraFocalPoint);
    renderer->ResetCamera();
}

void vtkWindowCube::setCameraAzimuth(double az)
{
    resetCamera();
    auto renderer = ui->qVtkCube->renderWindow()->GetRenderers()->GetFirstRenderer();
    renderer->GetActiveCamera()->Azimuth(az);
    ui->qVtkCube->renderWindow()->GetInteractor()->Render();
}

void vtkWindowCube::setCameraElevation(double el)
{
    resetCamera();
    auto renderer = ui->qVtkCube->renderWindow()->GetRenderers()->GetFirstRenderer();
    renderer->GetActiveCamera()->Elevation(el);
    ui->qVtkCube->renderWindow()->GetInteractor()->Render();
}

void vtkWindowCube::on_actionFront_triggered()
{
    setCameraAzimuth(0);
}

void vtkWindowCube::on_actionBack_triggered()
{
    setCameraAzimuth(-180);
}

void vtkWindowCube::on_actionTop_triggered()
{
    setCameraElevation(90);
}

void vtkWindowCube::on_actionRight_triggered()
{
    setCameraAzimuth(90);
}

void vtkWindowCube::on_actionBottom_triggered()
{
    setCameraElevation(-90);
}

void vtkWindowCube::on_actionLeft_triggered()
{
    setCameraAzimuth(-90);
}

void vtkWindowCube::on_thresholdText_editingFinished()
{
    double threshold = ui->thresholdText->text().toDouble();
    // Clamp threshold
    threshold = fmin(fmax(threshold, lowerBound), upperBound);
    ui->thresholdText->setText(QString::number(threshold, 'f', 4));

    int tickPosition = 100 * (threshold - lowerBound) / (upperBound - lowerBound);
    ui->thresholdSlider->setValue(tickPosition);
    setThreshold(threshold);
}

void vtkWindowCube::on_thresholdSlider_sliderReleased()
{
    double threshold =
            (ui->thresholdSlider->value() * (upperBound - lowerBound) / 100) + lowerBound;
    ui->thresholdText->setText(QString::number(threshold, 'f', 4));
    setThreshold(threshold);
}

void vtkWindowCube::on_contourCheckBox_toggled(bool checked)
{
    if (checked) {
        showContours();
    } else {
        removeContours();
    }
}

void vtkWindowCube::on_levelText_editingFinished()
{
    if (ui->contourCheckBox->isChecked()) {
        showContours();
    }
}

void vtkWindowCube::on_lowerBoundText_editingFinished()
{
    if (ui->contourCheckBox->isChecked()) {
        showContours();
    }
}

void vtkWindowCube::on_upperBoundText_editingFinished()
{
    if (ui->contourCheckBox->isChecked()) {
        showContours();
    }
}
