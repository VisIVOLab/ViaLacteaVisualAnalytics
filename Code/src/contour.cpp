#include "contour.h"
#include "ui_contour.h"
#include <vtkPolyDataMapper.h>
#include "vtkMath.h"
#include "vtkDoubleArray.h"
#include "vtkContourFilter.h"
#include "vtkPlaneSource.h"
#include "vtkMarchingCubes.h"
#include "vtkImageShiftScale.h"
#include "vtkImageData.h"
#include "vtkImageReader2.h"
#include "vtkPlane.h"
#include "vtkImageReader.h"
#include "vtkImageToStructuredPoints.h"
#include "vtkMarchingSquares.h"
#include "vtkProperty.h"
#include "vtkStructuredPoints.h"
#include "vtkLabeledDataMapper.h"
#include "vtkStripper.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkRenderWindow.h"
#include "vtkRendererCollection.h"
#include "vtkCamera.h"
#include "vtkMapper2D.h"
#include "vtkSmartPointer.h"
#include "vtkIntArray.h"
#include "vtkCutter.h"
#include "luteditor.h"
#include <vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h>
#include <vtkTextProperty.h>
#include <vtkPlanes.h>


#include <QTableView>
#include <QSplitter>
#include <QStandardItemModel>
#include <QMenu>
#include <QDebug>
#include <QTreeWidget>

static const int ItemTypeRoot = QTreeWidgetItem::UserType;
static const int ItemTypeChild = QTreeWidgetItem::UserType + 1;

/*
contour::contour(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::contour)
{
    ui->setupUi(this);
}*/


contour::contour(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::contour)
{
    ui->setupUi(this);
    isolines =
            vtkSmartPointer<vtkActor>::New();


    // ui->contourTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    //connect(this,SIGNAL(customContextMenuRequested(const QPoint&)),SLOT(onCustomContextMenuRequested(const QPoint&)));


    //   ui->->setContextMenuPolicy(Qt::ActionsContextMenu);

    QStringList ColumnNames;
    ColumnNames << "Id" << "Value";


    connect(ui->labelCheckBox, SIGNAL(clicked(bool)), this, SLOT(toggledLabel(bool)));
    connect(ui->idCheckBox, SIGNAL(clicked(bool)), this, SLOT(toggledId(bool)));


}



contour::~contour()
{
    delete ui;
}


void contour::toggledLabel(bool t)
{
    if(t)
    {

        ui->idCheckBox->setChecked(false);
        if(ui->activateCheckBox->isChecked())
        {
            vtkWin->removeActor(idlabel);
            vtkWin->addActor(isolabels);

        }
    }
    else
    {
        vtkWin->removeActor(isolabels);
    }

}

void contour::toggledId(bool t)
{
    ui->labelCheckBox->setChecked(false);

    if(t)
    {
        if(ui->activateCheckBox->isChecked())
        {
            vtkWin->removeActor(isolabels);
            vtkWin->addActor(idlabel);

        }
    }
    else
    {
        vtkWin->removeActor(idlabel);
    }

}

void contour::setFitsReader(vtkFitsReader *fits,vtkwindow_new *win)
{

    fitsReader=fits;
    vtkWin = win;

    //SET DEFAULT PARAMS
    ui->numOfContourText->setText("5");

    ui->minValueText->setText(QString::number( fitsReader->GetRMS()*3));
    ui->maxValueText->setText(QString::number(fitsReader->GetMax()));

    ui->minFitsText->setText(QString::number( fitsReader->GetMin()));
    ui->maxFitsText->setText(QString::number(fitsReader->GetMax()));


    ui->slicesText->setText(QString::number(fitsReader->GetNaxes(2)));
    ui->RMSText->setText(QString::number(fitsReader->GetRMS()));
    //ui->mediaText->setText(QString::number(fitsReader->GetMedia()));


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


    if(vtkWin->getContourVisualized())
    {
        //qDebug()<<"true";
        ui->activateCheckBox->setChecked(true);
        ui->LogLinearCheckBox->setChecked(true);
    }
    else
    {
        ui->activateCheckBox->setChecked(false);
    }


}

