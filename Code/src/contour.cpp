#include "contour.h"
#include "ui_contour.h"

#include "luteditor.h"
#include "ui_vtkwindow_new.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkContourFilter.h"
#include "vtkCutter.h"
#include "vtkDoubleArray.h"
#include "vtkImageData.h"
#include "vtkImageReader.h"
#include "vtkImageReader2.h"
#include "vtkImageShiftScale.h"
#include "vtkImageToStructuredPoints.h"
#include "vtkIntArray.h"
#include "vtkLabeledDataMapper.h"
#include "vtkMapper2D.h"
#include "vtkMarchingCubes.h"
#include "vtkMarchingSquares.h"
#include "vtkMath.h"
#include "vtkPlane.h"
#include "vtkPlaneSource.h"
#include "vtkPointData.h"
#include "vtkProperty.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"
#include "vtkSmartPointer.h"
#include "vtkStripper.h"
#include "vtkStructuredPoints.h"
#include "vtkwindow_new.h"
#include <vtkPlanes.h>
#include <vtkPolyDataMapper.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h>
#include <vtkTextProperty.h>

#include <QDebug>
#include <QMenu>
#include <QSplitter>
#include <QStandardItemModel>
#include <QTableView>
#include <QTreeWidget>

static const int ItemTypeRoot = QTreeWidgetItem::UserType;
static const int ItemTypeChild = QTreeWidgetItem::UserType + 1;

contour::contour(QWidget *parent) : QWidget(parent), ui(new Ui::contour)
{
    ui->setupUi(this);
    isolines = vtkSmartPointer<vtkActor>::New();

    QStringList ColumnNames;
    ColumnNames << "Id"
                << "Value";

    connect(ui->labelCheckBox, SIGNAL(clicked(bool)), this, SLOT(toggledLabel(bool)));
    connect(ui->idCheckBox, SIGNAL(clicked(bool)), this, SLOT(toggledId(bool)));
}

contour::~contour()
{
    delete ui;
}

void contour::toggledLabel(bool t)
{
    if (t) {

        ui->idCheckBox->setChecked(false);
        if (ui->activateCheckBox->isChecked()) {
            vtkWin->removeActor(idlabel);
            vtkWin->addActor(isolabels);
        }
    } else {
        vtkWin->removeActor(isolabels);
    }
}

void contour::toggledId(bool t)
{
    ui->labelCheckBox->setChecked(false);

    if (t) {
        if (ui->activateCheckBox->isChecked()) {
            vtkWin->removeActor(isolabels);
            vtkWin->addActor(idlabel);
        }
    } else {
        vtkWin->removeActor(idlabel);
    }
}

void contour::setFitsReader(vtkFitsReader *fits, vtkwindow_new *win)
{

    fitsReader = fits;
    vtkWin = win;

    // SET DEFAULT PARAMS
    ui->numOfContourText->setText("5");

    ui->minValueText->setText(QString::number(fitsReader->GetRMS() * 3));
    ui->maxValueText->setText(QString::number(fitsReader->GetMax()));

    ui->minFitsText->setText(QString::number(fitsReader->GetMin()));
    ui->maxFitsText->setText(QString::number(fitsReader->GetMax()));

    ui->slicesText->setText(QString::number(fitsReader->GetNaxes(2)));
    ui->RMSText->setText(QString::number(fitsReader->GetRMS()));
    // ui->mediaText->setText(QString::number(fitsReader->GetMedia()));

    ui->minFitsText->setEnabled(false);
    ui->maxFitsText->setEnabled(false);
    ui->slicesText->setEnabled(false);
    ui->RMSText->setEnabled(false);
    ui->LogLinearCheckBox->setChecked(true);

    ui->redText->hide();
    ui->greenText->hide();
    ui->blueText->hide();
    ui->label_2->hide();
    ui->label_3->hide();
    ui->label->hide();

    if (vtkWin->getContourVisualized()) {
        // qDebug()<<"true";
        ui->activateCheckBox->setChecked(true);
        ui->LogLinearCheckBox->setChecked(true);
    } else {
        ui->activateCheckBox->setChecked(false);
    }
}

