#include "vtkwindowsources.h"
#include "ui_vtkwindowsources.h"

#include "vispoint.h"

#include <vtkAxesActor.h>
#include <vtkCaptionActor2D.h>
#include <vtkCellArray.h>
#include <vtkCubeAxesActor2D.h>
#include <vtkFloatArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkLineSource.h>
#include <vtkPVLODActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkOutlineCornerFilter.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolygon.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSphereSource.h>
#include <vtkTextActor.h>

vtkWindowSources::vtkWindowSources(QWidget *parent, VisPoint *visPoint)
    : QMainWindow(parent),
      ui(new Ui::vtkWindowSources),
      visPoint(visPoint),
      xThresholdMin(-15000),
      xThresholdMax(15000),
      yThresholdMin(-10000),
      yThresholdMax(25000),
      zThresholdMin(-200),
      zThresholdMax(200),
      zScaleFactor(40)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(visPoint->getName());

    for (uint i = 0; i < visPoint->getOrigin()->getNumberOfColumns(); ++i) {
        QString field = QString::fromStdString(visPoint->getOrigin()->getColName(i));
        ui->lutScalarComboBox->addItem(field);
        ui->glyphScalarComboBox->addItem(field);
    }

    auto renWin = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->qVtk->setDefaultCursor(Qt::ArrowCursor);
    ui->qVtk->setRenderWindow(renWin);

    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->GlobalWarningDisplayOff();
    renWin->AddRenderer(renderer);

    showSources();
    showAxesLegend();
    showXYAxes();
    showBoundingBox();
    showOutlineTextLegend();

    renderer->ResetCamera();
    renWin->GetInteractor()->Render();
}

vtkWindowSources::~vtkWindowSources()
{
    delete ui;
}

void vtkWindowSources::render()
{
    ui->qVtk->renderWindow()->GetInteractor()->Render();
}

vtkRenderer *vtkWindowSources::renderer()
{
    return ui->qVtk->renderWindow()->GetRenderers()->GetFirstRenderer();
}

void vtkWindowSources::showSources()
{
    auto vstable = visPoint->getOrigin();
    auto nRows = vstable->getNumberOfRows();

    auto xAxis = vtkSmartPointer<vtkFloatArray>::New();
    xAxis->SetNumberOfTuples(nRows);
    xAxis->SetName(visPoint->getX().toUtf8().constData());
    int xIndex = vstable->getColId(visPoint->getX().toStdString());

    auto yAxis = vtkSmartPointer<vtkFloatArray>::New();
    yAxis->SetNumberOfTuples(nRows);
    yAxis->SetName(visPoint->getY().toUtf8().constData());
    int yIndex = vstable->getColId(visPoint->getY().toStdString());

    auto zAxis = vtkSmartPointer<vtkFloatArray>::New();
    zAxis->SetNumberOfTuples(nRows);
    zAxis->SetName(visPoint->getZ().toUtf8().constData());
    int zIndex = vstable->getColId(visPoint->getZ().toStdString());

    for (ulong i = 0; i < nRows; ++i) {
        double xval = atof(vstable->getTableData()[xIndex][i].c_str());
        double yval = atof(vstable->getTableData()[yIndex][i].c_str());
        double zval = atof(vstable->getTableData()[zIndex][i].c_str());

        if ((xval > xThresholdMin && xval < xThresholdMax)
            && (yval > yThresholdMin && yval < yThresholdMax)
            && (zval > zThresholdMin && zval < zThresholdMax)) {
            xAxis->SetValue(i, xval);
            yAxis->SetValue(i, yval);
            zAxis->SetValue(i, zval);
        }
    }

    // xAxis->GetRange(xRange);
    // yAxis->GetRange(yRange);
    // zAxis->GetRange(zRange);

    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(nRows);
    for (ulong i = 0; i < nRows; ++i) {
        points->SetPoint(i, xAxis->GetValue(i), yAxis->GetValue(i),
                         zAxis->GetValue(i) * zScaleFactor);
    }

    auto verts = vtkSmartPointer<vtkCellArray>::New();
    verts->AllocateEstimate(nRows, 1);
    for (ulong i = 0; i < nRows; ++i) {
        verts->InsertNextCell(1);
        verts->InsertCellPoint(i);
    }

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetVerts(verts);

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    auto actor = vtkSmartPointer<vtkPVLODActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetOpacity(0.65);

    renderer()->AddActor(actor);
}