void contour::createContour()
{
    std::cout<<"Inside create Contour"<<std::endl;

    //TEST da qui

    int pointThreshold = 10;
    vtkSmartPointer<vtkContourFilter> mycontours =
            vtkSmartPointer<vtkContourFilter>::New();

    double range[2];
    fitsReader->GetOutput()->GetScalarRange(range);
    mycontours->SetValue(0, (range[1] + range[0]) / 2.0);
    mycontours->SetInputConnection(fitsReader->GetOutputPort());

    vtkSmartPointer<vtkPlaneSource> plane =
            vtkSmartPointer<vtkPlaneSource>::New();
    int *size=vtkWin->renwin->GetSize();
    plane->SetXResolution(size[0]);
    plane->SetYResolution(size[1]);
    plane->Update();

    vtkSmartPointer<vtkDoubleArray> randomScalars =
            vtkSmartPointer<vtkDoubleArray>::New();
    randomScalars->SetNumberOfComponents(1);
    randomScalars->SetName("Isovalues");
    for (int i = 0; i < plane->GetOutput()->GetNumberOfPoints(); i++)
    {
        randomScalars->InsertNextTuple1(vtkMath::Random(-100.0, 100.0));
    }

    //TEST2
    /*
    vtkSmartPointer<vtkResliceImageViewer>viewer  =vtkSmartPointer<vtkResliceImageViewer>::New();
    viewer->SetInput(fitsReader->GetOutput());
    vtkSmartPointer<vtkDoubleArray> scalars =
          vtkSmartPointer<vtkDoubleArray>::New();
    viewer->SetSlice(101);
    scalars= viewer->GetSlice()-;

*/



    plane->GetOutput()->GetPointData()->SetScalars(randomScalars);
    vtkSmartPointer<vtkPolyData> polyData =
            vtkSmartPointer<vtkPolyData>::New();

    polyData = plane->GetOutput();
    mycontours->SetInputConnection(plane->GetOutputPort());
    mycontours->GenerateValues(5, -100, 100);
    pointThreshold = 0;

    // Connect the segments of the contours into polylines
    vtkSmartPointer<vtkStripper> contourStripper =
            vtkSmartPointer<vtkStripper>::New();
    contourStripper->SetInputConnection( mycontours->GetOutputPort());
    contourStripper->Update();


    // Da qui commentato
    int numberOfContourLines = contourStripper->GetOutput()->GetNumberOfLines();


    std::cout << "There are "
              << numberOfContourLines << " contours lines."
              << std::endl;


    vtkPoints *points     =
            contourStripper->GetOutput()->GetPoints();
    vtkCellArray *cells   =
            contourStripper->GetOutput()->GetLines();
    vtkDataArray *scalars =
            contourStripper->GetOutput()->GetPointData()->GetScalars();

    // Create a polydata that contains point locations for the contour
    // line labels
    vtkSmartPointer<vtkPolyData> labelPolyData =
            vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> labelPoints =
            vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkDoubleArray> labelScalars =
            vtkSmartPointer<vtkDoubleArray>::New();
    labelScalars->SetNumberOfComponents(1);
    labelScalars->SetName("Isovalues");

    const vtkIdType *indices;
    vtkIdType numberOfPoints;
    unsigned int lineCount = 0;
    for (cells->InitTraversal();
         cells->GetNextCell(numberOfPoints, indices);
         lineCount++)
    {
        if (numberOfPoints < pointThreshold)
        {
            continue;
        }
        std::cout << "Line " << lineCount << ": " << std::endl;

        // Compute the point id to hold the label
        // Mid point or a random point
        vtkIdType midPointId = indices[numberOfPoints / 2];
        midPointId =
                indices[static_cast<vtkIdType>(vtkMath::Random(0, numberOfPoints))];

        double midPoint[3];
        points->GetPoint(midPointId, midPoint);
        /*
        std::cout << "\tmidPoint is " << midPointId << " with coordinate "
                  << "("
                  << midPoint[0] << ", "
                  << midPoint[1] << ", "
                  << midPoint[2] << ")"
                  << " and value " << scalars->GetTuple1(midPointId)
                  << std::endl;*/
        labelPoints->InsertNextPoint(midPoint);
        labelScalars->InsertNextTuple1(scalars->GetTuple1(midPointId));
    }
    labelPolyData->SetPoints(labelPoints);
    labelPolyData->GetPointData()->SetScalars(labelScalars);

    vtkSmartPointer<vtkPolyDataMapper> contourMapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
    contourMapper->SetInputConnection(contourStripper->GetOutputPort());
    //contourMapper->ScalarVisibilityOff();



    vtkSmartPointer<vtkActor> isolines =
            vtkSmartPointer<vtkActor>::New();
    isolines->SetMapper(contourMapper);

    vtkSmartPointer<vtkLookupTable> surfaceLUT =
            vtkSmartPointer<vtkLookupTable>::New();
    surfaceLUT->SetRange(
                polyData->GetPointData()->GetScalars()->GetRange());
    surfaceLUT->Build();

    vtkSmartPointer<vtkPolyDataMapper> surfaceMapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();

    surfaceMapper->SetInputData(polyData);

    surfaceMapper->ScalarVisibilityOn();
    surfaceMapper->SetScalarRange(
                polyData->GetPointData()->GetScalars()->GetRange());
    surfaceMapper->SetLookupTable(surfaceLUT);

    vtkSmartPointer<vtkActor> surface =
            vtkSmartPointer<vtkActor>::New();
    surface->SetMapper(surfaceMapper);

    // The labeled data mapper will place labels at the points
    vtkSmartPointer<vtkLabeledDataMapper> labelMapper =
            vtkSmartPointer<vtkLabeledDataMapper>::New();
    labelMapper->SetFieldDataName("Isovalues");

    labelMapper->SetInputData(labelPolyData);

    labelMapper->SetLabelModeToLabelScalars();
    labelMapper->SetLabelFormat("%6.2f");

    vtkSmartPointer<vtkActor2D> isolabels =
            vtkSmartPointer<vtkActor2D>::New();
    isolabels->SetMapper(labelMapper);

    // Create a renderer and render window
    /*vtkSmartPointer<vtkRenderer> renderer =
           vtkSmartPointer<vtkRenderer>::New();

       vtkSmartPointer<vtkRenderWindow> renderWindow =
           vtkSmartPointer<vtkRenderWindow>::New();
       renderWindow->AddRenderer(renderer);

       // Create an interactor
       vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
           vtkSmartPointer<vtkRenderWindowInteractor>::New();
       renderWindowInteractor->SetRenderWindow(renderWindow);*/

    // Add the actors to the scene

    vtkWin->ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(isolines);
    vtkWin->ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(isolabels);

    //renderer->AddActor(isolabels);
    //renderer->AddActor(surface);

    // Render the scene (lights and cameras are created automatically)
    vtkWin->ui->qVTK1->renderWindow()->Render();
    vtkWin->ui->qVTK1->update();

    //renderWindow->Render();
    //renderWindowInteractor->Start();


    //fine TEST


    /* commentato per via del TEST
    contours = vtkMarchingSquares::New();
    contours->SetInputConnection(fitsReader->GetOutputPort());
            //();


    contours->GenerateValues(ui->numOfContourText->text().toInt(),ui->minValueText->text().toDouble(),ui->maxValueText->text().toDouble() );

    qDebug()<<"valori per contours: "<<ui->numOfContourText->text().toInt()<<ui->minValueText->text().toDouble()<<ui->maxValueText->text().toDouble();


    // Connect the segments of the contours into polylines
    contourStripper =  vtkSmartPointer<vtkStripper>::New();
    contourStripper->SetInputConnection(contours->GetOutputPort());
    contourStripper->Update();
    contourStripper->GetOutput()->BuildLinks();*/

    //esempio qui: http://www.vtk.org/Wiki/VTK/Examples/Cxx/Visualization/LabelContours

    /*
     //OK QUESTO FUNZIONA PER RIMUOVERE I VARI CONTOUR
    std::cout<<"PRE- SIZE CELLS: "<<contourStripper->GetOutput()->GetNumberOfCells()<<std::endl;
     contourStripper->GetOutput()->BuildLinks();

     for(int i=0;i<20;i++)
         contourStripper->GetOutput()->DeleteCell(i);
    contourStripper->GetOutput()->RemoveDeletedCells();
    std::cout<<"POST- SIZE CELLS: "<<contourStripper->GetOutput()->GetNumberOfCells()<<std::endl;
*/

    //commentato per via di TEST: codice originario
    /*
    vtkPolyDataMapper *contour_mapper = vtkPolyDataMapper::New();
    //contour_mapper->SetInput(contour->GetOutput());
    contour_mapper->SetInput(contourStripper->GetOutput());
    contour_mapper->ScalarVisibilityOff();

    contour_actor = vtkActor::New();
    contour_actor->SetMapper(contour_mapper);
    contour_actor->GetProperty()->SetColor(ui->redText->text().toDouble(), ui->greenText->text().toDouble(), ui->blueText->text().toDouble());
*/
    //fa parte di TEST
    vtkWin->ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(contour_actor);
    vtkWin->ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(isolabels);

    vtkWin->ui->qVTK1->renderWindow()->Render();
    vtkWin->ui->qVTK1->update();
}