void contour::createContour()
{
    int pointThreshold = 10;
    vtkSmartPointer<vtkContourFilter> mycontours = vtkSmartPointer<vtkContourFilter>::New();

    double range[2];
    fitsReader->GetOutput()->GetScalarRange(range);
    mycontours->SetValue(0, (range[1] + range[0]) / 2.0);
    mycontours->SetInputConnection(fitsReader->GetOutputPort());

    vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();
    int *size = vtkWin->renwin->GetSize();
    plane->SetXResolution(size[0]);
    plane->SetYResolution(size[1]);
    plane->Update();

    vtkSmartPointer<vtkDoubleArray> randomScalars = vtkSmartPointer<vtkDoubleArray>::New();
    randomScalars->SetNumberOfComponents(1);
    randomScalars->SetName("Isovalues");
    for (int i = 0; i < plane->GetOutput()->GetNumberOfPoints(); i++) {
        randomScalars->InsertNextTuple1(vtkMath::Random(-100.0, 100.0));
    }

    plane->GetOutput()->GetPointData()->SetScalars(randomScalars);
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();

    polyData = plane->GetOutput();
    mycontours->SetInputConnection(plane->GetOutputPort());
    mycontours->GenerateValues(5, -100, 100);
    pointThreshold = 0;

    // Connect the segments of the contours into polylines
    vtkSmartPointer<vtkStripper> contourStripper = vtkSmartPointer<vtkStripper>::New();
    contourStripper->SetInputConnection(mycontours->GetOutputPort());
    contourStripper->Update();

    int numberOfContourLines = contourStripper->GetOutput()->GetNumberOfLines();

    vtkPoints *points = contourStripper->GetOutput()->GetPoints();
    vtkCellArray *cells = contourStripper->GetOutput()->GetLines();
    vtkDataArray *scalars = contourStripper->GetOutput()->GetPointData()->GetScalars();

    // Create a polydata that contains point locations for the contour
    // line labels
    vtkSmartPointer<vtkPolyData> labelPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> labelPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkDoubleArray> labelScalars = vtkSmartPointer<vtkDoubleArray>::New();
    labelScalars->SetNumberOfComponents(1);
    labelScalars->SetName("Isovalues");

    const vtkIdType *indices;
    vtkIdType numberOfPoints;
    unsigned int lineCount = 0;
    for (cells->InitTraversal(); cells->GetNextCell(numberOfPoints, indices); lineCount++) {
        if (numberOfPoints < pointThreshold) {
            continue;
        }

        // Compute the point id to hold the label
        // Mid point or a random point
        vtkIdType midPointId = indices[numberOfPoints / 2];
        midPointId = indices[static_cast<vtkIdType>(vtkMath::Random(0, numberOfPoints))];

        double midPoint[3];
        points->GetPoint(midPointId, midPoint);

        labelPoints->InsertNextPoint(midPoint);
        labelScalars->InsertNextTuple1(scalars->GetTuple1(midPointId));
    }
    labelPolyData->SetPoints(labelPoints);
    labelPolyData->GetPointData()->SetScalars(labelScalars);

    vtkSmartPointer<vtkPolyDataMapper> contourMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    contourMapper->SetInputConnection(contourStripper->GetOutputPort());

    vtkSmartPointer<vtkActor> isolines = vtkSmartPointer<vtkActor>::New();
    isolines->SetMapper(contourMapper);

    vtkSmartPointer<vtkLookupTable> surfaceLUT = vtkSmartPointer<vtkLookupTable>::New();
    surfaceLUT->SetRange(polyData->GetPointData()->GetScalars()->GetRange());
    surfaceLUT->Build();

    vtkSmartPointer<vtkPolyDataMapper> surfaceMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

    surfaceMapper->SetInputData(polyData);

    surfaceMapper->ScalarVisibilityOn();
    surfaceMapper->SetScalarRange(polyData->GetPointData()->GetScalars()->GetRange());
    surfaceMapper->SetLookupTable(surfaceLUT);

    vtkSmartPointer<vtkActor> surface = vtkSmartPointer<vtkActor>::New();
    surface->SetMapper(surfaceMapper);

    // The labeled data mapper will place labels at the points
    vtkSmartPointer<vtkLabeledDataMapper> labelMapper =
            vtkSmartPointer<vtkLabeledDataMapper>::New();
    labelMapper->SetFieldDataName("Isovalues");

    labelMapper->SetInputData(labelPolyData);

    labelMapper->SetLabelModeToLabelScalars();
    labelMapper->SetLabelFormat("%6.2f");

    vtkSmartPointer<vtkActor2D> isolabels = vtkSmartPointer<vtkActor2D>::New();
    isolabels->SetMapper(labelMapper);

    // Add the actors to the scene
    vtkWin->ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(isolines);
    vtkWin->ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(isolabels);
    vtkWin->ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(contour_actor);
    vtkWin->ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(isolabels);

    vtkWin->ui->qVTK1->renderWindow()->Render();
    vtkWin->ui->qVTK1->update();
}

