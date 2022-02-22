#include "vtkwindowcube.h"
#include "ui_vtkwindowcube.h"

#include "vtkfitsreader.h"
#include "vtklegendscaleactor.h"

#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCaptionActor2D.h>
#include <vtkFlyingEdges3D.h>
#include <vtkFrustumSource.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkOutlineFilter.h>
#include <vtkPlanes.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

vtkWindowCube::vtkWindowCube(QWidget *parent, vtkSmartPointer<vtkFitsReader> fitsReader)
    : QMainWindow(parent), ui(new Ui::vtkWindowCube), fitsReader(fitsReader)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(QString::fromStdString(fitsReader->GetFileName()));

    fitsReader->is3D = true;
    fitsReader->CalculateRMS();

    auto renWin = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->qVtkCube->setDefaultCursor(Qt::ArrowCursor);
    ui->qVtkCube->setRenderWindow(renWin);

    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->SetBackground(0.21, 0.23, 0.25);
    renderer->GlobalWarningDisplayOff();
    renWin->AddRenderer(renderer);

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
    auto planeActor = vtkSmartPointer<vtkActor>::New();
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
    axesWidget->SetInteractor(renWin->GetInteractor());
    axesWidget->SetOrientationMarker(axesActor);
    axesWidget->SetOutlineColor(0.9300, 0.5700, 0.1300);
    axesWidget->SetViewport(0.0, 0.0, 0.2, 0.2);
    axesWidget->SetEnabled(1);
    axesWidget->InteractiveOff();

    // Legend
    auto legendActor = vtkSmartPointer<vtkLegendScaleActor>::New();
    legendActor->LegendVisibilityOff();
    legendActor->setFitsFile(fitsReader);

    renderer->AddActor(outlineActor);
    renderer->AddActor(isosurfaceActor);
    renderer->AddActor(planeActor);
    renderer->AddActor(legendActor);

    renderer->ResetCamera();
    renWin->GetInteractor()->Render();
}

vtkWindowCube::~vtkWindowCube()
{
    delete ui;
}