void contour::deleteContour()
{


    contourStripper->GetOutput()->RemoveDeletedCells();



    //vtkWin->removeActor(contour_actor);
    // vtkWin->addActor(contour_actor);

    //contour_actor->GetMapper()->Update();

    vtkWin->ui->qVTK1->renderWindow()->Render();
    std::cout<<"POST- SIZE CELLS: "<<contourStripper->GetOutput()->GetNumberOfCells()<<std::endl;


}

void contour::on_okButton_clicked()
{
    int pointThreshold = 1;

    //std::cout<<"ENTRO DENTRO LA FUNZIONE"<<std::endl;
    //std::cout<<ui->activateCheckBox->isChecked()<<std::endl;

    if(ui->activateCheckBox->isChecked())
    {
        vtkWin->removeActor(isolabels);
        vtkWin->removeActor(isolines);
        vtkWin->removeActor(idlabel);
        vtkWin->removeActor(scalarBar);
        ui->idCheckBox->setChecked(false);

        addContours();

        ui->labelCheckBox->setEnabled(true);
        ui->activateCheckBox->setChecked(true);
        vtkWin->setContourVisualized(true);
        std::cout<<"Labels activated"<<std::endl;

        /*int numberOfContourLines = contourStripper->GetOutput()->GetNumberOfLines();

        std::cout << "****There are " << numberOfContourLines << " contours lines." << std::endl;


        vtkPoints *points     = contourStripper->GetOutput()->GetPoints();
        vtkCellArray *cells   = contourStripper->GetOutput()->GetLines();
        vtkDataArray *scalars = contourStripper->GetOutput()->GetPointData()->GetScalars();

        // Create a polydata that contains point locations for the contour
        // line labels
        vtkSmartPointer<vtkPolyData> labelPolyData = vtkSmartPointer<vtkPolyData>::New();
        vtkSmartPointer<vtkPoints> labelPoints =  vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkDoubleArray> labelScalars = vtkSmartPointer<vtkDoubleArray>::New();



        vtkSmartPointer<vtkPolyData> idPolyData = vtkSmartPointer<vtkPolyData>::New();
        vtkSmartPointer<vtkIntArray> idScalars = vtkSmartPointer<vtkIntArray>::New();

       // vtkSmartPointer<vtkDoubleArray> labelId = vtkSmartPointer<vtkDoubleArray>::New();

        labelScalars->SetNumberOfComponents(1);
        labelScalars->SetName("Isovalues");

     //   labelId->SetNumberOfComponents(1);
      //  labelId->SetName("Id");

        vtkIdType *indices;
        vtkIdType numberOfPoints;
        unsigned int lineCount = 0;
        int i=0;
        for (cells->InitTraversal(); cells->GetNextCell(numberOfPoints, indices);lineCount++)
          {
          if (numberOfPoints < pointThreshold)
            {
            continue;
            }

       //   std::cout << "Line " << lineCount << ":("<<numberOfPoints<<") " << std::endl;
       //   std::cout << "indices " << indices << std::endl;

          // Compute the point id to hold the label
          // Mid point or a random point
          vtkIdType midPointId = indices[numberOfPoints / 2];
          midPointId = indices[static_cast<vtkIdType>(vtkMath::Random(0, numberOfPoints))];

          double midPoint[3];
          points->GetPoint(midPointId, midPoint);*/

        /*già commentato nel codice di fabio
          std::cout << "\tmidPoint is " << midPointId << " with coordinate "
                    << "("
                    << midPoint[0] << ", "
                    << midPoint[1] << ", "
                    << midPoint[2] << ")"
                    << " and value " << scalars->GetTuple1(midPointId)
                    << std::endl;
        */
        /*
          labelPoints->InsertNextPoint(midPoint);
          labelScalars->InsertNextTuple1(scalars->GetTuple1(midPointId));

          idScalars->InsertNextTuple1(i);
     //     labelId->->InsertNextTuple1(scalars->GetTuple1(midPointId));
          addTreeRoot( QString::number(i), QString::number(labelScalars->GetTuple1(i)),midPoint[0],midPoint[1],midPoint[2]);

          i++;
        }
        labelPolyData->SetPoints(labelPoints);
        labelPolyData->GetPointData()->SetScalars(labelScalars);

        idPolyData->SetPoints(labelPoints);
        idPolyData->GetPointData()->SetScalars(idScalars);


        // The labeled data mapper will place labels at the points
        vtkSmartPointer<vtkLabeledDataMapper> labelMapper = vtkSmartPointer<vtkLabeledDataMapper>::New();
        labelMapper->SetFieldDataName("Isovalues");
        labelMapper->SetInput(labelPolyData);
        labelMapper->SetLabelModeToLabelScalars();
        labelMapper->SetLabelFormat("%6.2f");


        // The labeled data mapper will place labels at the points
        vtkSmartPointer<vtkLabeledDataMapper> idMapper = vtkSmartPointer<vtkLabeledDataMapper>::New();
        idMapper->SetFieldDataName("Id");
        idMapper->SetInput(idPolyData);
        idMapper->SetLabelModeToLabelScalars();

        isolabels = vtkSmartPointer<vtkActor2D>::New();
        isolabels->SetMapper(labelMapper);

        idlabel = vtkSmartPointer<vtkActor2D>::New();
        idlabel->SetMapper(idMapper);*/

    }
    else{
        vtkWin->setContourVisualized(false);
        ui->labelCheckBox->setEnabled(false);
        ui->idCheckBox->setEnabled(false);
    }


}