void vtkWindowSources::showBoundingBox()
{
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->InsertNextPoint(xThresholdMin, yThresholdMin, zThresholdMin * zScaleFactor);
    points->InsertNextPoint(xThresholdMax, yThresholdMin, zThresholdMin * zScaleFactor);
    points->InsertNextPoint(xThresholdMin, yThresholdMax, zThresholdMin * zScaleFactor);
    points->InsertNextPoint(xThresholdMax, yThresholdMax, zThresholdMin * zScaleFactor);
    points->InsertNextPoint(xThresholdMin, yThresholdMin, zThresholdMax * zScaleFactor);
    points->InsertNextPoint(xThresholdMax, yThresholdMin, zThresholdMax * zScaleFactor);
    points->InsertNextPoint(xThresholdMin, yThresholdMax, zThresholdMax * zScaleFactor);
    points->InsertNextPoint(xThresholdMax, yThresholdMax, zThresholdMax * zScaleFactor);

    auto polygon = vtkSmartPointer<vtkPolygon>::New();
    polygon->GetPointIds()->SetNumberOfIds(8);
    polygon->GetPointIds()->SetId(0, 0);
    polygon->GetPointIds()->SetId(1, 1);
    polygon->GetPointIds()->SetId(2, 2);
    polygon->GetPointIds()->SetId(3, 3);
    polygon->GetPointIds()->SetId(4, 4);
    polygon->GetPointIds()->SetId(5, 5);
    polygon->GetPointIds()->SetId(6, 6);
    polygon->GetPointIds()->SetId(7, 7);

    auto polygons = vtkSmartPointer<vtkCellArray>::New();
    polygons->InsertNextCell(polygon);

    auto polygonPolyData = vtkSmartPointer<vtkPolyData>::New();
    polygonPolyData->SetPoints(points);
    polygonPolyData->SetPolys(polygons);

    auto corner = vtkSmartPointer<vtkOutlineCornerFilter>::New();
    corner->SetInputData(polygonPolyData);
    corner->ReleaseDataFlagOn();
    corner->Update();

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(corner->GetOutputPort());

    auto property = vtkSmartPointer<vtkProperty>::New();
    property->SetColor(1, 1, 1);
    property->SetOpacity(0.99);
    property->SetAmbient(1);
    property->SetRepresentationToWireframe();
    property->SetInterpolationToFlat();

    auto actor = vtkSmartPointer<vtkPVLODActor>::New();
    actor->SetMapper(mapper);
    actor->SetProperty(property);
    actor->SetPickable(false);
    actor->SetVisibility(true);

    renderer()->AddActor(actor);
}

void vtkWindowSources::showXYAxes()
{
    auto l1 = vtkSmartPointer<vtkLineSource>::New();
    l1->SetPoint1(0.0, yThresholdMin, 0.0);
    l1->SetPoint2(0.0, yThresholdMax, 0.0);
    l1->Update();
    auto m1 = vtkSmartPointer<vtkPolyDataMapper>::New();
    m1->SetInputConnection(l1->GetOutputPort());
    auto a1 = vtkSmartPointer<vtkActor>::New();
    a1->SetMapper(m1);
    a1->GetProperty()->SetLineWidth(1);

    auto l2 = vtkSmartPointer<vtkLineSource>::New();
    l2->SetPoint1(xThresholdMin, 0.0, 0.0);
    l2->SetPoint2(xThresholdMax, 0.0, 0.0);
    l2->Update();
    auto m2 = vtkSmartPointer<vtkPolyDataMapper>::New();
    m2->SetInputConnection(l2->GetOutputPort());
    auto a2 = vtkSmartPointer<vtkActor>::New();
    a2->SetMapper(m2);
    a2->GetProperty()->SetLineWidth(1);

    auto sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetCenter(0, 8400, 0);
    sphereSource->SetRadius(100);
    sphereSource->Update();
    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(sphereSource->GetOutputPort());
    auto sphere = vtkSmartPointer<vtkActor>::New();
    sphere->SetMapper(mapper);
    sphere->GetProperty()->SetColor(1, 0, 0);

    renderer()->AddActor(a1);
    renderer()->AddActor(a2);
    renderer()->AddActor(sphere);
}