void contour::deleteContour()
{
    contourStripper->GetOutput()->RemoveDeletedCells();
    vtkWin->ui->qVTK1->renderWindow()->Render();
}

void contour::on_okButton_clicked()
{
    if (ui->activateCheckBox->isChecked()) {
        vtkWin->removeActor(isolabels);
        vtkWin->removeActor(isolines);
        vtkWin->removeActor(idlabel);
        vtkWin->removeActor(scalarBar);
        ui->idCheckBox->setChecked(false);

        addContours();

        ui->labelCheckBox->setEnabled(true);
        ui->activateCheckBox->setChecked(true);
        vtkWin->setContourVisualized(true);
    } else {
        vtkWin->setContourVisualized(false);
        ui->labelCheckBox->setEnabled(false);
        ui->idCheckBox->setEnabled(false);
    }
}

void contour::addContours()
{
    // start cutter:
    vtkSmartPointer<vtkCutter> cutter = vtkSmartPointer<vtkCutter>::New();
    // Create a plane to cut,here it cuts in the XZ direction (xz normal=(1,0,0);XY =(0,0,1),YZ
    // =(0,1,0)

    if (vtkWin->ui->spinBox_channels->text().toInt() == 0) {
        vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();

        plane->SetOrigin(0, 0, vtkWin->viewer->GetSlice());
        //  plane->SetOrigin(0,0,vtkWin->imageViewer->GetSlice());
        plane->SetNormal(0, 0, 1);
        // Set cutter function for a single planes
        cutter->SetCutFunction(plane);
    } else {

        vtkSmartPointer<vtkPlanes> planes = vtkSmartPointer<vtkPlanes>::New();
        planes->SetBounds(
                0, fitsReader->GetNaxes(0), 0, fitsReader->GetNaxes(1),
                vtkWin->imageViewer->GetSlice() - vtkWin->ui->spinBox_channels->text().toInt(),
                vtkWin->imageViewer->GetSlice() + vtkWin->ui->spinBox_channels->text().toInt());

        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        points = planes->GetPoints();

        int i;
        double *point;
        for (i = 0; i < points->GetNumberOfPoints(); i++) {
            point = points->GetPoint(i);
        }

        // Set cutter function for planes
        cutter->SetCutFunction(planes);
    }

    cutter->SetInputData(fitsReader->GetOutput());
    cutter->Update();

    vtkSmartPointer<vtkPolyDataMapper> cutterMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    cutterMapper->SetInputConnection(cutter->GetOutputPort());

    // Create plane actor
    vtkSmartPointer<vtkActor> planeActor = vtkSmartPointer<vtkActor>::New();
    planeActor->GetProperty()->SetColor(1.0, 1, 0);
    planeActor->GetProperty()->SetLineWidth(2);
    planeActor->SetMapper(cutterMapper);

    // Create renderers and add actors of plane and cube
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->AddActor(planeActor); // display the rectangle resulting from the cut

    // Add renderer to renderwindow and render
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    renderWindow->SetSize(600, 600);

    vtkSmartPointer<vtkRenderWindowInteractor> interactor =
            vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(renderWindow);
    renderer->SetBackground(0, 0, 0);
    // end cutter

    int pointThreshold = 10;

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkContourFilter> contoursFilter = vtkSmartPointer<vtkContourFilter>::New();

    polyData = cutter->GetOutput();
    contoursFilter->GenerateValues(ui->numOfContourText->text().toInt(),
                                   ui->minValueText->text().toDouble(),
                                   ui->maxValueText->text().toDouble());
    contoursFilter->SetInputConnection(cutter->GetOutputPort());

    // Connect the segments of the conours into polylines
    vtkSmartPointer<vtkStripper> contourStripper = vtkSmartPointer<vtkStripper>::New();
    contourStripper->SetInputConnection(contoursFilter->GetOutputPort());
    contourStripper->Update();

    vtkPoints *points = contourStripper->GetOutput()->GetPoints();
    vtkCellArray *cells = contourStripper->GetOutput()->GetLines();
    vtkDataArray *scalars = contourStripper->GetOutput()->GetPointData()->GetScalars();

    // Create a polydata that contains point locations for the contour
    // line labels
    vtkSmartPointer<vtkPolyData> labelPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> labelPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkDoubleArray> labelScalars = vtkSmartPointer<vtkDoubleArray>::New();
    labelScalars->SetNumberOfComponents(1);
    labelScalars->SetName("Isovalues");

    const vtkIdType *indices;
    vtkIdType numberOfPoints;
    unsigned int lineCount = 0;
    for (cells->InitTraversal(); cells->GetNextCell(numberOfPoints, indices); lineCount++) {
        if (numberOfPoints < pointThreshold) {
            continue;
        }
        // Compute the point id to hold the label
        // Mid point or a random point
        vtkIdType midPointId = indices[numberOfPoints / 2];
        midPointId = indices[static_cast<vtkIdType>(vtkMath::Random(0, numberOfPoints))];

        double midPoint[3];
        points->GetPoint(midPointId, midPoint);

        labelPoints->InsertNextPoint(midPoint);
        labelScalars->InsertNextTuple1(scalars->GetTuple1(midPointId));
    }
    labelPolyData->SetPoints(labelPoints);
    labelPolyData->GetPointData()->SetScalars(labelScalars);

    vtkSmartPointer<vtkPolyDataMapper> contourMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    contourMapper->SetInputConnection(contourStripper->GetOutputPort());

    /*START LUT*/
    contourMapper->ScalarVisibilityOn();
    contourMapper->SetScalarModeToUsePointData();
    contourMapper->SetColorModeToMapScalars();
    contourMapper->SetScalarRange(fitsReader->GetMin(), fitsReader->GetMax());

    scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
    scalarBar->DragableOn();
    scalarBar->SetNumberOfLabels(5);

    // Create one text property for all
    vtkSmartPointer<vtkTextProperty> textProperty = vtkSmartPointer<vtkTextProperty>::New();
    textProperty->SetFontSize(0.1);
    textProperty->SetColor(0.4, 0.4, 1);
    textProperty->SetFontFamilyToArial();
    scalarBar->SetLabelTextProperty(textProperty);
    scalarBar->SetWidth(0.05);
    scalarBar->SetHeight(0.9);

    vtkSmartPointer<vtkTextProperty> titleTextProperty = vtkSmartPointer<vtkTextProperty>::New();
    titleTextProperty->SetFontSize(24);
    titleTextProperty->SetFontFamilyToArial();
    titleTextProperty->SetBold(true);
    titleTextProperty->SetColor(1., 0., 0.);

    scalarBar->SetTitleTextProperty(titleTextProperty);

    // Create a lookup table to share between the mapper and the scalarbar
    vtkSmartPointer<vtkLookupTable> hueLut = vtkSmartPointer<vtkLookupTable>::New();
    hueLut->SetHueRange(0, 1);
    hueLut->SetSaturationRange(1, 1);
    hueLut->SetValueRange(1, 1);
    hueLut->Build();

    contourMapper->SetLookupTable(hueLut);
    scalarBar->SetLookupTable(hueLut);
    isolines->SetMapper(contourMapper);

    vtkSmartPointer<vtkLookupTable> surfaceLUT = vtkSmartPointer<vtkLookupTable>::New();
    surfaceLUT->SetRange(polyData->GetPointData()->GetScalars()->GetRange());
    surfaceLUT->Build();

    vtkSmartPointer<vtkPolyDataMapper> surfaceMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    surfaceMapper->SetInputData(polyData);
    surfaceMapper->ScalarVisibilityOn();
    surfaceMapper->SetScalarRange(polyData->GetPointData()->GetScalars()->GetRange());
    surfaceMapper->SetLookupTable(surfaceLUT);

    vtkSmartPointer<vtkActor> surface = vtkSmartPointer<vtkActor>::New();
    surface->SetMapper(surfaceMapper);

    // The labeled data mapper will place labels at the points
    vtkSmartPointer<vtkLabeledDataMapper> labelMapper =
            vtkSmartPointer<vtkLabeledDataMapper>::New();
    labelMapper->SetFieldDataName("Isovalues");

    labelMapper->SetInputData(labelPolyData);

    labelMapper->SetLabelModeToLabelScalars();
    labelMapper->SetLabelFormat("%6.2f");

    isolabels = vtkSmartPointer<vtkActor2D>::New();
    isolabels->SetMapper(labelMapper);

    // Create an interactor
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
            vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);

    vtkWin->m_Ren2->AddActor(isolines);
    if (ui->labelCheckBox->isChecked() == true)
        vtkWin->m_Ren2->AddActor(isolabels);
    vtkWin->m_Ren2->Render();
    vtkWin->ui->isocontourVtkWin->update();
}