void contour::addContours()
{

    /*
    qDebug()<<"Window name:"<<vtkWin->getWindowName();

    vtkSmartPointer<vtkImageToStructuredPoints> convertFilter =
    vtkSmartPointer<vtkImageToStructuredPoints>::New();

    qDebug()<<"Get slice *"<<vtkWin->imageViewer->GetSlice();

    convertFilter->SetInput(vtkWin->imageViewer->GetInput());
    convertFilter->Update();

    vtkStructuredPoints *myPoint= vtkStructuredPoints::New();

    myPoint=convertFilter->GetStructuredPointsOutput();
    myPoint->GetNumberOfPoints();
    qDebug()<<"GetNumberOfPoints:"<<myPoint->GetNumberOfPoints();
    qDebug()<<"GetNumberOfScalarComponents:"<<myPoint->GetNumberOfScalarComponents();
    qDebug()<<"GetNumberOfCells:"<<myPoint->GetNumberOfCells();


    qDebug()<<"fits file"<<fitsReader->GetNaxes(0)<<fitsReader->GetNaxes(1)<<fitsReader->GetNaxes(2);

*/
    //start cutter:
    vtkSmartPointer<vtkCutter> cutter =
            vtkSmartPointer<vtkCutter>::New();
    // Create a plane to cut,here it cuts in the XZ direction (xz normal=(1,0,0);XY =(0,0,1),YZ =(0,1,0)

    if(vtkWin->ui->spinBox_channels->text().toInt()==0)
    {
        vtkSmartPointer<vtkPlane> plane =
                vtkSmartPointer<vtkPlane>::New();

         plane->SetOrigin(0,0,vtkWin->viewer->GetSlice());
        //  plane->SetOrigin(0,0,vtkWin->imageViewer->GetSlice());
        plane->SetNormal(0,0,1);
        // Set cutter function for a single planes
        cutter->SetCutFunction(plane);
    }
    else
    {

        vtkSmartPointer<vtkPlanes> planes =
                vtkSmartPointer<vtkPlanes>::New();
        planes->SetBounds(0, fitsReader->GetNaxes(0), 0, fitsReader->GetNaxes(1), vtkWin->imageViewer->GetSlice()- vtkWin->ui->spinBox_channels->text().toInt(), vtkWin->imageViewer->GetSlice()+vtkWin->ui->spinBox_channels->text().toInt() );

        vtkSmartPointer<vtkPoints> points =vtkSmartPointer<vtkPoints>::New();
        points=planes->GetPoints();


        int i;
        double *point;
        for (i=0;i<points->GetNumberOfPoints();i++)
        {
            point=points->GetPoint(i);
        }


        // Set cutter function for planes
        cutter->SetCutFunction(planes);

    }


    cutter->SetInputData(fitsReader->GetOutput());
    cutter->Update();

    vtkSmartPointer<vtkPolyDataMapper> cutterMapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
    cutterMapper->SetInputConnection( cutter->GetOutputPort());



    // Create plane actor
    vtkSmartPointer<vtkActor> planeActor =
            vtkSmartPointer<vtkActor>::New();
    planeActor->GetProperty()->SetColor(1.0,1,0);
    planeActor->GetProperty()->SetLineWidth(2);
    planeActor->SetMapper(cutterMapper);



    // Create renderers and add actors of plane and cube
    vtkSmartPointer<vtkRenderer> renderer =
            vtkSmartPointer<vtkRenderer>::New();
    renderer->AddActor(planeActor); //display the rectangle resulting from the cut
    //renderer->AddActor(cubeActor); //display the cube

    // Add renderer to renderwindow and render
    vtkSmartPointer<vtkRenderWindow> renderWindow =
            vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    renderWindow->SetSize(600, 600);

    vtkSmartPointer<vtkRenderWindowInteractor> interactor =
            vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(renderWindow);
    renderer->SetBackground(0,0,0);


    //end cutter


    int pointThreshold = 10;

    vtkSmartPointer<vtkPolyData> polyData =
            vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkContourFilter> contoursFilter =
            vtkSmartPointer<vtkContourFilter>::New();

    polyData = cutter->GetOutput();
   contoursFilter->GenerateValues(ui->numOfContourText->text().toInt(), ui->minValueText->text().toDouble(), ui->maxValueText->text().toDouble());

    //contoursFilter->SetValue(0, abs((abs(ui->maxValueText->text().toDouble())-abs(ui->minValueText->text().toDouble()))) / 2.0);

    contoursFilter->SetInputConnection(cutter->GetOutputPort());

    // Connect the segments of the conours into polylines
    vtkSmartPointer<vtkStripper> contourStripper =
            vtkSmartPointer<vtkStripper>::New();
    contourStripper->SetInputConnection(contoursFilter->GetOutputPort());
    contourStripper->Update();

    int numberOfContourLines = contourStripper->GetOutput()->GetNumberOfLines();

    std::cout << "There are "
              << numberOfContourLines << " contours lines."
              << std::endl;

    vtkPoints *points     =
            contourStripper->GetOutput()->GetPoints();
    vtkCellArray *cells   =
            contourStripper->GetOutput()->GetLines();
    vtkDataArray *scalars =
            contourStripper->GetOutput()->GetPointData()->GetScalars();

    // Create a polydata that contains point locations for the contour
    // line labels
    vtkSmartPointer<vtkPolyData> labelPolyData =
            vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> labelPoints =
            vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkDoubleArray> labelScalars =
            vtkSmartPointer<vtkDoubleArray>::New();
    labelScalars->SetNumberOfComponents(1);
    labelScalars->SetName("Isovalues");

    const vtkIdType *indices;
    vtkIdType numberOfPoints;
    unsigned int lineCount = 0;
    for (cells->InitTraversal();
         cells->GetNextCell(numberOfPoints, indices);
         lineCount++)
    {
        if (numberOfPoints < pointThreshold)
        {
            continue;
        }
        //std::cout << "Line " << lineCount << ": " << std::endl;

        // Compute the point id to hold the label
        // Mid point or a random point
        vtkIdType midPointId = indices[numberOfPoints / 2];
        midPointId =
                indices[static_cast<vtkIdType>(vtkMath::Random(0, numberOfPoints))];

        double midPoint[3];
        points->GetPoint(midPointId, midPoint);
        /*std::cout << "\tmidPoint is " << midPointId << " with coordinate "
                         << "("
                         << midPoint[0] << ", "
                         << midPoint[1] << ", "
                         << midPoint[2] << ")"
                         << " and value " << scalars->GetTuple1(midPointId)
                         << std::endl;*/
        labelPoints->InsertNextPoint(midPoint);
        labelScalars->InsertNextTuple1(scalars->GetTuple1(midPointId));
    }
    labelPolyData->SetPoints(labelPoints);
    labelPolyData->GetPointData()->SetScalars(labelScalars);

    vtkSmartPointer<vtkPolyDataMapper> contourMapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
    contourMapper->SetInputConnection(contourStripper->GetOutputPort());
    //contourMapper->ScalarVisibilityOff();


    /*START LUT*/
    contourMapper->ScalarVisibilityOn();
    contourMapper->SetScalarModeToUsePointData();
    contourMapper->SetColorModeToMapScalars();

    contourMapper->SetScalarRange(fitsReader->GetMin(), fitsReader->GetMax());

    //vtkSmartPointer<vtkScalarBarActor>
    scalarBar =
            vtkSmartPointer<vtkScalarBarActor>::New();

    //scalarBar->SetLookupTable(contourMapper->GetLookupTable());
    //scalarBar->SetTitle("Contours Scalar Bar");
    scalarBar->DragableOn();

    scalarBar->SetNumberOfLabels(5);
    //qDebug()<<"GetTextPosition:"<<scalarBar->GetTextPosition();

    //scalarBar->SetOrientationToHorizontal();

    // Create one text property for all
    vtkSmartPointer<vtkTextProperty> textProperty =
            vtkSmartPointer<vtkTextProperty>::New();
    textProperty->SetFontSize(0.1);
    //textProperty->SetJustificationToCentered();
    textProperty->SetColor(0.4,0.4,1);
    textProperty->SetFontFamilyToArial();

    scalarBar->SetLabelTextProperty(textProperty);

    //scalarBar->SetTextureGridWidth(0.1);
    scalarBar->SetWidth(0.05);
    scalarBar->SetHeight(0.9);

    vtkSmartPointer<vtkTextProperty> titleTextProperty =
            vtkSmartPointer<vtkTextProperty>::New();
    titleTextProperty->SetFontSize(24);
    titleTextProperty->SetFontFamilyToArial();
    titleTextProperty->SetBold(true);
    titleTextProperty->SetColor(1.,0.,0.);

    scalarBar->SetTitleTextProperty(titleTextProperty);
    //vtkWin->showColorbar(true);


    // Create a lookup table to share between the mapper and the scalarbar
    vtkSmartPointer<vtkLookupTable> hueLut =
            vtkSmartPointer<vtkLookupTable>::New();
    //hueLut->SetTableRange (0, 1);
    hueLut->SetHueRange (0, 1);
    hueLut->SetSaturationRange (1, 1);
    hueLut->SetValueRange (1, 1);
    hueLut->Build();

    contourMapper->SetLookupTable( hueLut );
    scalarBar->SetLookupTable( hueLut );

    //# create the scalar_bar_widget
    //vtkSmartPointer<vtkScalarBarWidget>  scalar_bar_widget = vtkSmartPointer<vtkScalarBarWidget>::New();
    //scalar_bar_widget->SetInteractor(vtkWin->ui->qVTK1->GetRenderWindow()->GetInteractor());//QVTKOpenGLWindow::GetRenderWindow() is deprecated, use renderWindow() instead.
    //scalar_bar_widget->SetScalarBarActor(scalarBar);
    //scalar_bar_widget->On();
    //scalar_bar_widget->SetEnabled(1);

    /*

        vtkAxesWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
        vtkAxesWidget->SetInteractor(ui->qVTK1->GetRenderWindow()->GetInteractor());//QVTKOpenGLWindow::GetRenderWindow() is deprecated, use renderWindow() instead.

        vtkAxesWidget->SetOrientationMarker(vtkAxes);

        vtkAxesWidget->SetOutlineColor( 0.9300, 0.5700, 0.1300 );
        vtkAxesWidget->SetViewport( 0.0, 0.0, 0.2, 0.2 );
        vtkAxesWidget->SetEnabled(1);
        vtkAxesWidget->InteractiveOff();
*/


    //vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
    //lut->SetTableRange( 0, 255 );
    //qDebug()<<"my LUT scale: "<<lut->GetScale();
    //SelectLookTable("glow",lut);
    //contourMapper->SetLookupTable(lut);
    //qDebug()<<"cutterMapper LUT scale: "<<contourMapper->GetLookupTable();

    //ui->qVTK1->update();

    /*FINE LUT*/


    // isolines =
    //   vtkSmartPointer<vtkActor>::New();
    isolines->SetMapper(contourMapper);


    //contour_actor=vtkSmartPointer<vtkActor>::New();
    //contour_actor->SetMapper(contourMapper);

    vtkSmartPointer<vtkLookupTable> surfaceLUT =
            vtkSmartPointer<vtkLookupTable>::New();
    surfaceLUT->SetRange(
                polyData->GetPointData()->GetScalars()->GetRange());
    surfaceLUT->Build();

    vtkSmartPointer<vtkPolyDataMapper> surfaceMapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();

    surfaceMapper->SetInputData(polyData);

    surfaceMapper->ScalarVisibilityOn();
    surfaceMapper->SetScalarRange(
                polyData->GetPointData()->GetScalars()->GetRange());
    surfaceMapper->SetLookupTable(surfaceLUT);

    vtkSmartPointer<vtkActor> surface =
            vtkSmartPointer<vtkActor>::New();
    surface->SetMapper(surfaceMapper);

    // The labeled data mapper will place labels at the points
    vtkSmartPointer<vtkLabeledDataMapper> labelMapper =
            vtkSmartPointer<vtkLabeledDataMapper>::New();
    labelMapper->SetFieldDataName("Isovalues");

    labelMapper->SetInputData(labelPolyData);

    labelMapper->SetLabelModeToLabelScalars();
    labelMapper->SetLabelFormat("%6.2f");




    //vtkSmartPointer<vtkActor2D> isolabels =
    isolabels= vtkSmartPointer<vtkActor2D>::New();
    isolabels->SetMapper(labelMapper);


    // Create an interactor
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
            vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);


    //colore
    //isolines->GetProperty()->SetColor(ui->redText->text().toDouble(), ui->greenText->text().toDouble(), ui->blueText->text().toDouble());



    vtkWin->m_Ren2->AddActor(isolines);
   // vtkWin->m_Ren2->AddActor(scalarBar);
   // vtkWin->m_Ren2->AddActor(idlabel);
    if (ui->labelCheckBox->isChecked()==true)
        vtkWin->m_Ren2->AddActor(isolabels);
    vtkWin->m_Ren2->Render();
    vtkWin->ui->isocontourVtkWin->update();
    // Add the actors to the scene
    //renderer->AddActor(isolines);
    //renderer->AddActor(isolabels);8
    //  renderer->AddActor(surface);

    // Render the scene (lights and cameras are created automatically)
    //renderWindow->Render();
    //renderWindowInteractor->Start();




    /*
    vtkMarchingSquares *contour = vtkMarchingSquares::New();

    contour->SetInput(cutter->GetOutput());

    contour->GenerateValues(5,fitsReader->GetMin(),fitsReader->GetMax());

    // Connect the segments of the conours into polylines
    vtkSmartPointer<vtkStripper>contourStripper =  vtkSmartPointer<vtkStripper>::New();
    contourStripper->SetInputConnection(contour->GetOutputPort());
    contourStripper->Update();
    contourStripper->GetOutput()->BuildLinks();


    vtkPolyDataMapper *contour_mapper = vtkPolyDataMapper::New();
    //contour_mapper->SetInput(contour->GetOutput());
    contour_mapper->SetInputConnection(contourStripper->GetOutputPort());
    contour_mapper->ScalarVisibilityOff();

    qDebug()<<"add actor contour";
    vtkActor* contour_actor = vtkActor::New();
    contour_actor->SetMapper(contour_mapper);
    //vtkWin->m_Ren1->AddActor(contour_actor);


    //UFFa
    // Add the actors
    renderer->AddActor(contour_actor);
    // Begin interaction
    //renderWindow->Render();


    // Create renderers and add actors of plane and cube
    vtkSmartPointer<vtkRenderer> renderer2 =
            vtkSmartPointer<vtkRenderer>::New();
    renderer2->AddActor(contour_actor); //display the rectangle resulting from the cut
    //renderer->AddActor(cubeActor); //display the cube

    // Add renderer to renderwindow and render
    vtkSmartPointer<vtkRenderWindow> renderWindow2 =
            vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow2->AddRenderer(renderer);
    renderWindow2->SetSize(600, 600);

    vtkSmartPointer<vtkRenderWindowInteractor> interactor2 =
            vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor2->SetRenderWindow(renderWindow);
    renderer2->SetBackground(0,0,0);


    renderWindow->Render();
    interactor->Start();


    renderWindow2->Render();
    interactor2->Start();

    qDebug()<<"fine me";*/



}