void vtkWindowSources::showAxesLegend()
{
    auto axesActor = vtkSmartPointer<vtkAxesActor>::New();
    axesActor->SetXAxisLabelText("X");
    axesActor->SetYAxisLabelText("Y");
    axesActor->SetZAxisLabelText("Z");
    axesActor->DragableOn();
    axesActor->GetXAxisCaptionActor2D()->GetTextActor()->SetTextScaleMode(0);
    axesActor->GetYAxisCaptionActor2D()->GetTextActor()->SetTextScaleMode(0);
    axesActor->GetZAxisCaptionActor2D()->GetTextActor()->SetTextScaleMode(0);
    axesWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    axesWidget->SetInteractor(ui->qVtk->renderWindow()->GetInteractor());
    axesWidget->SetOrientationMarker(axesActor);
    axesWidget->SetOutlineColor(0.9300, 0.5700, 0.1300);
    axesWidget->SetViewport(0.0, 0.0, 0.2, 0.2);
    axesWidget->SetEnabled(1);
    axesWidget->InteractiveOff();
}

void vtkWindowSources::showOutlineTextLegend()
{
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->InsertNextPoint(xThresholdMin, yThresholdMin, zThresholdMin * zScaleFactor);
    points->InsertNextPoint(xThresholdMax, yThresholdMin, zThresholdMin * zScaleFactor);
    points->InsertNextPoint(xThresholdMin, yThresholdMax, zThresholdMin * zScaleFactor);
    points->InsertNextPoint(xThresholdMax, yThresholdMax, zThresholdMin * zScaleFactor);
    points->InsertNextPoint(xThresholdMin, yThresholdMin, zThresholdMax * zScaleFactor);
    points->InsertNextPoint(xThresholdMax, yThresholdMin, zThresholdMax * zScaleFactor);
    points->InsertNextPoint(xThresholdMin, yThresholdMax, zThresholdMax * zScaleFactor);
    points->InsertNextPoint(xThresholdMax, yThresholdMax, zThresholdMax * zScaleFactor);

    auto polygon = vtkSmartPointer<vtkPolygon>::New();
    polygon->GetPointIds()->SetNumberOfIds(8);
    polygon->GetPointIds()->SetId(0, 0);
    polygon->GetPointIds()->SetId(1, 1);
    polygon->GetPointIds()->SetId(2, 2);
    polygon->GetPointIds()->SetId(3, 3);
    polygon->GetPointIds()->SetId(4, 4);
    polygon->GetPointIds()->SetId(5, 5);
    polygon->GetPointIds()->SetId(6, 6);
    polygon->GetPointIds()->SetId(7, 7);

    auto polygons = vtkSmartPointer<vtkCellArray>::New();
    polygons->InsertNextCell(polygon);

    auto polygonPolyData = vtkSmartPointer<vtkPolyData>::New();
    polygonPolyData->SetPoints(points);
    polygonPolyData->SetPolys(polygons);

    auto axesActor = vtkSmartPointer<vtkCubeAxesActor2D>::New();
    axesActor->SetInputData(polygonPolyData);
    axesActor->UseRangesOn();
    axesActor->SetBounds(xThresholdMin, xThresholdMax, yThresholdMin, yThresholdMax,
                         zThresholdMin * zScaleFactor, zThresholdMax * zScaleFactor);
    axesActor->SetRanges(xThresholdMin, xThresholdMax, yThresholdMin, yThresholdMax, zThresholdMin,
                         zThresholdMax);
    axesActor->SetNumberOfLabels(6);
    axesActor->SetViewProp(NULL);
    axesActor->SetScaling(0);
    axesActor->SetPickable(0);
    axesActor->SetCamera(renderer()->GetActiveCamera());
    axesActor->SetCornerOffset(0.1);
    axesActor->SetLabelFormat("%3.3g");
    axesActor->SetInertia(100);
    axesActor->SetFlyModeToOuterEdges();
    axesActor->SetVisibility(true);
    axesActor->SetXLabel(visPoint->getX().toUtf8().constData());
    axesActor->SetYLabel(visPoint->getY().toUtf8().constData());
    axesActor->SetZLabel(visPoint->getZ().toUtf8().constData());
    axesActor->Modified();

    renderer()->AddActor2D(axesActor);
}