void contour::addContours2()
{

    // Create a plane to cut,here it cuts in the XZ direction (xz normal=(1,0,0);XY =(0,0,1),YZ =(0,1,0)
    vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(0, 0, vtkWin->imageViewer->GetSlice());
    plane->SetNormal(0, 0, 1);

    // Create cutter
    vtkSmartPointer<vtkCutter> cutter = vtkSmartPointer<vtkCutter>::New();
    cutter->SetCutFunction(plane);
    cutter->SetInputConnection(fitsReader->GetOutputPort());
    cutter->GenerateValues(5, fitsReader->GetMin(), fitsReader->GetMax());
    cutter->Update();

    vtkSmartPointer<vtkPolyDataMapper> cutterMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    cutterMapper->SetInputConnection(cutter->GetOutputPort());

    // Create plane actor
    vtkSmartPointer<vtkActor> planeActor = vtkSmartPointer<vtkActor>::New();
    planeActor->GetProperty()->SetColor(1.0, 1, 0);
    planeActor->GetProperty()->SetLineWidth(2);
    planeActor->SetMapper(cutterMapper);

    // Create renderers and add actors of plane and cube
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->AddActor(planeActor); // display the rectangle resulting from the cut

    // Add renderer to renderwindow and render
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    renderWindow->SetSize(600, 600);

    vtkSmartPointer<vtkRenderWindowInteractor> interactor =
            vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(renderWindow);
    renderer->SetBackground(0, 0, 0);

    renderWindow->Render();
    interactor->Start();
}

void contour::createwin() { }