void contour::addContours2()
{


    // Create a plane to cut,here it cuts in the XZ direction (xz normal=(1,0,0);XY =(0,0,1),YZ =(0,1,0)
    vtkSmartPointer<vtkPlane> plane =
            vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(0,0,vtkWin->imageViewer->GetSlice());
    plane->SetNormal(0,0,1);


    // Create cutter
    vtkSmartPointer<vtkCutter> cutter =
            vtkSmartPointer<vtkCutter>::New();
    cutter->SetCutFunction(plane);
    cutter->SetInputConnection(fitsReader->GetOutputPort());
    cutter->GenerateValues(5, fitsReader->GetMin(), fitsReader->GetMax());
    cutter->Update();

    vtkSmartPointer<vtkPolyDataMapper> cutterMapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
    cutterMapper->SetInputConnection ( cutter->GetOutputPort());
    //cutterMapper->ScalarVisibilityOff();


    // Create plane actor
    vtkSmartPointer<vtkActor> planeActor =
            vtkSmartPointer<vtkActor>::New();
    planeActor->GetProperty()->SetColor(1.0,1,0);
    planeActor->GetProperty()->SetLineWidth(2);
    planeActor->SetMapper(cutterMapper);

    // Create renderers and add actors of plane and cube
    vtkSmartPointer<vtkRenderer> renderer =
            vtkSmartPointer<vtkRenderer>::New();
    renderer->AddActor(planeActor); //display the rectangle resulting from the cut
    //renderer->AddActor(cubeActor); //display the cube

    // Add renderer to renderwindow and render
    vtkSmartPointer<vtkRenderWindow> renderWindow =
            vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    renderWindow->SetSize(600, 600);

    vtkSmartPointer<vtkRenderWindowInteractor> interactor =
            vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(renderWindow);
    renderer->SetBackground(0,0,0);

    renderWindow->Render();
    interactor->Start();

}


void contour::createwin()
{

}
