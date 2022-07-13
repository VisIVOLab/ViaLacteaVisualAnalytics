#include "vtkwindow_new.h"
#include "ui_vtkwindow_new.h"

#include "astroutils.h"
#include "authwrapper.h"
#include "caesarwindow.h"
#include "catalogue.h"
#include "dbquery.h"
#include "extendedglyph3d.h"
#include "filtercustomize.h"
#include "fitsimagestatisiticinfo.h"
#include "higalselectedsources.h"
#include "lutcustomize.h"
#include "luteditor.h"
#include "qdebug.h"
#include "selectedsourcefieldsselect.h"
#include "selectedsourcesform.h"
#include "sfilterdialog.h"
#include "singleton.h"
#include "source.h"
#include "sourcewidget.h"
#include "ui_higalselectedsources.h"
#include "vialactea.h"
#include "vialactea_fileload.h"
#include "vialacteainitialquery.h"
#include "vispoint.h"
#include "vlkbsimplequerycomposer.h"
#include "vtkAbstractPicker.h"
#include "vtkAppendPolyData.h"
#include "vtkAreaPicker.h"
#include "vtkAssembly.h"
#include "vtkAutoInit.h"
#include "vtkAxisActor2D.h"
#include "vtkCaptionActor2D.h"
#include "vtkCellPicker.h"
#include "vtkCleanPolyData.h"
#include "vtkClipPolyData.h"
#include "vtkCollection.h"
#include "vtkContourFilter.h"
#include "vtkCornerAnnotation.h"
#include "vtkCubeAxesActor.h"
#include "vtkCubeAxesActor2D.h"
#include "vtkCutter.h"
#include "vtkDataSetMapper.h"
#include "vtkDoubleArray.h"
#include "vtkExtractGeometry.h"
#include "vtkextracthistogram.h"
#include "vtkExtractPolyDataGeometry.h"
#include "vtkExtractSelectedFrustum.h"
#include "vtkExtractSelection.h"
#include "vtkfitstoolswidget.h"
#include "vtkfitstoolwidget_new.h"
#include "vtkfitstoolwidgetobject.h"
#include "vtkFrustumSource.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkGeometryFilter.h"
#include "vtkGlyph2D.h"
#include "vtkHardwareSelector.h"
#include "vtkImageActorPointPlacer.h"
#include "vtkImageBlend.h"
#include "vtkImageChangeInformation.h"
#include "vtkImageDataGeometryFilter.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkImageProperty.h"
#include "vtkImageResize.h"
#include "vtkImageShiftScale.h"
#include "vtkImageSliceCollection.h"
#include "vtkImageSliceMapper.h"
#include "vtkImageStack.h"
#include "vtkInteractorStyleDrawPolygon.h"
#include "vtkInteractorStyleImage.h"
#include "vtkInteractorStyleRubberBand2D.h"
#include "vtkInteractorStyleRubberBandPick.h"
#include "vtkInteractorStyleTrackballActor.h"
#include "vtklegendscaleactor.h"
#include "vtkMath.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkOutlineFilter.h"
#include "vtkPlane.h"
#include "vtkPlanes.h"
#include "vtkPointPicker.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyLine.h"
#include "vtkProp3D.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkPropPicker.h"
#include "vtkRegularPolygonSource.h"
#include "vtkRemovePolyData.h"
#include "vtkRendererCollection.h"
#include "vtkSelection.h"
#include "vtkStringArray.h"
#include "vtkStripper.h"
#include "vtkTable.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtktoolswidget.h"
#include "vtkTransform.h"
#include "vtkUnstructuredGrid.h"
#include "vtkVertexGlyphFilter.h"

#include "ds9region/DS9Region.h"
#include "ds9region/DS9RegionParser.h"

#include <QDir>
#include <QSettings>
#include <QSignalMapper>
#include <QtConcurrent>

VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)
// VTK_MODULE_INIT(vtkRenderingFreeTypeOpenGL)
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2)

#define VTK_NEW(type, instance) vtkSmartPointer<type> instance = vtkSmartPointer<type>::New();

class InteractorStyleFreeHandOn3DVisualization : public vtkInteractorStyleDrawPolygon
{
private:
    vtkwindow_new *vtkwin;
    vtkSmartPointer<vtkPolyData> Points;
    vtkSmartPointer<vtkPolyData> Points_ori;
    vtkSmartPointer<vtkActor> SelectedActor;
    vtkSmartPointer<vtkPolyDataMapper> SelectedMapper;

public:
    static InteractorStyleFreeHandOn3DVisualization *New();
    vtkTypeMacro(InteractorStyleFreeHandOn3DVisualization, vtkInteractorStyleDrawPolygon);

    InteractorStyleFreeHandOn3DVisualization()
    {
        this->SelectedMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        this->SelectedActor = vtkSmartPointer<vtkActor>::New();
        this->SelectedActor->SetMapper(SelectedMapper);
    }

    virtual void OnLeftButtonUp()
    {

        vtkInteractorStyleDrawPolygon::OnLeftButtonUp();
        std::vector<vtkVector2i> points = this->GetPolygonPoints();

        if (points.size() >= 3) {
            vtkNew<vtkIntArray> polygonPointsArray;
            polygonPointsArray->SetNumberOfComponents(2);
            polygonPointsArray->SetNumberOfTuples(points.size());
            for (unsigned int j = 0; j < points.size(); ++j) {
                const vtkVector2i &v = points[j];
                int pos[2] = { v[0], v[1] };
                polygonPointsArray->SetTypedTuple(j, pos);
            }

            vtkNew<vtkHardwareSelector> hardSel;
            hardSel->SetRenderer(
                    this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

            int *wsize = this->GetInteractor()
                                 ->GetRenderWindow()
                                 ->GetRenderers()
                                 ->GetFirstRenderer()
                                 ->GetSize();
            int *origin = this->GetInteractor()
                                  ->GetRenderWindow()
                                  ->GetRenderers()
                                  ->GetFirstRenderer()
                                  ->GetOrigin();
            hardSel->SetArea(origin[0], origin[1], origin[0] + wsize[0] - 1,
                             origin[1] + wsize[1] - 1);
            hardSel->SetFieldAssociation(vtkDataObject::FIELD_ASSOCIATION_POINTS);

            if (hardSel->CaptureBuffers()) {

                vtkSelection *psel = hardSel->GeneratePolygonSelection(
                        polygonPointsArray->GetPointer(0),
                        polygonPointsArray->GetNumberOfTuples() * 2);
                hardSel->ClearBuffers();

                vtkSmartPointer<vtkSelection> sel;
                sel.TakeReference(psel);

                vtkSmartPointer<vtkExtractSelection> extractSelection =
                        vtkSmartPointer<vtkExtractSelection>::New();

                extractSelection->SetInputData(this->Points);

                extractSelection->SetInputData(1, sel);

                extractSelection->Update();

                // In selection
                vtkSmartPointer<vtkUnstructuredGrid> selected =
                        vtkSmartPointer<vtkUnstructuredGrid>::New();
                selected->ShallowCopy(extractSelection->GetOutput());

                std::cout << "There are " << selected->GetNumberOfPoints()
                          << " points in the selection." << std::endl;
                std::cout << "There are " << selected->GetNumberOfCells()
                          << " cells in the selection." << std::endl;

                vtkSmartPointer<vtkGeometryFilter> geometryFilter =
                        vtkSmartPointer<vtkGeometryFilter>::New();

                geometryFilter->SetInputData(selected);

                geometryFilter->Update();
                vtkPolyData *selected_poly = geometryFilter->GetOutput();

                this->SelectedMapper->SetInputData(selected_poly);

                this->SelectedMapper->ScalarVisibilityOff();

                double r = vtkMath::Random(0.0, 1.0);
                double g = vtkMath::Random(0.0, 1.0);
                double b = vtkMath::Random(0.0, 1.0);

                // START
                std::cout << "Selected " << selected->GetNumberOfPoints() << " points."
                          << std::endl;
                std::cout << "Selected " << selected->GetNumberOfCells() << " cells." << std::endl;
                // END

                this->SelectedActor->GetProperty()->SetColor(r, g, b); //(R,G,B)
                this->SelectedActor->GetProperty()->SetPointSize(3);
                this->GetInteractor()
                        ->GetRenderWindow()
                        ->GetRenderers()
                        ->GetFirstRenderer()
                        ->AddActor(SelectedActor);
                this->GetInteractor()->GetRenderWindow()->Render();
                this->HighlightProp(NULL);

                /*
                    if(selected->GetNumberOfPoints()>0)
                    {
                        this->CurrentRenderer->RemoveActor(vtkwin->selectedActor);
                        vtkwin->setSelectedActor(SelectedActor);
                        vtkwin->setVtkInteractorStyle3DFreehand(selected_poly);
                    }
                */
            }
        }
    }

    void SetPoints(vtkSmartPointer<vtkPolyData> points)
    {
        this->Points = points;
        this->Points_ori = points;
    }

    void setVtkWin(vtkwindow_new *w) { vtkwin = w; }

    virtual void PrintSelf(std::ostream &os, vtkIndent indent) { }

    virtual void PrintHeader(ostream &os, vtkIndent indent) { }

    virtual void PrintTrailer(std::ostream &os, vtkIndent indent) { }
    virtual void CollectRevisions(std::ostream &os) { }
};
vtkStandardNewMacro(InteractorStyleFreeHandOn3DVisualization);

class InteractorStyleSelctionPointOn3DVisualization : public vtkInteractorStyleRubberBandPick
{
private:
    vtkwindow_new *vtkwin;
    vtkSmartPointer<vtkPolyData> Points;
    vtkSmartPointer<vtkPolyData> Points_ori;
    vtkSmartPointer<vtkActor> SelectedActor;
    vtkSmartPointer<vtkPolyDataMapper> SelectedMapper;

public:
    static InteractorStyleSelctionPointOn3DVisualization *New();
    vtkTypeMacro(InteractorStyleSelctionPointOn3DVisualization, vtkInteractorStyleRubberBandPick);

    InteractorStyleSelctionPointOn3DVisualization()
    {
        // this->SelectedMapper = vtkSmartPointer<vtkDataSetMapper>::New();
        this->SelectedMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        this->SelectedActor = vtkSmartPointer<vtkActor>::New();
        this->SelectedActor->SetMapper(SelectedMapper);
    }

    virtual void OnLeftButtonUp()
    {

        vtkInteractorStyleRubberBandPick::OnLeftButtonUp();

        // Forward events
        vtkPlanes *frustum =
                static_cast<vtkAreaPicker *>(this->GetInteractor()->GetPicker())->GetFrustum();

        vtkSmartPointer<vtkExtractGeometry> extractGeometry =
                vtkSmartPointer<vtkExtractGeometry>::New();
        extractGeometry->SetImplicitFunction(frustum);

        extractGeometry->SetInputData(this->Points);

        extractGeometry->Update();

        vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter =
                vtkSmartPointer<vtkVertexGlyphFilter>::New();
        glyphFilter->SetInputConnection(extractGeometry->GetOutputPort());
        glyphFilter->Update();

        vtkPolyData *selected = glyphFilter->GetOutput();

        this->SelectedMapper->SetInputData(selected);

        this->SelectedMapper->ScalarVisibilityOff();

        double r = vtkMath::Random(0.0, 1.0);
        double g = vtkMath::Random(0.0, 1.0);
        double b = vtkMath::Random(0.0, 1.0);

        // START
        std::cout << "Selected " << selected->GetNumberOfPoints() << " points." << std::endl;
        std::cout << "Selected " << selected->GetNumberOfCells() << " cells." << std::endl;
        /*
                    vtkIdTypeArray* ids =
           vtkIdTypeArray::SafeDownCast(selected->GetPointData()->GetArray("ids")); for(vtkIdType i
           = 0; i < ids->GetNumberOfTuples(); i++)
                      {
                      std::cout << "Id " << i << " : " << ids->GetValue(i) << std::endl;
                      }
        */
        // END

        this->SelectedActor->GetProperty()->SetColor(r, g, b); //(R,G,B)
        this->SelectedActor->GetProperty()->SetPointSize(3);
        this->CurrentRenderer->AddActor(SelectedActor);
        // this->GetInteractor()->GetRenderWindow()->Render();
        this->HighlightProp(NULL);

        if (selected->GetNumberOfPoints() > 0) {

            this->CurrentRenderer->RemoveActor(vtkwin->selectedActor);
            this->GetInteractor()->GetRenderWindow()->Render();
            vtkwin->setSelectedActor(SelectedActor);
            vtkwin->setVtkInteractorStyle3DPicker(selected);
        }
        //  this->GetInteractor()->GetRenderWindow()->Render();

        /*
        // Visualize
        vtkSmartPointer<vtkRenderer> renderer =
          vtkSmartPointer<vtkRenderer>::New();
        vtkSmartPointer<vtkRenderWindow> renderWindow =
          vtkSmartPointer<vtkRenderWindow>::New();
        renderWindow->AddRenderer(renderer);
        vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
          vtkSmartPointer<vtkRenderWindowInteractor>::New();
        renderWindowInteractor->SetRenderWindow(renderWindow);
        renderer->AddActor(SelectedActor);
        renderWindow->Render();
        renderWindowInteractor->Start();
*/
    }

    void SetPoints(vtkSmartPointer<vtkPolyData> points)
    {
        this->Points = points;
        this->Points_ori = points;
    }

    void setVtkWin(vtkwindow_new *w) { vtkwin = w; }

    virtual void PrintSelf(std::ostream &os, vtkIndent indent) { }

    virtual void PrintHeader(ostream &os, vtkIndent indent) { }

    virtual void PrintTrailer(std::ostream &os, vtkIndent indent) { }
    virtual void CollectRevisions(std::ostream &os) { }
};
vtkStandardNewMacro(InteractorStyleSelctionPointOn3DVisualization);

class MyRubberBand : public vtkInteractorStyleRubberBand2D
{
private:
    vtkwindow_new *vtkwin;

public:
    static MyRubberBand *New();
    vtkTypeMacro(MyRubberBand, vtkInteractorStyleRubberBand2D);

    void setVtkWin(vtkwindow_new *w) { vtkwin = w; }

    virtual void OnMouseMove()
    {

        // Forward events
        vtkSmartPointer<vtkCoordinate> coordinate = vtkSmartPointer<vtkCoordinate>::New();
        coordinate->SetCoordinateSystemToDisplay();
        coordinate->SetValue(this->GetInteractor()->GetEventPosition()[0],
                             this->GetInteractor()->GetEventPosition()[1], 0);

        double *world_coord = coordinate->GetComputedWorldValue(
                this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
        double *sky_coord = new double[2];
        double *sky_coord_gal = new double[2];
        double *sky_coord_fk5 = new double[2];

        QString statusBarText = "";
        float *pixel;
        pixel = static_cast<float *>(vtkwin->getFitsImage()->GetOutput()->GetScalarPointer(
                world_coord[0], world_coord[1], 0));

        statusBarText = "<value> ";
        if (pixel != NULL)
            statusBarText += QString::number(pixel[0]);
        else
            statusBarText += "NaN";

        statusBarText += " <image> X: " + QString::number(world_coord[0])
                + " Y: " + QString::number(world_coord[1]);

        // WCS_GALACTIC = 3
        AstroUtils().xy2sky(vtkwin->filenameWithPath, world_coord[0], world_coord[1], sky_coord_gal,
                            3);
        statusBarText += " <galactic> GLON: " + QString::number(sky_coord_gal[0])
                + " GLAT: " + QString::number(sky_coord_gal[1]);

        // WCS_J2000 = 1
        AstroUtils().xy2sky(vtkwin->filenameWithPath, world_coord[0], world_coord[1], sky_coord_fk5,
                            1);
        statusBarText += " <fk5> RA: " + QString::number(sky_coord_fk5[0])
                + " DEC: " + QString::number(sky_coord_fk5[1]);
        AstroUtils().xy2sky(vtkwin->filenameWithPath, world_coord[0], world_coord[1], sky_coord);
        statusBarText += " <ecliptic> RA: " + QString::number(sky_coord[0])
                + " DEC: " + QString::number(sky_coord[1]);

        vtkwin->ui->statusbar->showMessage(statusBarText);
        vtkInteractorStyleRubberBand2D::OnMouseMove();
    }

    virtual void OnLeftButtonUp()
    {

        // Forward events
        vtkSmartPointer<vtkCoordinate> coordinate = vtkSmartPointer<vtkCoordinate>::New();
        coordinate->SetCoordinateSystemToDisplay();
        coordinate->SetValue(this->StartPosition[0], this->StartPosition[1], 0);

        double *world_start = coordinate->GetComputedWorldValue(
                this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

        vtkSmartPointer<vtkCoordinate> coordinate_end = vtkSmartPointer<vtkCoordinate>::New();
        coordinate_end->SetCoordinateSystemToDisplay();
        coordinate_end->SetValue(this->EndPosition[0], this->EndPosition[1], 0);

        double *world_end = coordinate_end->GetComputedWorldValue(
                this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

        double w = world_end[0] - world_start[0];
        double h = world_end[1] - world_start[1];
        vtkRectf *rect = new vtkRectf(world_start[0], world_start[1], w, h);

        HigalSelectedSources *selectedSources = new HigalSelectedSources(vtkwin);

        QHash<QString, vtkSmartPointer<vtkLODActor>>::iterator i;
        QHash<QString, vtkSmartPointer<vtkLODActor>> tmp = vtkwin->getEllipseActorList();

        QHash<QString, QListWidget *> listWidget_list;

        for (i = tmp.begin(); i != tmp.end(); ++i) {

            QListWidget *tmpWidget = new QListWidget();
            tmpWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
            listWidget_list.insert(i.key(), tmpWidget);
            selectedSources->ui->tabWidget->addTab(tmpWidget, i.key().split("_").at(0));
            selectedSources->setConnect(tmpWidget);
        }

        bool empty = true;
        vtkEllipse *el;

        // quifv
        foreach (el, vtkwin->getEllipseList())
        // foreach( el, vtkwin->getFtEllipseList() )
        {

            if (el->isInsideRect(rect)) {

                qDebug() << el->getSourceName();
                QListWidgetItem *newItem = new QListWidgetItem;
                newItem->setText(el->getSourceName());
                QString name = vtkwin->getDesignation2fileMap().value(el->getSourceName());

                int row = listWidget_list.value(name)->row(
                        listWidget_list.value(name)->currentItem());

                listWidget_list.value(name)->insertItem(row, newItem);
                empty = false;
            }
        }

        // rimuovo i tab vuoti
        QHash<QString, QListWidget *>::iterator it;

        int index;

        for (it = listWidget_list.begin(); it != listWidget_list.end(); ++it) {
            if (it.value()->count() == 0) {
                index = selectedSources->ui->tabWidget->indexOf(it.value());
                selectedSources->ui->tabWidget->removeTab(index);
            }
        }

        if (empty)
            delete selectedSources;
        else
            selectedSources->show();

        vtkInteractorStyleRubberBand2D::OnLeftButtonUp();
        vtkwin->setVtkInteractorStyleImage();
    }

    virtual void PrintSelf(std::ostream &os, vtkIndent indent) { }
    virtual void PrintHeader(ostream &os, vtkIndent indent) { }
    virtual void PrintTrailer(std::ostream &os, vtkIndent indent) { }
    virtual void CollectRevisions(std::ostream &os) { }
};
vtkStandardNewMacro(MyRubberBand);

class myVtkInteractorContourWindow : public vtkInteractorStyleImage
{
private:
    vtkwindow_new *vtkwin;

public:
    static myVtkInteractorContourWindow *New();
    double *startPosition;
    double *endPosition;
    vtkSmartPointer<vtkActor> lineActor;

    void setVtkWin(vtkwindow_new *w)
    {
        vtkwin = w;
        startPosition = new double[3];
        endPosition = new double[3];
    }

    virtual void OnMouseMove() { }

    virtual void OnLeftButtonDown()
    {

        vtkwin->removeActor(lineActor);
        startPosition[0] = this->Interactor->GetEventPosition()[0];
        startPosition[1] = this->Interactor->GetEventPosition()[1];
    }

    virtual void OnLeftButtonUp()
    {

        endPosition[0] = this->Interactor->GetEventPosition()[0];
        endPosition[1] = this->Interactor->GetEventPosition()[1];

        // Forward events
        vtkSmartPointer<vtkCoordinate> coordinate_start = vtkSmartPointer<vtkCoordinate>::New();
        coordinate_start->SetCoordinateSystemToDisplay();
        coordinate_start->SetValue(this->startPosition[0], this->startPosition[1], 0);

        double *world_start = coordinate_start->GetComputedWorldValue(
                this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

        vtkSmartPointer<vtkCoordinate> coordinate_end = vtkSmartPointer<vtkCoordinate>::New();
        coordinate_end->SetCoordinateSystemToDisplay();
        coordinate_end->SetValue(this->endPosition[0], this->endPosition[1], 0);

        double *world_end = coordinate_end->GetComputedWorldValue(
                this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

        vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();

        double deltaX = abs(world_end[0] - world_start[0]);
        double deltaY = abs(world_end[1] - world_start[1]);
        int resolution;

        double lineLenght = qSqrt(deltaX * deltaX * +deltaY * deltaY);
        if (deltaX > deltaY)
            resolution = (int)deltaX;
        else
            resolution = (int)deltaY;
        // int resolution=(int) lineLenght;

        lineSource->SetResolution(resolution);
        lineSource->SetPoint1(world_start);
        lineSource->SetPoint2(world_end);
        lineSource->Update();

        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        //        points=lineSource->GetPoints();

        vtkPolyData *polydata = lineSource->GetOutput();
        // Write all of the coordinates of the points in the vtkPolyData to the console.
        for (vtkIdType i = 0; i < polydata->GetNumberOfPoints(); i++) {
            double p[3];
            polydata->GetPoint(i, p);
            // This is identical to:
            // polydata->GetPoints()->GetPoint(i,p);
            std::cout << "Point " << i << " : (" << p[0] << " " << p[1] << " " << p[2] << ")"
                      << std::endl;
        }

        //  double *point=vtkwin->lineSource->GetPoint1();
        //  qDebug()<<"point: "<<point[0]<<point[1];

        // vtkIdType num= points->GetNumberOfPoints();

        /* //LINE WIDGET
         vtkSmartPointer<vtkLineWidget2> lineWidget =
         vtkSmartPointer<vtkLineWidget2>::New();

         //lineWidget->SetInteractor(vtkwin->ui->qVTK1->GetRenderWindow()->GetInteractor());//QVTKOpenGLWindow::GetRenderWindow()
         is deprecated, use renderWindow() instead. lineWidget->SetInteractor(this->Interactor);



         //lineWidget->CreateDefaultRepresentation();


         vtkSmartPointer<vtkLineRepresentation> lineRepresentation
         =vtkSmartPointer<vtkLineRepresentation>::New();
         lineRepresentation->SetPoint1WorldPosition(world_start);
         lineRepresentation->SetPoint2WorldPosition(world_end);
         lineRepresentation->SetLineColor(102, 0,102);


         lineWidget->SetRepresentation(lineRepresentation);

         qDebug()<<"lineWidget world 0"<<static_cast <vtkLineRepresentation
         *>(lineWidget->GetRepresentation())->GetPoint1WorldPosition()[0]; qDebug()<<"lineWidget
         world 1"<<static_cast <vtkLineRepresentation
         *>(lineWidget->GetRepresentation())->GetPoint1WorldPosition()[1];


         qDebug()<<"world_start 0"<<world_start[0];
         qDebug()<<"world_start 1"<<world_start[1];




         vtkwin->ui->qVTK1->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->Render();//QVTKOpenGLWindow::GetRenderWindow()
         is deprecated, use renderWindow() instead. lineWidget->On();
         */

        //        // Get the actual box coordinates of the line
        //             vtkSmartPointer<vtkPolyData> polydata =
        //                 vtkSmartPointer<vtkPolyData>::New();
        //             static_cast<vtkLineRepresentation*>(lineWidget->GetRepresentation())->GetPolyData
        //             (polydata);

        //             //Visualize
        //             vtkSmartPointer<vtkPolyDataMapper> mapper =
        //                            vtkSmartPointer<vtkPolyDataMapper>::New();
        //                    //mapper->SetInputConnection(lineWidget->GetOutputPort());
        //             mapper->SetInput(polydata);

        //                    vtkSmartPointer<vtkActor> actor =
        //                            vtkSmartPointer<vtkActor>::New();
        //                    actor->SetMapper(mapper);
        //                    actor->GetProperty()->SetLineWidth(2);
        //                    actor->GetProperty()->SetColor(51, 0,102);
        //                    vtkwin->addActor(actor);

        // Visualize
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(lineSource->GetOutputPort());

        lineActor = vtkSmartPointer<vtkActor>::New();
        lineActor->SetMapper(mapper);
        lineActor->GetProperty()->SetLineWidth(1);
        lineActor->GetProperty()->SetColor(102, 0, 102);
        vtkwin->addActor(lineActor);
        vtkwin->ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer()->Render();
    }

    virtual void OnChar() { }

    virtual void PrintSelf(std::ostream &os, vtkIndent indent) { }
    virtual void PrintHeader(ostream &os, vtkIndent indent) { }
    virtual void PrintTrailer(std::ostream &os, vtkIndent indent) { }
    virtual void CollectRevisions(std::ostream &os) { }
};
vtkStandardNewMacro(myVtkInteractorContourWindow);

class myVtkInteractorStyleImage : public vtkInteractorStyleImage
{
private:
    vtkwindow_new *vtkwin;
    bool isSlice = false;

public:
    static myVtkInteractorStyleImage *New();

    void setVtkWin(vtkwindow_new *w) { vtkwin = w; }

    void setIsSlice() { isSlice = true; }

    virtual void OnMouseMove()
    {
        vtkRenderWindowInteractor *rwi = this->Interactor;
        if (this->CurrentImageProperty) {
            vtkImageProperty *property = this->CurrentImageProperty;

            if (!vtkwin->image_init_window_level.contains(property)) {
                vtkwin->image_init_window_level.insert(property, property->GetColorWindow());
            }

            if (!vtkwin->image_init_color_level.contains(property)) {

                vtkwin->image_init_color_level.insert(property, property->GetColorLevel());
            }
        }
        // Forward events
        vtkSmartPointer<vtkCoordinate> coordinate = vtkSmartPointer<vtkCoordinate>::New();
        coordinate->SetCoordinateSystemToDisplay();
        coordinate->SetValue(this->GetInteractor()->GetEventPosition()[0],
                             this->GetInteractor()->GetEventPosition()[1], 0);

        double *world_coord = coordinate->GetComputedWorldValue(
                this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

        double *sky_coord = new double[2];
        double *sky_coord_gal = new double[2];
        double *sky_coord_fk5 = new double[2];

        QString statusBarText = "";
        vtkSmartPointer<vtkFitsReader> fits;
        if (!isSlice) {
            fits = vtkwin->getLayerListImages().at(0)->getFits();

            if (vtkwin->ui->listWidget->selectionModel()->selectedRows().count() != 0
                && vtkwin->getLayerListImages()
                                .at(vtkwin->ui->listWidget->selectionModel()
                                            ->selectedRows()
                                            .at(0)
                                            .row())
                                ->getType()
                        == 0) {
                fits = vtkwin->getLayerListImages()
                               .at(vtkwin->ui->listWidget->selectionModel()
                                           ->selectedRows()
                                           .at(0)
                                           .row())
                               ->getFits();
            }
        } else {
            fits = vtkwin->getFitsImage();
        }

        float *pixel;
        if (!isSlice)
            pixel = static_cast<float *>(
                    fits->GetOutput()->GetScalarPointer(world_coord[0], world_coord[1], 0));
        else
            pixel = static_cast<float *>(fits->GetOutput()->GetScalarPointer(
                    world_coord[0], world_coord[1], vtkwin->viewer->GetSlice()));

        statusBarText = "<value> ";
        if (pixel != NULL)
            statusBarText += QString::number(pixel[0]);
        else
            statusBarText += "NaN";

        statusBarText += " <image> X: " + QString::number(world_coord[0])
                + " Y: " + QString::number(world_coord[1]);

        // WCS_GALACTIC = 3
        AstroUtils().xy2sky(vtkwin->filenameWithPath, world_coord[0], world_coord[1], sky_coord_gal,
                            3);

        statusBarText += " <galactic> GLON: " + QString::number(sky_coord_gal[0])
                + " GLAT: " + QString::number(sky_coord_gal[1]);

        // WCS_J2000 = 1
        AstroUtils().xy2sky(vtkwin->filenameWithPath, world_coord[0], world_coord[1], sky_coord_fk5,
                            1);

        statusBarText += " <fk5> RA: " + QString::number(sky_coord_fk5[0])
                + " DEC: " + QString::number(sky_coord_fk5[1]);

        AstroUtils().xy2sky(vtkwin->filenameWithPath, world_coord[0], world_coord[1], sky_coord);
        statusBarText += " <ecliptic> RA: " + QString::number(sky_coord[0])
                + " DEC: " + QString::number(sky_coord[1]);

        vtkwin->ui->statusbar->showMessage(statusBarText);

        /*

        vtkSmartPointer<vtkImageActorPointPlacer> pointPlacer =
        vtkSmartPointer<vtkImageActorPointPlacer>::New(); if (!isSlice)
            pointPlacer->SetImageActor(vtkwin->imageViewer->GetImageActor());
        else
            pointPlacer->SetImageActor(vtkwin->viewer->GetImageActor());


        QString statusBarText="";

        if(pointPlacer->ValidateWorldPosition(world_coord)==1)
        {

            float* pixel;
            if(!isSlice)
                pixel=static_cast<
        float*>(vtkwin->getFitsImage()->GetOutput()->GetScalarPointer(world_coord[0],world_coord[1],0));
            else
                pixel=static_cast<
        float*>(vtkwin->getFitsImage()->GetOutput()->GetScalarPointer(world_coord[0],world_coord[1],vtkwin->viewer->GetSlice()));


            statusBarText = "<value> "+QString::number(pixel[0]);
            statusBarText+= " <image> X: "+QString::number(world_coord[0])+" Y:
        "+QString::number(world_coord[1]);

            //WCS_GALACTIC = 3
            //
        AstroUtils().xy2sky(vtkwin->getFilenameWithPath(),world_coord[0],world_coord[1],sky_coord_gal,3);
            AstroUtils().xy2sky(vtkwin->filenameWithPath,world_coord[0],world_coord[1],sky_coord_gal,3);


            statusBarText+=" <galactic> GLON: "+QString::number(sky_coord_gal[0])+" GLAT:
        "+QString::number(sky_coord_gal[1]);

            //WCS_J2000 = 1
            AstroUtils().xy2sky(vtkwin->filenameWithPath,world_coord[0],world_coord[1],sky_coord_fk5,1);
            //
        AstroUtils().xy2sky(vtkwin->getFilenameWithPath(),world_coord[0],world_coord[1],sky_coord_fk5,1);

            statusBarText+=" <fk5> RA: "+QString::number(sky_coord_fk5[0])+" DEC:
        "+QString::number(sky_coord_fk5[1]);


            //
        AstroUtils().xy2sky(vtkwin->getFilenameWithPath(),world_coord[0],world_coord[1],sky_coord);
            AstroUtils().xy2sky(vtkwin->filenameWithPath,world_coord[0],world_coord[1],sky_coord);
            statusBarText+=" <ecliptic> RA: "+QString::number(sky_coord[0])+" DEC:
        "+QString::number(sky_coord[1]);

        }
        else
        {
            statusBarText="";
        }

        vtkwin->ui->statusbar->showMessage(statusBarText);
 */
        vtkInteractorStyleImage::OnMouseMove();
    }

    virtual void OnChar()
    {
        vtkRenderWindowInteractor *rwi = this->Interactor;

        switch (rwi->GetKeyCode()) {
        case 'r':
        case 'R':
            // Allow either shift/ctrl to trigger the usual 'r' binding
            // otherwise trigger reset window level event
            if (rwi->GetShiftKey() || rwi->GetControlKey()) {
                this->Superclass::OnChar();
            } else if (this->HandleObservers
                       && this->HasObserver(vtkCommand::ResetWindowLevelEvent)) {
                this->InvokeEvent(vtkCommand::ResetWindowLevelEvent, this);
            } else if (this->CurrentImageProperty) {
                vtkImageProperty *property = this->CurrentImageProperty;
                qDebug() << property;

                property->SetColorWindow(vtkwin->image_init_window_level.value(property));
                property->SetColorLevel(vtkwin->image_init_color_level.value(property));

                qDebug() << vtkwin->image_init_window_level.value(property);

                // property->SetColorWindow(this->WindowLevelInitial[0]);
                // property->SetColorLevel(this->WindowLevelInitial[1]);
                this->Interactor->Render();
            }
            break;
        }
    }

    virtual void PrintSelf(std::ostream &os, vtkIndent indent) { }
    virtual void PrintHeader(ostream &os, vtkIndent indent) { }
    virtual void PrintTrailer(std::ostream &os, vtkIndent indent) { }
    virtual void CollectRevisions(std::ostream &os) { }
};
vtkStandardNewMacro(myVtkInteractorStyleImage);

class SkyRegionSelector : public vtkInteractorStyleRubberBand2D
{
private:
    vtkwindow_new *vtkwin;
    bool is3D, isFilament, is3dSelections, isBubble;

public:
    static SkyRegionSelector *New();
    vtkTypeMacro(SkyRegionSelector, vtkInteractorStyleRubberBand2D);

    SkyRegionSelector()
    {
        is3D = false;
        isFilament = false;
        is3dSelections = false;
        isBubble = false;
    }

    void setIsFilament() { isFilament = true; }

    void setIsBubble() { isBubble = true; }

    void setIs3dSelections() { is3dSelections = true; }

    void setIs3D() { is3D = true; }

    void setVtkWin(vtkwindow_new *w) { vtkwin = w; }

    vtkwindow_new *getVtkWin() { return vtkwin; }

    virtual void OnLeftButtonUp()
    {

        // Forward events
        vtkSmartPointer<vtkCoordinate> coordinate = vtkSmartPointer<vtkCoordinate>::New();
        coordinate->SetCoordinateSystemToDisplay();
        coordinate->SetValue(this->StartPosition[0], this->StartPosition[1], 0);

        double *world_start = coordinate->GetComputedWorldValue(
                this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

        vtkSmartPointer<vtkCoordinate> coordinate_end = vtkSmartPointer<vtkCoordinate>::New();
        coordinate_end->SetCoordinateSystemToDisplay();
        coordinate_end->SetValue(this->EndPosition[0], this->EndPosition[1], 0);

        double *world_end = coordinate_end->GetComputedWorldValue(
                this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

        double *coor_start = new double[2];
        double *coor_end = new double[2];

        AstroUtils().xy2sky(vtkwin->filenameWithPath, world_start[0], world_start[1], coor_start,
                            3);
        AstroUtils().xy2sky(vtkwin->filenameWithPath, world_end[0], world_end[1], coor_end, 3);

        vtkInteractorStyleRubberBand2D::OnLeftButtonUp();
        vtkwin->ui->rectangularSelectionCS->setStyleSheet("");
        vtkwin->ui->fil_rectPushButton->setStyleSheet("");
        vtkwin->ui->tdRectPushButton->setStyleSheet("");
        vtkwin->ui->bubblePushButton->setStyleSheet("");

        if (!(vtkwin->isDatacube)) {

            VLKBSimpleQueryComposer *skyregionquery = new VLKBSimpleQueryComposer(vtkwin);

            if (isFilament) {
                skyregionquery->setIsFilament();
            } else if (is3dSelections) {
                skyregionquery->setIs3dSelections();
            } else if (isBubble) {
                skyregionquery->setIsBubble();
            }

            skyregionquery->setLongitude(coor_start[0], coor_end[0]);
            skyregionquery->setLatitude(coor_start[1], coor_end[1]);
            skyregionquery->show();

            vtkwin->setVtkInteractorStyleImage();
        } else {

            dbquery *queryWindow = new dbquery();
            QString glong, glat;

            glong = QString::number(coor_end[0] + (coor_start[0] - coor_end[0]) / 2);
            glat = QString::number(coor_end[1] + (coor_start[1] - coor_end[1]) / 2);

            double width = coor_start[0] - coor_end[0];
            double lenght = coor_start[1] - coor_end[1];
            vtkRect<double> *rect =
                    new vtkRect<double>(coor_start[0], coor_start[1], width, lenght);

            queryWindow->setCoordinate(glong, glat);
            queryWindow->show();
        }
    }

    virtual void PrintSelf(std::ostream &os, vtkIndent indent) { }
    virtual void PrintHeader(ostream &os, vtkIndent indent) { }
    virtual void PrintTrailer(std::ostream &os, vtkIndent indent) { }
    virtual void CollectRevisions(std::ostream &os) { }
};
vtkStandardNewMacro(SkyRegionSelector);

class InteractorStyleExtractSources : public vtkInteractorStyleRubberBand2D
{
public:
    static InteractorStyleExtractSources *New();
    vtkTypeMacro(InteractorStyleExtractSources, vtkInteractorStyleRubberBand2D);

    std::function<void(int rect[4])> Callback;

    void OnLeftButtonUp() override
    {
        this->Superclass::OnLeftButtonUp();

        if (this->Callback) {
            int rect[4] = { this->StartPosition[0], this->StartPosition[1], this->EndPosition[0],
                            this->EndPosition[1] };
            Callback(rect);
        }
    }

protected:
    InteractorStyleExtractSources() = default;

private:
    InteractorStyleExtractSources(const InteractorStyleExtractSources &) = delete;
    void operator=(const InteractorStyleExtractSources &) = delete;
};
vtkStandardNewMacro(InteractorStyleExtractSources);

class InteractorStyleEditSource : public vtkInteractorStyleTrackballActor
{
public:
    static InteractorStyleEditSource *New();
    vtkTypeMacro(InteractorStyleEditSource, vtkInteractorStyleTrackballActor);

    void OnMouseMove() override
    {
        if (!this->Interactor || !moving) {
            return;
        }

        int x = this->Interactor->GetEventPosition()[0];
        int y = this->Interactor->GetEventPosition()[1];

        /*
        this->FindPokedRenderer(x, y);
        if (!this->CurrentRenderer) {
            return;
        }
        */
        this->Coordinate = vtkSmartPointer<vtkCoordinate>::New();
        this->Coordinate->SetCoordinateSystemToDisplay();
        this->Coordinate->SetValue(x, y);
        double *coords = this->Coordinate->GetComputedWorldValue(this->CurrentRenderer);

        qDebug() << Q_FUNC_INFO << "POINT" << PointId << " - NEW COORDS" << coords[0] << coords[1];

        this->Points->SetPoint(PointId, coords);
        this->Points->Modified();

        this->Interactor->Render();
    }

    void OnLeftButtonDown() override
    {
        this->moving = false;

        if (!this->Interactor) {
            return;
        }

        int x = this->Interactor->GetEventPosition()[0];
        int y = this->Interactor->GetEventPosition()[1];
        qDebug() << Q_FUNC_INFO << "Display coords" << x << y;

        this->FindPokedRenderer(x, y);
        // this->FindPickedActor(x, y);
        if (!this->CurrentRenderer /* || !this->InteractionProp*/) {
            return;
        }

        // auto actor = vtkLODActor::SafeDownCast(this->InteractionProp);
        //  auto actor = this->Actor;
        this->ActorPicker->PickProp(x, y, this->CurrentRenderer);
        auto actor = vtkLODActor::SafeDownCast(this->ActorPicker->GetViewProp());
        if (!actor || (actor != this->ActorFilter && actor != this->Actor)) {
            return;
        }
        qDebug() << Q_FUNC_INFO << "Actor Picked" << actor;

        if (this->PointPicker->Pick(x, y, 0, this->CurrentRenderer) == 0) {
            return;
        }
        PointId = this->PointPicker->GetPointId();
        qDebug() << Q_FUNC_INFO << "Point Picked" << PointId;
        this->moving = true;

        /*
        auto mapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
        if (!mapper) {
            return;
        }
        qDebug() << Q_FUNC_INFO << "Mapper OK";

        auto polydata = mapper->GetInput();
        Points = polydata->GetPoints();
        qDebug() << Q_FUNC_INFO << "Points OK" << Points->GetNumberOfPoints();
        std::cout << std::endl;
        this->Interactor->Render();
        */
    }

    void OnLeftButtonUp() override
    {
        qDebug() << Q_FUNC_INFO;
        this->moving = false;

        if (!this->Interactor) {
            return;
        }

        /*
        if (this->PointId >= 0) {
            // this->Points = nullptr;
            this->PointId = -1;
            this->Interactor->Render();
        }
        */
    }

    void OnMiddleButtonDown() override { }
    void OnMiddleButtonUp() override { }
    void OnRightButtonDown() override { }
    void OnRightButtonUp() override { }

    void SetSource(vtkSmartPointer<vtkPoints> points, vtkSmartPointer<vtkLODActor> actor)
    {
        this->Points = points;
        this->Actor = actor;

        vtkNew<vtkPolyData> polydata;
        polydata->SetPoints(points);

        vtkNew<vtkVertexGlyphFilter> filter;
        filter->SetInputData(polydata);
        filter->Update();

        vtkNew<vtkPolyDataMapper> mapperFilter;
        mapperFilter->SetInputConnection(filter->GetOutputPort());

        ActorFilter = vtkSmartPointer<vtkLODActor>::New();
        ActorFilter->SetMapper(mapperFilter);
        ActorFilter->GetProperty()->SetPointSize(5);
        ActorFilter->GetProperty()->SetColor(0, 1, 0);

        RenderWindow->GetRenderers()->GetFirstRenderer()->AddActor(ActorFilter);
        RenderWindow->Render();
    }

    void SetRenderWindow(vtkRenderWindow *win) { this->RenderWindow = win; }

protected:
    InteractorStyleEditSource()
    {
        this->moving = false;

        this->PointPicker = vtkSmartPointer<vtkPointPicker>::New();
        this->PointPicker->UseCellsOn();
        this->ActorPicker = vtkSmartPointer<vtkPropPicker>::New();

        this->Coordinate = vtkSmartPointer<vtkCoordinate>::New();
        this->Coordinate->SetCoordinateSystemToDisplay();

        this->Points = nullptr;
    }

    ~InteractorStyleEditSource()
    {
        RenderWindow->GetRenderers()->GetFirstRenderer()->RemoveActor(ActorFilter);
        RenderWindow->Render();
    }

private:
    InteractorStyleEditSource(const InteractorStyleEditSource &) = delete;
    void operator=(const InteractorStyleEditSource &) = delete;

    bool moving;
    vtkSmartPointer<vtkCoordinate> Coordinate;
    vtkSmartPointer<vtkPointPicker> PointPicker;
    vtkSmartPointer<vtkPropPicker> ActorPicker;
    vtkSmartPointer<vtkPoints> Points;
    vtkIdType PointId;
    vtkSmartPointer<vtkLODActor> Actor;
    vtkSmartPointer<vtkLODActor> ActorFilter;

    vtkRenderWindow *RenderWindow;
};
vtkStandardNewMacro(InteractorStyleEditSource);

vtkwindow_new::~vtkwindow_new()
{
    delete ui;
}

vtkwindow_new::vtkwindow_new(QWidget *parent, VisPoint *vis)
    : QMainWindow(parent), ui(new Ui::vtkwindow_new)
{

    ui->setupUi(this);

    stringDictWidget = &Singleton<VialacteaStringDictWidget>::Instance();

    ui->actionSave_session->setEnabled(false);
    ui->menuMoment->menuAction()->setVisible(false);
    ui->ElementListWidget->hide();
    ui->tableWidget->hide();
    ui->listWidget->hide();
    ui->toolsGroupBox->hide();
    ui->datacubeGroupBox->hide();
    ui->tdGroupBox->hide();
    ui->filamentsGroupBox->hide();
    ui->bubbleGroupBox->hide();
    ui->compactSourcesGroupBox->hide();
    ui->PVPlotPushButton->hide();
    ui->PVPlot_radioButton->hide();
    ui->label_survey->hide();
    ui->lineEdit_survey->hide();
    ui->label_species->hide();
    ui->lineEdit_species->hide();
    ui->label_transition->hide();
    ui->lineEdit_transition->hide();
    ui->contourGroupBox->hide();
    ui->ThresholdGroupBox->hide();
    ui->cuttingPlaneGroupBox->hide();
    ui->spinBox_channels->hide();
    ui->label_channels->hide();
    ui->spinBox_cuttingPlane->hide();
    ui->label_channels->hide();
    ui->selectionGroupBox->hide();
    ui->isocontourVtkWin->hide();
    ui->valueGroupBox->hide();
    ui->actionTools->setVisible(true);

    // ui->glyphGroupBox->hide();
    // ui->glyphScalarComboBox->hide();

    fitsViewer = false;
    /*m_Ren1 = vtkRenderer::New();
    //renwin = vtkRenderWindow::New();
    vtkNew<vtkGenericOpenGLRenderWindow> rw;
    renwin = rw;
    renwin->AddRenderer(m_Ren1);
    renwin->SetInteractor(ui->qVTK1->interactor());
    ui->qVTK1->setRenderWindow(renwin);*/

    auto renWin = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    renwin = renWin;
    ui->qVTK1->setRenderWindow(renwin);

    auto interactor = renwin->GetInteractor();

    auto ren = vtkSmartPointer<vtkRenderer>::New();
    m_Ren1 = ren;
    // m_Ren1->SetBackground(0.21,0.23,0.25);
    renwin->AddRenderer(m_Ren1);

    interactor->Render();

    m_Ren1->GlobalWarningDisplayOff();
    loadObservedObject(vis);

    // add lut on main window FV
    for (unsigned int i = 0; i < vis->getOrigin()->getNumberOfColumns(); i++) {
        QString field = QString(vis->getOrigin()->getColName(i).c_str());
        ui->scalarComboBox->addItem(field);
        ui->glyphScalarComboBox->addItem(field);
    }
    // end add lut on main window FV

    vtkAxes = vtkSmartPointer<vtkAxesActor>::New();
    vtkAxesWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    vtkAxesWidget->SetInteractor(ui->qVTK1->renderWindow()->GetInteractor());

    vtkAxesWidget->SetOrientationMarker(vtkAxes);

    vtkAxesWidget->SetOutlineColor(0.9300, 0.5700, 0.1300);
    vtkAxesWidget->SetViewport(0.0, 0.0, 0.2, 0.2);
    vtkAxesWidget->SetEnabled(1);
    vtkAxesWidget->InteractiveOff();

    update();

    pp->getRenderer()->GetActiveCamera()->GetPosition(cam_init_pos);
    pp->getRenderer()->GetActiveCamera()->GetFocalPoint(cam_init_foc);

    scaleActivate = true;
    isDatacube = false;
}

vtkwindow_new::vtkwindow_new(QWidget *parent, vtkSmartPointer<vtkFitsReader> vis, int b,
                             vtkwindow_new *p)
    : QMainWindow(parent), ui(new Ui::vtkwindow_new)
{
    QSettings settings(QDir::homePath()
                               .append(QDir::separator())
                               .append("VisIVODesktopTemp")
                               .append("/setting.ini"),
                       QSettings::NativeFormat);
    vlkbUrl = settings.value("vlkburl", "").toString();
    stringDictWidget = &Singleton<VialacteaStringDictWidget>::Instance();

    myfits = vis;
    filenameWithPath = vis->GetFileName();
    myParentVtkWindow = p;
    vtkwintype = b;

    imageObject = new vtkfitstoolwidgetobject(0);
    imageObject->setName(QString::fromUtf8(myfits->GetFileName().c_str()));
    imageObject->setFitsReader(myfits);
    // setto specie e transition
    imageObject->setSpecies(vis->getSpecies());
    imageObject->setSurvey(vis->getSurvey().replace("%20", " "));
    imageObject->setTransition(vis->getTransition());

    selected_scale = "Log";

    switch (b) {
    case 0: {
        ui->setupUi(this);

        this->setWindowTitle(myfits->GetFileName().c_str());
        this->isDatacube = false;

        ui->menuCamera->menuAction()->setVisible(false);
        ui->menuMoment->menuAction()->setVisible(false);
        ui->cameraControlgroupBox->hide();
        ui->selectionGroupBox->hide();
        ui->ThresholdGroupBox->hide();
        ui->valueGroupBox->hide();
        ui->cuttingPlaneGroupBox->hide();
        ui->spinBox_cuttingPlane->hide();
        ui->spinBox_channels->hide();
        ui->label_channels->hide();
        ui->label_survey->hide();
        ui->label_species->hide();
        ui->label_transition->hide();
        ui->lineEdit_survey->hide();
        ui->lineEdit_species->hide();
        ui->lineEdit_transition->hide();
        ui->contourGroupBox->hide();
        ui->PVPlotPushButton->hide();
        ui->PVPlot_radioButton->hide();
        ui->datacubeGroupBox->hide();
        ui->lut3dGroupBox->hide();
        ui->glyphGroupBox->hide();
        ui->filterGroupBox->hide();
        ui->bubbleGroupBox->hide();
        ui->isocontourVtkWin->hide();

        ui->ElementListWidget->installEventFilter(this);
        ui->ElementListWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->listWidget->setDragDropMode(QAbstractItemView::InternalMove);
        connect(ui->listWidget->model(), SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)),
                this, SLOT(movedLayersRow(QModelIndex, int, int, QModelIndex, int)));

        auto renWin = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        renwin = renWin;
        ui->qVTK1->setRenderWindow(renwin);

        auto interactor = renwin->GetInteractor();

        auto m_Ren0 = vtkSmartPointer<vtkRenderer>::New();
        m_Ren1 = m_Ren0;
        m_Ren1->SetBackground(0.21, 0.23, 0.25);
        renwin->AddRenderer(m_Ren1);

        interactor->Render();
        ui->qVTK1->setDefaultCursor(Qt::ArrowCursor);

        m_Ren1->GlobalWarningDisplayOff();
        m_Ren1->SetBackground(0.21, 0.23, 0.25);

        QAction *select = new QAction("Select", this);
        select->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
        connect(select, SIGNAL(triggered()), this, SLOT(setSelectionFitsViewerInteractorStyle()));
        ui->menuWindow->addAction(select);

        QAction *extract = new QAction("Extract", this);
        extract->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
        connect(extract, &QAction::triggered, this, &vtkwindow_new::setVtkInteractorExtractSources);
        ui->menuWindow->addAction(extract);

        QAction *filter = new QAction("Filter", this);
        connect(filter, &QAction::triggered, this, &vtkwindow_new::openFilterDialog);
        ui->menuWindow->addAction(filter);

        QMenu *compact = ui->menuFile->addMenu("Add compact sources");
        QAction *local = new QAction("Local", this);
        local->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
        connect(local, SIGNAL(triggered()), this, SLOT(addLocalSources()));
        compact->addAction(local);

        QAction *localJson = new QAction("From JSON catalogue");
        connect(localJson, &QAction::triggered, this, [this]() {
            QString fn = QFileDialog::getOpenFileName(this, "Open JSON Catalogue", QDir::homePath(),
                                                      "JSON Catalogue (*.json)");
            if (!fn.isEmpty()) {
                addSourcesFromJson(fn);
            }
        });
        compact->addAction(localJson);

        QAction *ds9 = new QAction("From DS9 Region");
        connect(ds9, &QAction::triggered, this, &vtkwindow_new::loadDS9RegionFile);
        compact->addAction(ds9);

        QAction *remote = new QAction("Remote", this);
        remote->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
        connect(remote, SIGNAL(triggered()), this, SLOT(setSkyRegionSelectorInteractorStyle()));
        compact->addAction(remote);

        QAction *normal_selector = new QAction("Normal", this);
        normal_selector->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
        connect(normal_selector, SIGNAL(triggered()), this, SLOT(setVtkInteractorStyleImage()));
        compact->addAction(normal_selector);

        QAction *selector_3D = new QAction("3D", this);
        selector_3D->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_3));
        connect(selector_3D, SIGNAL(triggered()), this, SLOT(on_tdRectPushButton_clicked()));
        compact->addAction(selector_3D);

        setVtkInteractorStyleImage();

        // double* range = vis->GetOutput()->GetScalarRange();
        // qDebug()<<" r: "<<range[0]<<" .. "<<range[1];

        vtkSmartPointer<vtkImageShiftScale> resultScale =
                vtkSmartPointer<vtkImageShiftScale>::New();
        resultScale->SetOutputScalarTypeToUnsignedChar();
        resultScale->SetInputData(vis->GetOutput());

        resultScale->Update();

        vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
        float min = myfits->GetMin();
        if (min < 0)
            min = 0;
        lut->SetTableRange(min, myfits->GetMax());
        lut->SetScaleToLog10();

        SelectLookTable("Gray", lut);
        imageObject->setLutScale("Log");
        imageObject->setLutType("Gray");

        vtkSmartPointer<vtkImageMapToColors> colors = vtkSmartPointer<vtkImageMapToColors>::New();
        colors->SetInputData(vis->GetOutput());
        colors->SetLookupTable(lut);
        colors->Update();

        vtkSmartPointer<vtkImageSliceMapper> imageSliceMapperBase =
                vtkSmartPointer<vtkImageSliceMapper>::New();

        imageSliceMapperBase->SetInputData(colors->GetOutput());
        //  imageSliceMapperBase->SetInputData(vis->GetOutput());

        vtkSmartPointer<vtkImageSlice> imageSliceBase = vtkSmartPointer<vtkImageSlice>::New();
        imageSliceBase->SetMapper(imageSliceMapperBase);
        imageSliceBase->GetProperty()->SetInterpolationTypeToNearest();

        imageSliceBase->GetProperty()->SetLayerNumber(0);

        // Stack
        imageStack = vtkSmartPointer<vtkImageStack>::New();
        imageStack->AddImage(imageSliceBase);

        vtkSmartPointer<vtkLegendScaleActor> legendScaleActorImage =
                vtkSmartPointer<vtkLegendScaleActor>::New();
        legendScaleActorImage->LegendVisibilityOff();
        legendScaleActorImage->setFitsFile(myfits);

        m_Ren1->AddActor(legendScaleActorImage);
        m_Ren1->AddViewProp(imageStack);
        m_Ren1->ResetCamera();

        addLayer(imageObject);

        ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);

        createInfoWindow();
        showMaximized();
        activateWindow();

        break;
    }
    case 1: {

        vis->CalculateRMS();
        isDatacube = true;
        this->max = vis->GetMax();
        this->min = vis->GetMin();
        this->naxis3 = vis->GetNaxes(2);

        ui->setupUi(this);
        ui->menuTools->menuAction()->setVisible(false);
        ui->actionSave_session->setEnabled(false);
        ui->cameraControlgroupBox->hide();
        ui->splitter->hide();
        ui->ElementListWidget->hide();
        ui->tableWidget->hide();
        ui->listWidget->hide();
        ui->compactSourcesGroupBox->hide();
        ui->datacubeGroupBox->hide();
        ;
        ui->toolsGroupBox->hide();
        ui->filamentsGroupBox->hide();
        ui->bubbleGroupBox->hide();
        ui->tdGroupBox->hide();
        ui->label_survey->hide();
        ui->label_species->hide();
        ui->label_transition->hide();
        ui->lineEdit_survey->hide();
        ui->lineEdit_species->hide();
        ui->lineEdit_transition->hide();
        ui->spinBox_channels->hide();
        ui->label_channels->hide();
        ui->PVPlotPushButton->hide();
        ui->selectionGroupBox->hide();
        ui->PVPlot_radioButton->hide();
        ui->lut3dGroupBox->hide();
        ui->glyphGroupBox->hide();
        ui->filterGroupBox->hide();

        ui->minLineEdit->setText(QString::number(min, 'f', 4));
        ui->maxLineEdit->setText(QString::number(max, 'f', 4));
        ui->RmsLineEdit->setText(QString::number(vis->GetRMS(), 'f', 4));
        ui->thresholdValueLineEdit->setText(QString::number(3 * vis->GetRMS(), 'f', 4));
        ui->lowerBoundLineEdit->setText(QString::number(3 * vis->GetRMS(), 'f', 4));
        ui->upperBoundLineEdit->setText(QString::number(max, 'f', 4));

        // Start qVTK1 (left)
        auto renWin = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        renwin = renWin;
        ui->qVTK1->setDefaultCursor(Qt::ArrowCursor);
        ui->qVTK1->setRenderWindow(renwin);

        auto ren = vtkSmartPointer<vtkRenderer>::New();
        m_Ren1 = ren;
        m_Ren1->SetBackground(0.21, 0.23, 0.25);
        m_Ren1->GlobalWarningDisplayOff();
        renwin->AddRenderer(m_Ren1);

        // outline
        vtkOutlineFilter *outlineF = vtkOutlineFilter::New();
        outlineF->SetInputData(vis->GetOutput());
        vtkPolyDataMapper *outlineM = vtkPolyDataMapper::New();
        outlineM->SetInputConnection(outlineF->GetOutputPort());
        outlineM->ScalarVisibilityOff();
        vtkActor *outlineA = vtkActor::New();
        outlineA->SetMapper(outlineM);

        // isosurface
        shellE = vtkMarchingCubes::New();
        shellE->SetInputData(vis->GetOutput());
        shellE->ComputeNormalsOn();
        shellE->SetValue(0, 3 * vis->GetRMS());
        vtkPolyDataMapper *shellM = vtkPolyDataMapper::New();
        shellM->SetInputConnection(shellE->GetOutputPort());
        shellM->ScalarVisibilityOff();
        vtkActor *shellA = vtkActor::New();
        shellA->SetMapper(shellM);
        shellA->GetProperty()->SetColor(1.0, 0.5, 1.0);

        // slice
        vtkPlanes *sliceE = vtkPlanes::New();
        sliceE->SetBounds(vis->GetOutput()->GetBounds()[0], vis->GetOutput()->GetBounds()[1],
                          vis->GetOutput()->GetBounds()[2], vis->GetOutput()->GetBounds()[3], 0, 1);
        vtkSmartPointer<vtkFrustumSource> frustumSource = vtkSmartPointer<vtkFrustumSource>::New();
        frustumSource->ShowLinesOff();
        frustumSource->SetPlanes(sliceE);
        frustumSource->Update();
        vtkPolyData *frustum = frustumSource->GetOutput();
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputData(frustum);
        sliceA = vtkActor::New();
        sliceA->SetMapper(mapper);

        // axes and coords
        vtkAxes = vtkSmartPointer<vtkAxesActor>::New();
        vtkAxes->SetXAxisLabelText("X");
        vtkAxes->SetYAxisLabelText("Y");
        vtkAxes->SetZAxisLabelText("Z");
        vtkAxes->DragableOn();
        vtkSmartPointer<vtkTextProperty> tprop = vtkSmartPointer<vtkTextProperty>::New();
        tprop->SetFontSize(0);
        tprop->SetOpacity(0);
        vtkAxes->GetXAxisCaptionActor2D()->GetTextActor()->GetScaledTextProperty();
        vtkAxes->GetXAxisCaptionActor2D()->GetTextActor()->SetTextScaleMode(0);
        vtkAxes->GetYAxisCaptionActor2D()->GetTextActor()->GetScaledTextProperty();
        vtkAxes->GetYAxisCaptionActor2D()->GetTextActor()->SetTextScaleMode(0);
        vtkAxes->GetZAxisCaptionActor2D()->GetTextActor()->GetScaledTextProperty();
        vtkAxes->GetZAxisCaptionActor2D()->GetTextActor()->SetTextScaleMode(0);
        vtkAxesWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
        vtkAxesWidget->SetInteractor(ui->qVTK1->renderWindow()->GetInteractor());
        vtkAxesWidget->SetOrientationMarker(vtkAxes);
        vtkAxesWidget->SetOutlineColor(0.9300, 0.5700, 0.1300);
        vtkAxesWidget->SetViewport(0.0, 0.0, 0.2, 0.2);
        vtkAxesWidget->SetEnabled(1);
        vtkAxesWidget->InteractiveOff();
        vtkSmartPointer<vtkLegendScaleActor> legendScaleActor3d =
                vtkSmartPointer<vtkLegendScaleActor>::New();
        legendScaleActor3d->LegendVisibilityOff();
        legendScaleActor3d->setFitsFile(myfits);

        m_Ren1->GetActiveCamera()->GetPosition(cam_init_pos);
        m_Ren1->GetActiveCamera()->GetFocalPoint(cam_init_foc);

        // add actors to renderer
        m_Ren1->AddActor(outlineA);
        m_Ren1->AddActor(shellA);
        m_Ren1->AddActor(sliceA);
        m_Ren1->AddActor(legendScaleActor3d);

        // Start isocontourVtkWin (right)
        auto renWin2 = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        renwin2 = renWin2;
        // renwin2->SetNumberOfLayers(2);
        ui->isocontourVtkWin->setRenderWindow(renwin2);

        auto ren2 = vtkSmartPointer<vtkRenderer>::New();
        m_Ren2 = ren2;
        m_Ren2->SetBackground(0.21, 0.23, 0.25);
        m_Ren2->GlobalWarningDisplayOff();
        renwin2->AddRenderer(m_Ren2);

        vtkSmartPointer<vtkLookupTable> lutSlice = vtkSmartPointer<vtkLookupTable>::New();
        lutSlice->SetTableRange(myfits->GetRangeSlice(0)[0], myfits->GetRangeSlice(0)[1]);
        SelectLookTable("Gray", lutSlice);

        setVtkInteractorStyleImageContour();

        viewer = vtkSmartPointer<vtkResliceImageViewer>::New();
        viewer->SetInputData(vis->GetOutput());
        viewer->GetWindowLevel()->SetOutputFormatToRGB();
        viewer->GetWindowLevel()->SetLookupTable(lutSlice);
        viewer->GetImageActor()->InterpolateOff();
        viewer->SetRenderWindow(renwin2);
        viewer->SetRenderer(m_Ren2);
        viewer->GetRenderer()->ResetCamera();

        currentContourActor = vtkSmartPointer<vtkLODActor>::New();
        currentContourActorForMainWindow = vtkSmartPointer<vtkLODActor>::New();

        ui->cuttingPlane_Slider->setRange(1, vis->GetNaxes(2));
        ui->spinBox_cuttingPlane->setRange(1, vis->GetNaxes(2));

        vtkSmartPointer<vtkLegendScaleActor> legendScaleActorImage =
                vtkSmartPointer<vtkLegendScaleActor>::New();
        legendScaleActorImage->LegendVisibilityOff();
        legendScaleActorImage->setFitsFile(myfits);
        m_Ren2->AddActor(legendScaleActorImage);

        this->setWindowName("Datacube visualization");
        showMaximized();
        activateWindow();

        updateScene();
        break;
    }
    case 2: {
        qDebug() << "case 2";

        vis->CalculateRMS();

        isDatacube = true;
        vis->is3D = true;
        vis->GetOutput();
        ui->setupUi(this);
        this->setWindowName("Datacubes slices visualization");
        contourWin = new contour();

        m_Ren1 = vtkRenderer::New();
        m_Ren1->GlobalWarningDisplayOff();
        // renwin = vtkRenderWindow::New();
        vtkNew<vtkGenericOpenGLRenderWindow> rw;
        rw->AddRenderer(m_Ren1);
        ui->qVTK1->setRenderWindow(rw);
        ui->filamentsGroupBox->hide();
        ui->bubbleGroupBox->hide();

        ui->compactSourcesGroupBox->hide();
        ui->datacubeGroupBox->hide();
        ui->toolsGroupBox->hide();
        ui->tdGroupBox->hide();

        ui->splitter->hide();
        ui->ElementListWidget->hide();
        ui->tableWidget->hide();
        ui->listWidget->hide();
        ui->glyphGroupBox->hide();

        ui->cameraControlgroupBox->hide();
        ui->ThresholdGroupBox->hide();

        ui->cuttingPlaneGroupBox->hide();
        ui->spinBox_cuttingPlane->hide();
        ui->lineEdit_species->setText(species);
        ui->lineEdit_survey->setText(survey);
        ui->lineEdit_transition->setText(transition);
        ui->lineEdit_species->setEnabled(false);
        ui->lineEdit_survey->setEnabled(false);
        ui->lineEdit_transition->setEnabled(false);
        ui->selectionGroupBox->hide();
        ui->filterGroupBox->hide();

        naxis3 = vis->GetNaxes(2);

        fitsViewer = true;
        filenameWithPath = vis->GetFileName();

        // MODIFICHE
        // commentato: setVtkInteractorStyleImage();

        // A yellow-to-blue colormap defined by individually setting all values
        vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
        // lut->SetTableRange( 0, 255 );
        lut->SetScaleToLog10();

        SelectLookTable("Gray", lut);

        double *range = vis->GetOutput()->GetScalarRange();

        // The image viewers and writers are only happy with unsigned char
        // images.  This will convert the floats into that format.
        vtkSmartPointer<vtkImageShiftScale> resultScale =
                vtkSmartPointer<vtkImageShiftScale>::New();
        resultScale->SetOutputScalarTypeToUnsignedChar();
        resultScale->SetShift(0);
        resultScale->SetScale(range[1] - range[0]);
        resultScale->SetInputData(vis->GetOutput());
        // resultScale->SetInputConnection( vis->GetOutputPort() );

        resultScale->Update();

        // commentato qui, provo reslice
        imageViewer = vtkSmartPointer<vtkImageViewer2>::New();
        imageViewer->SetInputData(resultScale->GetOutput());

        // Set Color level and window
        imageViewer->SetColorLevel(0.5 * (range[1] + range[0]));
        imageViewer->SetColorWindow(range[1] - range[0]);

        imageViewer->SetupInteractor(ui->qVTK1->renderWindow()->GetInteractor());
        imageViewer->GetInteractorStyle()->AutoAdjustCameraClippingRangeOn();

        imageViewer->SetRenderer(m_Ren1);
        imageViewer->SetRenderWindow(rw);

        imageViewer->GetWindowLevel()->SetLookupTable(lut);

        viewer = vtkSmartPointer<vtkResliceImageViewer>::New();
        viewer->SetRenderer(ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer());
        viewer->SetRenderWindow(ui->qVTK1->renderWindow());
        viewer->SetupInteractor(ui->qVTK1->renderWindow()->GetInteractor());
        viewer->SetInputData(vis->GetOutput());
        viewer->SetSlice(1);

        double *pos = m_Ren1->GetActiveCamera()->GetPosition();
        cam_init_pos[0] = pos[0];
        cam_init_pos[1] = pos[1];
        cam_init_pos[2] = pos[2];

        vtkImageActor *imageActor = viewer->GetImageActor();
        m_Ren1->AddActor(imageActor);
        m_Ren1->SetBackground(0.21, 0.23, 0.25);

        update();

        break;
    }
    default:
        break;
    }
}

void vtkwindow_new::setWindowName(QString name)
{
    this->setWindowTitle(name);
}

QString vtkwindow_new::getWindowName()
{
    return this->windowTitle();
}

int vtkwindow_new::getThresholdValue()
{
    return ui->horizontalSlider_threshold->value();
}

void vtkwindow_new::setThresholdValue(int sliderValue)
{
    float value =
            (sliderValue * (myfits->GetMax() - 3 * myfits->GetRMS()) / 100) + 3 * myfits->GetRMS();
    ui->horizontalSlider_threshold->setValue(sliderValue);
    ui->thresholdValueLineEdit->setText(QString::number(value, 'f', 4));
    shellE->SetValue(0, value);
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::on_horizontalSlider_threshold_sliderReleased()
{
    setThresholdValue(ui->horizontalSlider_threshold->value());
    if (myParentVtkWindow != 0)
        myParentVtkWindow->sessionModified();
}

int vtkwindow_new::getCuttingPlaneValue()
{
    return ui->cuttingPlane_Slider->value();
}

void vtkwindow_new::on_cuttingPlane_Slider_valueChanged(int value)
{

    qDebug() << "slide: " << value;
    setSliceDatacube(value - 1);

    QString velocityUnit;
    if (myParentVtkWindow != 0) {
        velocityUnit = myParentVtkWindow->selectedCubeVelocityUnit;
        myParentVtkWindow->sessionModified();
    } else {
        velocityUnit = "km/s";
    }

    double velocityValue = myfits->getInitSlice() + myfits->GetCdelt(2) * (value - 1);
    qDebug() << "velocityUnit: " << velocityUnit;
    if (velocityUnit.startsWith("m")) {
        // Returns value in km/s
        velocityValue = velocityValue / 1000;
    }
    ui->velocityLineEdit->setText(QString::number(velocityValue) + " Km/s");

    ui->spinBox_cuttingPlane->setValue(value);
    sliceA->SetPosition(0, 0, value);
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::on_spinBox_cuttingPlane_valueChanged(int arg1)
{
    ui->cuttingPlane_Slider->setValue(arg1);
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
    if (myParentVtkWindow != 0)
        myParentVtkWindow->sessionModified();
}

void vtkwindow_new::on_cameraLeft_clicked()
{
    setCameraAzimuth(-90);
}

void vtkwindow_new::on_bottomCamera_clicked()
{
    setCameraElevation(-90);
}

void vtkwindow_new::on_topCamera_clicked()
{
    setCameraElevation(90);
}

void vtkwindow_new::on_frontCamera_clicked()
{
    setCameraAzimuth(0);
}

void vtkwindow_new::on_cameraRight_clicked()
{
    setCameraAzimuth(90);
}

void vtkwindow_new::on_cameraBack_clicked()
{
    setCameraAzimuth(-180);
}

void vtkwindow_new::setCameraAzimuth(double az)
{

    resetCamera();
    //    pp->getRenderer()->GetActiveCamera()->Azimuth(az);
    m_Ren1->GetActiveCamera()->Azimuth(az);

    ui->qVTK1->renderWindow()->GetInteractor()->Render();

    if (!isDatacube)
        scale(scaleActivate);
    else
        this->updateScene();
}

void vtkwindow_new::setCameraElevation(double el)
{

    resetCamera();

    m_Ren1->GetActiveCamera()->Elevation(el);
    ui->qVTK1->renderWindow()->GetInteractor()->Render();

    if (!isDatacube)
        scale(scaleActivate);
    else
        this->updateScene();
}

void vtkwindow_new::resetCamera()
{
    qDebug() << "resetto";
    // MARI
    m_Ren1->GetActiveCamera()->SetViewUp(0, 1, 0);
    m_Ren1->GetActiveCamera()->SetFocalPoint(cam_init_foc);
    m_Ren1->GetActiveCamera()->SetPosition(cam_init_pos);

    /*
    pp->getRenderer()->GetActiveCamera()->SetViewUp( 0, 1, 0 );
    pp->getRenderer()->GetActiveCamera()->SetFocalPoint( cam_init_foc );
    pp->getRenderer()->GetActiveCamera()->SetPosition( cam_init_pos);
*/
}

void vtkwindow_new::scale(bool checked)
{
    scaleActivate = checked;
    pp->activateScale(checked);
}

void vtkwindow_new::updateScene()
{
    m_Ren1->ResetCamera();
    ui->qVTK1->update();
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::addBubble(VSTableDesktop *m_VisIVOTable)
{

    qDebug() << "ok fino a qui";

    float centroid_glat;
    float centroid_glon;
    QString contour;

    double *coord = new double[3];
    vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();

    bool isOut;

    for (unsigned long long j = 0; j < m_VisIVOTable->getNumberOfRows(); j++) {
        isOut = false;
        centroid_glat =
                atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat_cen")][j].c_str());
        centroid_glon =
                atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon_cen")][j].c_str());
        contour = QString::fromUtf8(
                m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("contour")][j].c_str());

        QStringList pieces = contour.split(",");
        QStringList fil_glon = pieces.at(0).split("_");
        QStringList fil_glat = pieces.at(1).split("_");

        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

        vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();

        polyLine->GetPointIds()->SetNumberOfIds(fil_glat.size());
        // draw filament contour
        for (int i = 0; i < fil_glat.size() - 1; i++) {
            if (AstroUtils().sky2xy(filenameWithPath, fil_glon.at(i).toDouble(),
                                    fil_glat.at(i).toDouble(), coord)) {
                points->InsertNextPoint(coord);
                polyLine->GetPointIds()->SetId(i, i);

            } else {
                isOut = true;
                break;
            }
        }
        if (!isOut) {
            if (AstroUtils().sky2xy(filenameWithPath, fil_glon.at(0).toDouble(),
                                    fil_glat.at(0).toDouble(), coord)) {
                points->InsertNextPoint(coord);
                polyLine->GetPointIds()->SetId(fil_glat.size() - 1, fil_glat.size() - 1);
            }

            vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
            cells->InsertNextCell(polyLine);

            // Create a polydata to store everything in
            vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();

            // Add the points to the dataset
            polyData->SetPoints(points);

            // Add the lines to the dataset
            polyData->SetLines(cells);

            appendFilter->AddInputData(polyData);
        }
    }
    vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanFilter->SetInputConnection(appendFilter->GetOutputPort());
    cleanFilter->Update();

    // Visualize
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cleanFilter->GetOutputPort());

    vtkSmartPointer<vtkLODActor> actor = vtkSmartPointer<vtkLODActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(1, 0, 0);

    m_Ren1->AddActor(actor);
    ui->qVTK1->update();
    ui->qVTK1->renderWindow()->GetInteractor()->Render();

    QString name = "Bubbles_" + QString::number(visualized_actor_list.count());

    visualized_actor_list.insert(name, actor);

    vtkfitstoolwidgetobject *filamentObject = new vtkfitstoolwidgetobject(2);
    filamentObject->setName(name);
    filamentObject->setActor(actor);
    addLayer(filamentObject);
}

void vtkwindow_new::addFilaments(VSTableDesktop *m_VisIVOTable)
{
    float centroid_glat;
    float centroid_glon;
    QString contour;
    QString branches_contour1d;
    QString branches_flagspine;
    QString branches_contour_new;
    QString branches_contour;
    double *coord = new double[3];
    double *branches_contour1d_coord = new double[3];
    double *branches_contour_new_coord = new double[3];
    double *branches_contour_coord = new double[3];
    vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
    vtkSmartPointer<vtkAppendPolyData> branches_contour1d_appendFilter =
            vtkSmartPointer<vtkAppendPolyData>::New();
    vtkSmartPointer<vtkAppendPolyData> branches_contour1d_appendFilter_S =
            vtkSmartPointer<vtkAppendPolyData>::New();
    vtkSmartPointer<vtkAppendPolyData> branches_contour_new_appendFilter =
            vtkSmartPointer<vtkAppendPolyData>::New();
    vtkSmartPointer<vtkAppendPolyData> branches_contour_appendFilter =
            vtkSmartPointer<vtkAppendPolyData>::New();

    bool isOut;
    bool is_S;

    for (unsigned long long j = 0; j < m_VisIVOTable->getNumberOfRows(); j++) {
        isOut = false;
        is_S = false;
        centroid_glat =
                atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat")][j].c_str());
        centroid_glon =
                atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon")][j].c_str());
        contour = QString::fromUtf8(
                m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("contour")][j].c_str());

        branches_contour1d = QString::fromUtf8(
                m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("branches_contour1d")][j]
                        .c_str());
        branches_flagspine = QString::fromUtf8(
                m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flagspine_branches")][j]
                        .c_str());
        branches_contour_new = QString::fromUtf8(
                m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("branches_contour_new")][j]
                        .c_str());
        branches_contour = QString::fromUtf8(
                m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("branches_contour")][j]
                        .c_str());

        QStringList pieces = contour.split(",");
        QStringList fil_glon = pieces.at(0).split("_");
        QStringList fil_glat = pieces.at(1).split("_");

        QStringList branches_contour1d_pieces = branches_contour1d.split(",");
        QStringList branches_contour1d_glon = branches_contour1d_pieces.at(0).split("_");
        QStringList branches_contour1d_glat = branches_contour1d_pieces.at(1).split("_");

        QStringList branches_contour_new_pieces = branches_contour_new.split(",");
        QStringList branches_contour_new_glon = branches_contour_new_pieces.at(0).split("_");
        QStringList branches_contour_new_glat = branches_contour_new_pieces.at(1).split("_");

        QStringList branches_contour_pieces = branches_contour.split(",");
        QStringList branches_contour_glon = branches_contour_pieces.at(0).split("_");
        QStringList branches_contour_glat = branches_contour_pieces.at(1).split("_");

        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
        polyLine->GetPointIds()->SetNumberOfIds(fil_glat.size() - 1);

        vtkSmartPointer<vtkPoints> branches_contour1d_points = vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkPolyLine> branches_contour1d_polyLine =
                vtkSmartPointer<vtkPolyLine>::New();
        branches_contour1d_polyLine->GetPointIds()->SetNumberOfIds(branches_contour1d_glat.size()
                                                                   - 1);

        vtkSmartPointer<vtkPoints> branches_contour1d_points_S = vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkPolyLine> branches_contour1d_polyLine_S =
                vtkSmartPointer<vtkPolyLine>::New();
        branches_contour1d_polyLine_S->GetPointIds()->SetNumberOfIds(branches_contour1d_glat.size()
                                                                     - 1);

        vtkSmartPointer<vtkPoints> branches_contour_new_points = vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkPolyLine> branches_contour_new_polyLine =
                vtkSmartPointer<vtkPolyLine>::New();
        branches_contour_new_polyLine->GetPointIds()->SetNumberOfIds(
                branches_contour_new_glat.size() - 1);

        vtkSmartPointer<vtkPoints> branches_contour_points = vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkPolyLine> branches_contour_polyLine =
                vtkSmartPointer<vtkPolyLine>::New();
        branches_contour_polyLine->GetPointIds()->SetNumberOfIds(branches_contour_glat.size() - 1);

        // draw filament contour
        //!!!!!!!!!!!!!!!!!! si dovrebbe skippare per quelli già visualizzati controllare idfil_mos
        for (int i = 0; i < fil_glat.size() - 1; i++) {
            if (AstroUtils().sky2xy(filenameWithPath, fil_glon.at(i).toDouble(),
                                    fil_glat.at(i).toDouble(), coord)) {
                points->InsertNextPoint(coord);
                polyLine->GetPointIds()->SetId(i, i);

            } else {
                isOut = true;
                break;
            }
        }
        if (!isOut) {
            /*
            if (AstroUtils().sky2xy(filenameWithPath, fil_glon.at(0).toDouble(),
            fil_glat.at(0).toDouble(), coord))
            {

               // qDebug()<<"j: "<<j<<" fil_glon.at(0).toDouble(): "<<fil_glon.at(0).toDouble()<<"
            fil_glat.at(0).toDouble(): "<<fil_glat.at(0).toDouble()<<" coord: "<<coord[0]<<"
            "<<coord[1]<<" "<<coord[2]<<" "<<fil_glat.size()-1<<" "<<fil_glat.size()-1;

                points->InsertNextPoint(coord);
                polyLine->GetPointIds()->SetId(fil_glat.size()-1, fil_glat.size()-1);

            }
*/
            vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
            cells->InsertNextCell(polyLine);

            // Create a polydata to store everything in
            vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();

            // Add the points to the dataset
            polyData->SetPoints(points);

            // Add the lines to the dataset
            polyData->SetLines(cells);

            appendFilter->AddInputData(polyData);
        }

        // draw branches - contour1d
        isOut = false;
        for (int i = 0; i < branches_contour1d_glat.size() - 1; i++) {
            if (AstroUtils().sky2xy(filenameWithPath, branches_contour1d_glon.at(i).toDouble(),
                                    branches_contour1d_glat.at(i).toDouble(),
                                    branches_contour1d_coord)) {
                branches_contour1d_points->InsertNextPoint(branches_contour1d_coord);
                branches_contour1d_polyLine->GetPointIds()->SetId(i, i);

                if (branches_flagspine.compare("s", Qt::CaseInsensitive) == 0) {
                    is_S = true;
                    branches_contour1d_points_S->InsertNextPoint(branches_contour1d_coord);
                    branches_contour1d_polyLine_S->GetPointIds()->SetId(i, i);
                }

            } else {
                isOut = true;
                break;
            }
        }
        if (!isOut) {

            vtkSmartPointer<vtkCellArray> branches_contour1d_cells =
                    vtkSmartPointer<vtkCellArray>::New();
            branches_contour1d_cells->InsertNextCell(branches_contour1d_polyLine);

            // Create a polydata to store everything in
            vtkSmartPointer<vtkPolyData> branches_contour1d_polyData =
                    vtkSmartPointer<vtkPolyData>::New();

            // Add the points to the dataset
            branches_contour1d_polyData->SetPoints(branches_contour1d_points);

            // Add the lines to the dataset
            branches_contour1d_polyData->SetLines(branches_contour1d_cells);

            branches_contour1d_appendFilter->AddInputData(branches_contour1d_polyData);

            if (is_S) {
                // BRANCHES_S
                vtkSmartPointer<vtkCellArray> branches_contour1d_cells_S =
                        vtkSmartPointer<vtkCellArray>::New();
                branches_contour1d_cells_S->InsertNextCell(branches_contour1d_polyLine_S);

                // Create a polydata to store everything in
                vtkSmartPointer<vtkPolyData> branches_contour1d_polyData_S =
                        vtkSmartPointer<vtkPolyData>::New();

                // Add the points to the dataset
                branches_contour1d_polyData_S->SetPoints(branches_contour1d_points_S);

                // Add the lines to the dataset
                branches_contour1d_polyData_S->SetLines(branches_contour1d_cells_S);

                branches_contour1d_appendFilter_S->AddInputData(branches_contour1d_polyData_S);
            }
        }

        // draw Branches - contour_new
        for (int i = 0; i < branches_contour_new_glat.size() - 1; i++) {
            if (AstroUtils().sky2xy(filenameWithPath, branches_contour_new_glon.at(i).toDouble(),
                                    branches_contour_new_glat.at(i).toDouble(),
                                    branches_contour_new_coord)) {
                branches_contour_new_points->InsertNextPoint(branches_contour_new_coord);
                branches_contour_new_polyLine->GetPointIds()->SetId(i, i);
            } else {
                isOut = true;
                break;
            }
        }
        if (!isOut) {
            vtkSmartPointer<vtkCellArray> branches_contour_new_cells =
                    vtkSmartPointer<vtkCellArray>::New();
            branches_contour_new_cells->InsertNextCell(branches_contour_new_polyLine);

            // Create a polydata to store everything in
            vtkSmartPointer<vtkPolyData> branches_contour_new_polyData =
                    vtkSmartPointer<vtkPolyData>::New();

            // Add the points to the dataset
            branches_contour_new_polyData->SetPoints(branches_contour_new_points);

            // Add the lines to the dataset
            branches_contour_new_polyData->SetLines(branches_contour_new_cells);

            branches_contour_new_appendFilter->AddInputData(branches_contour_new_polyData);
        }

        // draw Branches - contour
        for (int i = 0; i < branches_contour_glat.size() - 1; i++) {
            if (AstroUtils().sky2xy(filenameWithPath, branches_contour_glon.at(i).toDouble(),
                                    branches_contour_glat.at(i).toDouble(),
                                    branches_contour_coord)) {
                branches_contour_points->InsertNextPoint(branches_contour_coord);
                branches_contour_polyLine->GetPointIds()->SetId(i, i);
            } else {
                isOut = true;
                break;
            }
        }
        if (!isOut) {
            vtkSmartPointer<vtkCellArray> branches_contour_cells =
                    vtkSmartPointer<vtkCellArray>::New();
            branches_contour_cells->InsertNextCell(branches_contour_polyLine);

            // Create a polydata to store everything in
            vtkSmartPointer<vtkPolyData> branches_contour_polyData =
                    vtkSmartPointer<vtkPolyData>::New();

            // Add the points to the dataset
            branches_contour_polyData->SetPoints(branches_contour_points);

            // Add the lines to the dataset
            branches_contour_polyData->SetLines(branches_contour_cells);

            branches_contour_appendFilter->AddInputData(branches_contour_polyData);
        }
    }

    QStringList names;
    // Contorno filamento
    vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanFilter->SetInputConnection(appendFilter->GetOutputPort());
    cleanFilter->Update();
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cleanFilter->GetOutputPort());
    vtkSmartPointer<vtkLODActor> actor = vtkSmartPointer<vtkLODActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0, 1, 0);
    QString name =
            stringDictWidget->getColDescStringDict().value("vlkb_filaments.filaments.contour");
    names << name;
    addCombinedLayer(name, actor, 2, true);

    // branches Contour1d
    vtkSmartPointer<vtkCleanPolyData> branches_contour1d_cleanFilter =
            vtkSmartPointer<vtkCleanPolyData>::New();
    branches_contour1d_cleanFilter->SetInputConnection(
            branches_contour1d_appendFilter->GetOutputPort());
    branches_contour1d_cleanFilter->Update();
    vtkSmartPointer<vtkPolyDataMapper> branches_contour1d_mapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
    branches_contour1d_mapper->SetInputConnection(branches_contour1d_cleanFilter->GetOutputPort());
    vtkSmartPointer<vtkLODActor> branches_contour1d_actor = vtkSmartPointer<vtkLODActor>::New();
    branches_contour1d_actor->SetMapper(branches_contour1d_mapper);
    branches_contour1d_actor->GetProperty()->SetColor(1, 0, 0);
    name = stringDictWidget->getColDescStringDict().value("vlkb_filaments.branches.contour1d");
    names << name;
    addCombinedLayer(name, branches_contour1d_actor, 2, true);

    // branches Contour
    vtkSmartPointer<vtkCleanPolyData> branches_contour_cleanFilter =
            vtkSmartPointer<vtkCleanPolyData>::New();
    branches_contour_cleanFilter->SetInputConnection(
            branches_contour_appendFilter->GetOutputPort());
    branches_contour_cleanFilter->Update();
    vtkSmartPointer<vtkPolyDataMapper> branches_contour_mapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
    branches_contour_mapper->SetInputConnection(branches_contour_cleanFilter->GetOutputPort());
    vtkSmartPointer<vtkLODActor> branches_contour_actor = vtkSmartPointer<vtkLODActor>::New();
    branches_contour_actor->SetMapper(branches_contour_mapper);
    branches_contour_actor->GetProperty()->SetColor(0, 1, 1);
    name = stringDictWidget->getColDescStringDict().value("vlkb_filaments.branches.contour");
    names << name;
    addCombinedLayer(name, branches_contour_actor, 2, false);

    filamentsList.insert(QString::fromStdString(m_VisIVOTable->getName()), names);
    sessionModified();
}

void vtkwindow_new::addCombinedLayer(QString name, vtkSmartPointer<vtkLODActor> actor, int objtype,
                                     bool active)

{
    if (VisualizedEllipseSourcesList.contains(name)) {
        vtkSmartPointer<vtkAppendPolyData> appendFilter2 =
                vtkSmartPointer<vtkAppendPolyData>::New();

        appendFilter2->AddInputData(vtkPolyData::SafeDownCast(
                VisualizedEllipseSourcesList.value(name)->GetMapper()->GetInputAsDataSet()));

        appendFilter2->AddInputData(
                vtkPolyData::SafeDownCast(actor->GetMapper()->GetInputAsDataSet()));

        // Remove any duplicate points.
        vtkSmartPointer<vtkCleanPolyData> cleanFilter2 = vtkSmartPointer<vtkCleanPolyData>::New();
        cleanFilter2->SetInputConnection(appendFilter2->GetOutputPort());
        cleanFilter2->Update();

        vtkSmartPointer<vtkPolyDataMapper> mapper2 = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper2->SetInputConnection(cleanFilter2->GetOutputPort());

        VisualizedEllipseSourcesList.value(name)->SetMapper(mapper2);

        m_Ren1->AddActor(VisualizedEllipseSourcesList.value(name));
    }

    else {

        visualized_actor_list.insert(name, actor);

        vtkfitstoolwidgetobject *singleBandObject = new vtkfitstoolwidgetobject(objtype);
        // singleBandObject->setParentItem(imageObject);
        singleBandObject->setName(name);

        // actor->GetProperty()->SetColor(0, 1,0);

        VisualizedEllipseSourcesList.insert(name, actor);
        m_Ren1->AddActor(actor);
        singleBandObject->setActor(actor);

        addLayer(singleBandObject, active);
        if (!active)
            actor->VisibilityOff();
    }
    // end mod fv
}

void vtkwindow_new::addSources(VSTableDesktop *m_VisIVOTable)
{
    QHash<QString, vtkEllipse *> ellipse_list;

    float semiMajorAxisLength;
    float semiMinorAxisLength;
    float angle;
    float ra;
    float dec;
    QString sourceName;
    double *coord = new double[3];

    for (unsigned long long j = 0; j < m_VisIVOTable->getNumberOfRows(); j++) {
        semiMajorAxisLength =
                atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("FWHMA")][j].c_str())
                / 2;
        semiMinorAxisLength =
                atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("FWHMB")][j].c_str())
                / 2;
        angle = atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("PA")][j].c_str());
        ra = atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("GLON")][j].c_str());
        dec = atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("GLAT")][j].c_str());
        sourceName = QString::fromStdString(
                m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("DESIGNATION")][j]);

        if (AstroUtils().sky2xy(filenameWithPath, ra, dec, coord)) {

            ellipse_list.insert(sourceName,
                                new vtkEllipse(semiMajorAxisLength, semiMinorAxisLength, angle,
                                               coord[0], coord[1], coord[2], 0, j, sourceName,
                                               m_VisIVOTable));

            /*
        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        const float p[3] = {coord[0], coord[1], 0};
        vtkSmartPointer<vtkCellArray> vertices =          vtkSmartPointer<vtkCellArray>::New();
        vtkIdType pid[1];
        pid[0] = points->InsertNextPoint(p);
        vertices->InsertNextCell(1,pid);
        // Create a polydata object
        vtkSmartPointer<vtkPolyData> point =
                vtkSmartPointer<vtkPolyData>::New();

        // Set the points and vertices we created as the geometry and topology of the polydata
        point->SetPoints(points);
        point->SetVerts(vertices);

        // Visualize
        vtkSmartPointer<vtkPolyDataMapper> mapper =
                vtkSmartPointer<vtkPolyDataMapper>::New();

#if VTK_MAJOR_VERSION <= 5
        mapper->SetInput(point);
#else
        mapper->SetInputData(point);
#endif

        vtkSmartPointer<vtkActor> actor =
                vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetPointSize(2);
        actor->GetProperty()->SetColor(1.0,0.0,0.0);


        m_Ren1->AddActor(actor);

*/
        }
    }

    file_wavelength.insert(QString::fromStdString(m_VisIVOTable->getName()),
                           m_VisIVOTable->getWavelength());
    drawEllipse(ellipse_list, QString::fromStdString(m_VisIVOTable->getName()),
                QString::fromStdString(m_VisIVOTable->getName()));

    this->update();
    sessionModified();
}

void vtkwindow_new::showFilteredSources(const QStringList &ids)
{
    // Remove existing actor
    auto renderer = ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer();
    renderer->RemoveActor(filteredSources);

    auto sources = catalogue->getSources();
    double arcsec = AstroUtils::arcsecPixel(myfits->GetFileName());
    auto appendPolyData = vtkSmartPointer<vtkAppendPolyData>::New();
    foreach (const QString &id, ids) {
        Source *s = sources.value(id);
        if (!s)
            continue;

        foreach (auto &&island, s->getIslands()) {
            auto vertices = island->getVertices();
            if (vertices.isEmpty()) {
                continue;
            }

            auto points = vtkSmartPointer<vtkPoints>::New();
            auto cells = vtkSmartPointer<vtkCellArray>::New();
            cells->InsertNextCell(vertices.count() + 1);
            foreach (auto &&point, vertices) {
                vtkIdType id = points->InsertNextPoint(point.first + 1, point.second + 1, arcsec);
                cells->InsertCellPoint(id);
            }
            cells->InsertCellPoint(0);

            auto polyData = vtkSmartPointer<vtkPolyData>::New();
            polyData->SetPoints(points);
            polyData->SetLines(cells);
            appendPolyData->AddInputData(polyData);
        }
    }

    auto cleanPolyData = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanPolyData->SetInputConnection(appendPolyData->GetOutputPort());

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cleanPolyData->GetOutputPort());

    filteredSources = vtkSmartPointer<vtkLODActor>::New();
    filteredSources->SetMapper(mapper);
    filteredSources->GetProperty()->SetColor(0, 1, 1);
    renderer->AddActor(filteredSources);

    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::hideFilteredSources()
{
    auto renderer = ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer();
    renderer->RemoveActor(filteredSources);
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::loadDS9RegionFile()
{
    QString fn = QFileDialog::getOpenFileName(this, "DS9 Region file", QDir::homePath(),
                                              "DS9 Region file (*.reg)");
    if (!fn.isEmpty())
        addDS9Regions(fn);
}

void vtkwindow_new::addDS9Regions(const QString &filepath)
{
    std::vector<DS9Region *> regions;
    std::string error;
    int result = DS9RegionParser::Parse(regions, filepath.toStdString(), error);
    if (result != 0) {
        QMessageBox::critical(this, "Error", QString::fromStdString(error));
        return;
    }

    std::vector<DS9Region *> polygons, circles, boxes, ellipses;
    for (const auto &region : regions) {
        switch (region->shapeType) {
        case DS9Region::ePOLYGON_SHAPE:
            polygons.push_back(region);
            break;
        case DS9Region::eCIRCLE_SHAPE:
            circles.push_back(region);
            break;
        case DS9Region::eBOX_SHAPE:
            boxes.push_back(region);
            break;
        case DS9Region::eELLIPSE_SHAPE:
            ellipses.push_back(region);
            break;
        }
    }

    if (!polygons.empty())
        drawPolygonRegions(polygons);

    if (!circles.empty())
        drawCircleRegions(circles);

    if (!boxes.empty())
        drawBoxRegions(boxes);

    if (!ellipses.empty())
        drawEllipseRegions(ellipses);

    ds9RegionFiles << filepath;
    sessionModified();
}

int vtkwindow_new::getDS9RegionCoordSystem(const DS9Region *region)
{
    switch (region->csType) {
    case WCSType::eGALACTIC:
        return WCS_GALACTIC;
    case WCSType::eJ2000:
        return WCS_J2000;
    case WCSType::eB1950:
        return WCS_B1950;
    }

    // Assume image
    return WCS_XY;
}

void vtkwindow_new::drawPolygonRegions(const std::vector<DS9Region *> &polygons)
{
    double arcsec = AstroUtils::arcsecPixel(myfits->GetFileName());
    auto appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();

    std::for_each(polygons.begin(), polygons.end(),
                  [this, arcsec, &appendFilter](DS9Region *region) {
                      auto polygon = dynamic_cast<DS9PolygonRegion *>(region);
                      int cs = getDS9RegionCoordSystem(region);

                      auto points = vtkSmartPointer<vtkPoints>::New();
                      auto cells = vtkSmartPointer<vtkCellArray>::New();
                      cells->InsertNextCell(polygon->points.size() + 1);

                      for (const auto &point : polygon->points) {
                          double coords[3] = { point.first, point.second, arcsec };

                          if (cs != WCS_XY) {
                              AstroUtils::sky2xy_t(myfits->GetFileName(), point.first, point.second,
                                                   cs, &coords[0], &coords[1]);
                          }

                          vtkIdType id = points->InsertNextPoint(coords);
                          cells->InsertCellPoint(id);
                      }
                      cells->InsertCellPoint(0);

                      auto polyData = vtkSmartPointer<vtkPolyData>::New();
                      polyData->SetPoints(points);
                      polyData->SetLines(cells);

                      appendFilter->AddInputData(polyData);
                  });

    auto cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanFilter->SetInputConnection(appendFilter->GetOutputPort());
    cleanFilter->Update();

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cleanFilter->GetOutputPort());

    auto actor = vtkSmartPointer<vtkLODActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(1, 0, 0);

    addCombinedLayer("DS9 Region - Polygons", actor, 2, true);
}

void vtkwindow_new::drawCircleRegions(const std::vector<DS9Region *> &circles)
{
    double arcsec = AstroUtils::arcsecPixel(myfits->GetFileName());
    auto appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();

    std::for_each(circles.begin(), circles.end(), [this, arcsec, &appendFilter](DS9Region *region) {
        auto circle = dynamic_cast<DS9CircleRegion *>(region);
        int cs = getDS9RegionCoordSystem(region);

        double center[3] = { circle->cx, circle->cy, arcsec };
        double radius = circle->r;

        if (cs != WCS_XY) {
            AstroUtils::sky2xy_t(myfits->GetFileName(), circle->cx, circle->cy, cs, &center[0],
                                 &center[1]);
            radius = radius / arcsec;
        }

        auto circleSource = vtkSmartPointer<vtkRegularPolygonSource>::New();
        circleSource->GeneratePolygonOff();
        circleSource->SetNumberOfSides(50);
        circleSource->SetRadius(radius);
        circleSource->SetCenter(center);
        circleSource->Update();

        appendFilter->AddInputData(circleSource->GetOutput());
    });

    auto cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanFilter->SetInputConnection(appendFilter->GetOutputPort());
    cleanFilter->Update();

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cleanFilter->GetOutputPort());

    auto actor = vtkSmartPointer<vtkLODActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0, 1, 0);

    addCombinedLayer("DS9 Region - Circles", actor, 2, true);
}

void vtkwindow_new::drawBoxRegions(const std::vector<DS9Region *> &boxes)
{
    double arcsec = AstroUtils::arcsecPixel(myfits->GetFileName());
    auto appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();

    std::for_each(boxes.begin(), boxes.end(), [this, arcsec, &appendFilter](DS9Region *region) {
        auto box = dynamic_cast<DS9BoxRegion *>(region);
        int cs = getDS9RegionCoordSystem(region);

        double center[2] = { box->cx, box->cy };
        double semiwidth = box->width * 0.5;
        double semiheight = box->height * 0.5;
        double angle = box->theta;

        if (cs != WCS_XY) {
            AstroUtils::sky2xy_t(myfits->GetFileName(), box->cx, box->cy, cs, &center[0],
                                 &center[1]);
            semiwidth = box->width / arcsec * 0.5;
            semiheight = box->height / arcsec * 0.5;

            double delta = 0;
            AstroUtils::getRotationAngle(myfits->GetFileName(), &delta, cs);
            angle += delta;
        }

        auto points = vtkSmartPointer<vtkPoints>::New();
        auto cells = vtkSmartPointer<vtkCellArray>::New();
        cells->InsertNextCell(5);

        vtkIdType tl =
                points->InsertNextPoint(center[0] - semiwidth, center[1] + semiheight, arcsec);
        cells->InsertCellPoint(tl);
        vtkIdType tr =
                points->InsertNextPoint(center[0] + semiwidth, center[1] + semiheight, arcsec);
        cells->InsertCellPoint(tr);
        vtkIdType br =
                points->InsertNextPoint(center[0] + semiwidth, center[1] - semiheight, arcsec);
        cells->InsertCellPoint(br);
        vtkIdType bl =
                points->InsertNextPoint(center[0] - semiwidth, center[1] - semiheight, arcsec);
        cells->InsertCellPoint(bl);
        cells->InsertCellPoint(tl);

        auto polyData = vtkSmartPointer<vtkPolyData>::New();
        polyData->SetPoints(points);
        polyData->SetLines(cells);

        auto transform = vtkSmartPointer<vtkTransform>::New();
        transform->PostMultiply();
        transform->Translate(-center[0], -center[1], -arcsec);
        transform->RotateZ(angle);
        transform->Translate(center[0], center[1], arcsec);

        auto transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
        transformFilter->SetInputData(polyData);
        transformFilter->SetTransform(transform);
        transformFilter->Update();

        appendFilter->AddInputData(transformFilter->GetOutput());
    });

    auto cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanFilter->SetInputConnection(appendFilter->GetOutputPort());
    cleanFilter->Update();

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cleanFilter->GetOutputPort());

    auto actor = vtkSmartPointer<vtkLODActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0, 0, 1);

    addCombinedLayer("DS9 Region - Boxes", actor, 2, true);
}

void vtkwindow_new::drawEllipseRegions(const std::vector<DS9Region *> &ellipses)
{
    double arcsec = AstroUtils::arcsecPixel(myfits->GetFileName());
    auto appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();

    std::for_each(ellipses.begin(), ellipses.end(),
                  [this, arcsec, &appendFilter](DS9Region *region) {
                      auto ellipse = dynamic_cast<DS9EllipseRegion *>(region);
                      int cs = getDS9RegionCoordSystem(region);

                      double center[2] = { ellipse->cx, ellipse->cy };
                      double semiMajorAxis = ellipse->a;
                      double semiMinorAxis = ellipse->b;
                      double angle = ellipse->theta;

                      if (cs != WCS_XY) {
                          AstroUtils::sky2xy_t(myfits->GetFileName(), ellipse->cx, ellipse->cy, cs,
                                               &center[0], &center[1]);

                          semiMajorAxis = semiMajorAxis / arcsec;
                          semiMinorAxis = semiMinorAxis / arcsec;

                          double delta = 0;
                          AstroUtils::getRotationAngle(myfits->GetFileName(), &delta, cs);
                          angle += delta;
                      }

                      auto points = vtkSmartPointer<vtkPoints>::New();
                      auto cells = vtkSmartPointer<vtkCellArray>::New();
                      cells->InsertNextCell(41);
                      for (double i = 0; i < 2 * vtkMath::Pi(); i += vtkMath::Pi() / 20) {
                          vtkIdType id = points->InsertNextPoint(semiMajorAxis * cos(i) + center[0],
                                                                 semiMinorAxis * sin(i) + center[1],
                                                                 arcsec);
                          cells->InsertCellPoint(id);
                      }
                      cells->InsertCellPoint(0);
                      cells->UpdateCellCount(points->GetNumberOfPoints() + 1);

                      auto polyData = vtkSmartPointer<vtkPolyData>::New();
                      polyData->SetPoints(points);
                      polyData->SetLines(cells);

                      auto transform = vtkSmartPointer<vtkTransform>::New();
                      transform->PostMultiply();
                      transform->Translate(-center[0], -center[1], -arcsec);
                      transform->RotateZ(angle);
                      transform->Translate(center[0], center[1], arcsec);

                      auto transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
                      transformFilter->SetInputData(polyData);
                      transformFilter->SetTransform(transform);
                      transformFilter->Update();

                      appendFilter->AddInputData(transformFilter->GetOutput());
                  });

    auto cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanFilter->SetInputConnection(appendFilter->GetOutputPort());
    cleanFilter->Update();

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cleanFilter->GetOutputPort());

    auto actor = vtkSmartPointer<vtkLODActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(1, 1, 0);

    addCombinedLayer("DS9 Region - Ellipses", actor, 2, true);
}

void vtkwindow_new::addSourcesFromJson(const QString &fn)
{
    this->catalogue = new Catalogue(this, fn, ui->qVTK1->renderWindow());
    auto sources = catalogue->getSources();
    if (sources.isEmpty()) {
        QMessageBox::information(this, "No sources", "The file does not contain any sources.");
        return;
    }

    double arcsec = AstroUtils::arcsecPixel(myfits->GetFileName());
    QMutex mutex;
    auto appendPolyData = vtkSmartPointer<vtkAppendPolyData>::New();

    QtConcurrent::blockingMap(sources, [arcsec, &appendPolyData, &mutex](Source *s) {
        foreach (auto &&island, s->getIslands()) {
            auto vertices = island->getVertices();
            if (vertices.isEmpty()) {
                continue;
            }

            auto points = vtkSmartPointer<vtkPoints>::New();
            auto cells = vtkSmartPointer<vtkCellArray>::New();
            cells->InsertNextCell(vertices.count() + 1);
            foreach (auto &&point, vertices) {
                vtkIdType id = points->InsertNextPoint(point.first + 1, point.second + 1, arcsec);
                cells->InsertCellPoint(id);
            }
            cells->InsertCellPoint(0);

            auto polyData = vtkSmartPointer<vtkPolyData>::New();
            polyData->SetPoints(points);
            polyData->SetLines(cells);

            mutex.lock();
            appendPolyData->AddInputData(polyData);
            mutex.unlock();
        }
    });

    auto cleanPolyData = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanPolyData->SetInputConnection(appendPolyData->GetOutputPort());
    catalogue->SetPolyDataFilter(cleanPolyData);

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cleanPolyData->GetOutputPort());

    auto actor = vtkSmartPointer<vtkLODActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(1, 0, 0);
    catalogue->SetActor(actor);

    auto renderer = ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer();
    renderer->AddActor(actor);

    ui->qVTK1->renderWindow()->GetInteractor()->Render();

    jsonRegionFiles.append(fn);
}

void vtkwindow_new::closeEvent(QCloseEvent *event)
{
    if (vtkwintype == 1) {
        removeContour();
        if (myParentVtkWindow != 0)
            myParentVtkWindow->sessionModified();
    }

    auto vl = &Singleton<ViaLactea>::Instance();
    if (vl->isMasterWin(this)) {
        if (!isSessionSaved() && !confirmSaveAndExit()) {
            // Cancel button was clicked, therefore do not close
            event->ignore();
            return;
        }

        vl->resetMasterWin();
    }
}

void vtkwindow_new::updateSpecies()
{

    ui->lineEdit_species->setText(species);
}

void vtkwindow_new::updateSurvey()
{

    ui->lineEdit_survey->setText(survey);
}

void vtkwindow_new::updateTransition()
{
    ui->lineEdit_transition->setText(transition);
}

void vtkwindow_new::setSpecies(QString q)
{
    species = q;
    vtkcontourwindow->species = q;
    emit speciesChanged();
}
void vtkwindow_new::setSurvey(QString q)
{
    survey = q;
    vtkcontourwindow->survey = q;
    emit surveyChanged();
}

void vtkwindow_new::setTransition(QString q)
{
    transition = q;
    vtkcontourwindow->transition = q;
    emit transitionChanged();
}

/*
 void vtkwindow_new::drawEllipse(QHash<QString,vtkEllipse *> ellipse, QString sourceFilename )
{
    QString ori_sourceFilename=sourceFilename;
    //    sourceFilename=sourceFilename+"_"+QString::number( visualized_actor_list.count() );
    sourceFilename=sourceFilename+"_"+QString::number( visualized_actor_list.count() );

    vtkSmartPointer<vtkAppendPolyData> appendFilter =vtkSmartPointer<vtkAppendPolyData>::New();
    ellipse_list.unite(ellipse);

    foreach(vtkEllipse *el, ellipse )
    {
#if VTK_MAJOR_VERSION <= 5
        appendFilter->AddInputConnection(el->getPolyData()->GetProducerPort());
#else
        appendFilter->AddInputData(el->getPolyData());
        //  appendFilter->AddInputConnection(el->getPolyData()->GetProducerPort());
#endif


        designation2fileMap.insert(el->getSourceName(), sourceFilename);

    }


    appendFilter->Update();

    // Remove any duplicate points.
    vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanFilter->SetInputConnection(appendFilter->GetOutputPort());
    cleanFilter->Update();


    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cleanFilter->GetOutputPort());



    vtkSmartPointer<vtkLODActor> ellipseActor = vtkSmartPointer<vtkLODActor>::New();
    ellipseActor->SetMapper(mapper);

    double numOfLayer = visualized_actor_list.count();
    //Color of ellipse actor, iterate throght Qt::GlobalColor
    QColor c= QColor( Qt::GlobalColor( Qt::red+numOfLayer ) );
    ellipseActor->GetProperty()->SetColor(c.redF(), c.greenF(), c.blueF());

    m_Ren1->AddActor(ellipseActor);
    if(ori_sourceFilename.compare(stringDictWidget->getColUtypeStringDict().value("vlkb_compactsources.sed_view_final.designationft"))!=0)
        ellipseActor->VisibilityOff();

    qDebug()<<"ADD: "<<ori_sourceFilename;



    ellipse_actor_list.insert( sourceFilename ,ellipseActor);
    visualized_actor_list.insert(sourceFilename,ellipseActor);

    vtkfitstoolwidgetobject* singleBandObject=new vtkfitstoolwidgetobject(1);
    singleBandObject->setParentItem(imageObject);
    singleBandObject->setName(sourceFilename);
    singleBandObject->setActor(ellipseActor);


    if(ori_sourceFilename.compare(stringDictWidget->getColUtypeStringDict().value("vlkb_compactsources.sed_view_final.designationft"))==0)
        addLayer(singleBandObject,true);
    else
        addLayer(singleBandObject,false);


}
*/

void vtkwindow_new::drawEllipse(QHash<QString, vtkEllipse *> ellipse, QString sourceFilename,
                                QString sourcePath)
{
    if (QDir::isAbsolutePath(sourceFilename))
        sourceFilename = QFileInfo(sourceFilename).fileName();

    QString ori_sourceFilename = sourceFilename;
    // sourceFilename=sourceFilename+"_"+QString::number( visualized_actor_list.count() );

    vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();

    // ellipse_list.unite(ellipse);
    ellipse_list = ellipse;

    foreach (vtkEllipse *el, ellipse) {

        appendFilter->AddInputData(el->getPolyData());
        designation2fileMap.insert(el->getSourceName(), sourceFilename);
    }

    appendFilter->Update();

    // Remove any duplicate points.
    vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanFilter->SetInputConnection(appendFilter->GetOutputPort());
    cleanFilter->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cleanFilter->GetOutputPort());

    vtkSmartPointer<vtkLODActor> ellipseActor = vtkSmartPointer<vtkLODActor>::New();
    ellipseActor->SetMapper(mapper);

    // mod fv  accorpamento layers

    if (VisualizedEllipseSourcesList.contains(ori_sourceFilename)) {

        vtkSmartPointer<vtkAppendPolyData> appendFilter2 =
                vtkSmartPointer<vtkAppendPolyData>::New();

        appendFilter2->AddInputData(
                vtkPolyData::SafeDownCast(VisualizedEllipseSourcesList.value(ori_sourceFilename)
                                                  ->GetMapper()
                                                  ->GetInputAsDataSet()));

        appendFilter2->AddInputData(
                vtkPolyData::SafeDownCast(ellipseActor->GetMapper()->GetInputAsDataSet()));

        // Remove any duplicate points.
        vtkSmartPointer<vtkCleanPolyData> cleanFilter2 = vtkSmartPointer<vtkCleanPolyData>::New();
        cleanFilter2->SetInputConnection(appendFilter2->GetOutputPort());
        cleanFilter2->Update();

        vtkSmartPointer<vtkPolyDataMapper> mapper2 = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper2->SetInputConnection(cleanFilter2->GetOutputPort());

        VisualizedEllipseSourcesList.value(ori_sourceFilename)->SetMapper(mapper2);

        m_Ren1->AddActor(VisualizedEllipseSourcesList.value(ori_sourceFilename));
    } else {

        ellipse_actor_list.insert(sourceFilename, ellipseActor);
        visualized_actor_list.insert(sourceFilename, ellipseActor);

        vtkfitstoolwidgetobject *singleBandObject = new vtkfitstoolwidgetobject(1);
        singleBandObject->setParentItem(imageObject);
        singleBandObject->setName(sourceFilename);
        singleBandObject->setPath(sourcePath);

        QColor c = QColor(Qt::GlobalColor(Qt::red + VisualizedEllipseSourcesList.count()));
        ellipseActor->GetProperty()->SetColor(c.redF(), c.greenF(), c.blueF());

        VisualizedEllipseSourcesList.insert(ori_sourceFilename, ellipseActor);
        m_Ren1->AddActor(ellipseActor);
        singleBandObject->setActor(ellipseActor);

        // if(ori_sourceFilename.compare(stringDictWidget->getColUtypeStringDict().value("vlkb_compactsources.sed_view_final.designationft"))==0)

        if (ori_sourceFilename.compare("Band-merged Source Designation") == 0)
            addLayer(singleBandObject, true);
        else {
            addLayer(singleBandObject, false);
            ellipseActor->VisibilityOff();
        }
    }

    // end mod fv accorpamento layers

    /*
    double numOfLayer = visualized_actor_list.count();

    QColor c= QColor( Qt::GlobalColor( Qt::red+numOfLayer ) );
    ellipseActor->GetProperty()->SetColor(c.redF(), c.greenF(), c.blueF());

    m_Ren1->AddActor(ellipseActor);

    if(ori_sourceFilename.compare(stringDictWidget->getColUtypeStringDict().value("vlkb_compactsources.sed_view_final.designationft"))!=0)
        ellipseActor->VisibilityOff();

    qDebug()<<"ADD: "<<ori_sourceFilename;

    ellipse_actor_list.insert( sourceFilename ,ellipseActor);
    visualized_actor_list.insert(sourceFilename,ellipseActor);

    vtkfitstoolwidgetobject* singleBandObject=new vtkfitstoolwidgetobject(1);
    singleBandObject->setParentItem(imageObject);
    singleBandObject->setName(sourceFilename);
    singleBandObject->setActor(ellipseActor);


    if(ori_sourceFilename.compare(stringDictWidget->getColUtypeStringDict().value("vlkb_compactsources.sed_view_final.designationft"))==0)
        addLayer(singleBandObject,true);
    else
        addLayer(singleBandObject,false);
*/
}

void vtkwindow_new::removeActor(vtkProp *actor)
{
    ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor2D(actor);
    ui->qVTK1->renderWindow()->Render();
}

void vtkwindow_new::changeScalar(std::string scalar)
{
    pp->colorScalar = scalar;
    pp->setLookupTable();
    // FV
    pp->setLookupTableScale();

    qDebug() << "Lut Scalar " << QString::fromStdString(scalar);
    vtkSmartPointer<vtkDataArray> data =
            pp->getPolyData()->GetPointData()->GetArray(scalar.c_str());
    if (data != 0) {
        double range[2];
        data->GetRange(range);
        qDebug() << range[0] << " " << range[1];
    }
    ui->qVTK1->update();
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::showColorbar(bool checked)
{

    pp->showColorBar = checked;
    pp->scalarBar->SetVisibility(checked);

    ui->qVTK1->update();
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::slot_clicked(vtkObject *, unsigned long, void *, void *)
{

    // Forward events
    vtkSmartPointer<vtkCoordinate> coordinate = vtkSmartPointer<vtkCoordinate>::New();
    coordinate->SetCoordinateSystemToDisplay();
    coordinate->SetValue(ui->qVTK1->renderWindow()->GetInteractor()->GetEventPosition()[0],
                         ui->qVTK1->renderWindow()->GetInteractor()->GetEventPosition()[1], 0);

    double *world_coord = coordinate->GetComputedWorldValue(ui->qVTK1->renderWindow()
                                                                    ->GetInteractor()
                                                                    ->GetRenderWindow()
                                                                    ->GetRenderers()
                                                                    ->GetFirstRenderer());

    vtkSmartPointer<vtkImageActorPointPlacer> pointPlacer =
            vtkSmartPointer<vtkImageActorPointPlacer>::New();
    pointPlacer->SetImageActor(imageViewer->GetImageActor());

    if (pointPlacer->ValidateWorldPosition(world_coord) == 1) {

        // cerca le sorgenti che cadono dove hai cliccato

        /*
        vtkSmartPointer<vtkSelectEnclosedPoints> selectEnclosedPoints =
            vtkSmartPointer<vtkSelectEnclosedPoints>::New();
        selectEnclosedPoints->SetInput(pointsPolydata);
*/

        QSignalMapper *signalMapper = new QSignalMapper(this);

        QMenu *menu = new QMenu(this);
        // QAction *viewAct = new QAction(tr("&View"), this);
        QAction *viewDC = new QAction(tr("&Cutout Datacube"), this);
        // viewAct->setStatusTip(tr("Visualize selected map file"));
        connect(viewDC, SIGNAL(triggered()), signalMapper, SLOT(map()));

        // Forward events
        vtkSmartPointer<vtkCoordinate> coordinate = vtkSmartPointer<vtkCoordinate>::New();
        coordinate->SetCoordinateSystemToDisplay();
        coordinate->SetValue(ui->qVTK1->interactor()->GetEventPosition()[0],
                             ui->qVTK1->interactor()->GetEventPosition()[1], 0);

        double *world_coord = coordinate->GetComputedWorldValue(
                ui->qVTK1->interactor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

        double *sky_coord_gal = new double[2];

        AstroUtils().xy2sky(filenameWithPath, world_coord[0], world_coord[1], sky_coord_gal, 3);

        signalMapper->setMapping(viewDC,
                                 QString::number(sky_coord_gal[0]) + ";"
                                         + QString::number(sky_coord_gal[1]));

        connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(cutoutDatacube(QString)));

        menu->addAction(viewDC);

        menu->popup(QCursor::pos());
    }
}

void vtkwindow_new::changePalette(std::string palette)
{
    pp->palette = palette;
    //  pp->setLookupTable();
    pp->setLookupTableScale();

    ui->qVTK1->update();
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

FitsImageStatisiticInfo *vtkwindow_new::getInfoWindow()
{
    return info;
}

void vtkwindow_new::cutoutDatacube(QString c)
{
    QStringList splittedStrings = c.split(";");

    dbquery *queryWindow = new dbquery();
    queryWindow->setCoordinate(splittedStrings.at(0), splittedStrings.at(1));
    queryWindow->show();
}

void vtkwindow_new::openFilterDialog()
{
    SFilterDialog *d = new SFilterDialog(this->catalogue, this);
    d->setAttribute(Qt::WA_DeleteOnClose);
    // d->setModal(true);
    d->show();
    d->raise();
    d->activateWindow();
    connect(d, &SFilterDialog::accepted, this, [this]() {
        showFilteredSources(catalogue->getFilteredIds());
        showSourceDockWidget();
    });
}

void vtkwindow_new::addLocalSources()
{
    vtkfitstoolsw->on_addLocalSourcesPushButton_clicked();
}

void vtkwindow_new::changeFitsScale(std::string palette, std::string scale)
{

    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();

    QString myscale = scale.c_str();

    // applico la lut al livello selezionato nella tabella in basso adx, se nessuna selezione è
    // fatta appico al livello 0
    int pos = 0;

    if (ui->listWidget->selectionModel()->selectedRows().count() != 0
        && imgLayerList.at(ui->listWidget->selectionModel()->selectedRows().at(0).row())->getType()
                == 0) {
        qDebug() << "inside";
        pos = ui->listWidget->selectionModel()->selectedRows().at(0).row();
    }

    float min = imgLayerList.at(pos)->getFits()->GetMin();
    if (min < 0)
        min = 0;
    float max = imgLayerList.at(pos)->getFits()->GetMax();

    lut->SetTableRange(min, max);

    imgLayerList.at(pos)->setLutScale(myscale);
    imgLayerList.at(pos)->setLutType(QString::fromStdString(palette));

    if (myscale == "Linear")
        lut->SetScaleToLinear();
    else if (myscale == "Log")
        lut->SetScaleToLog10();

    SelectLookTable(palette.c_str(), lut);

    vtkSmartPointer<vtkImageMapToColors> colors = vtkSmartPointer<vtkImageMapToColors>::New();

    colors->SetInputData(imgLayerList.at(pos)->getFits()->GetOutput());
    colors->SetLookupTable(lut);
    colors->Update();

    vtkSmartPointer<vtkImageSliceMapper> imageSliceMapperLutModified =
            vtkSmartPointer<vtkImageSliceMapper>::New();
    imageSliceMapperLutModified->SetInputData(colors->GetOutput());

    // vtkImageSlice::SafeDownCast( imageStack->GetImages()->GetItemAsObject(imgLayerList.at(pos
    // )->getLayerNumber()))->SetMapper(imageSliceMapperLutModified);
    vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(pos))
            ->SetMapper(imageSliceMapperLutModified);
    // vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(cb))->VisibilityOn();

    ui->qVTK1->update();
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::addSourcesFromBM(VSTableDesktop *m_VisIVOTable)
{

    QStringList wavelen;
    // wavelen<<"ft"<<"1100"<<"870"<<"500"<<"350"<<"250"<<"160"<<"70"<<"24"<<"22"<<"21";
    wavelen << "1100"
            << "870"
            << "500"
            << "350"
            << "250"
            << "160"
            << "70"
            << "24"
            << "22"
            << "21"
            << "ft";

    float semiMajorAxisLength;
    float semiMinorAxisLength;
    float angle;
    float ra;
    float dec;
    int numidtree;
    int numid_intree;
    QString sourceName;
    double *coord = new double[3];

    // QHash<QString, vtkEllipse* > ft_ellipse_list;

    //    int index_ft=0;
    //   int old_index_ft=0;

    for (int i = 0; i < wavelen.size(); i++) {

        QHash<QString, vtkEllipse *> ellipse_list_local;
        QList<QString> *list = new QList<QString>();

        for (unsigned long long j = 0; j < m_VisIVOTable->getNumberOfRows(); j++) {

            sourceName =
                    QString::fromStdString(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId(
                            "DESIGNATION" + wavelen[i].toStdString())][j]);
            if (sourceName.compare("missing") != 0 && list->indexOf(sourceName) == -1) {
                list->push_back(sourceName);
                // finchè marco non mette la tabella
                /*
                if(wavelen[i].compare("ft")==0)
                {

                    old_index_ft=index_ft;
                    index_ft=j;
                    qDebug()<<"\t index_ft: "<<index_ft <<" "<<old_index_ft ;
                    semiMajorAxisLength=maxSemiMajorAxisLength;
                    semiMinorAxisLength=maxSemiMinorAxisLength;
                    angle=maxAngle;

                }
                else
                {
                */
                semiMajorAxisLength = atof(m_VisIVOTable
                                                   ->getTableData()[m_VisIVOTable->getColId(
                                                           "FWHMA" + wavelen[i].toStdString())][j]
                                                   .c_str())
                        / 2;
                semiMinorAxisLength = atof(m_VisIVOTable
                                                   ->getTableData()[m_VisIVOTable->getColId(
                                                           "FWHMB" + wavelen[i].toStdString())][j]
                                                   .c_str())
                        / 2;
                angle = atof(m_VisIVOTable
                                     ->getTableData()[m_VisIVOTable->getColId(
                                             "PA" + wavelen[i].toStdString())][j]
                                     .c_str());
                //  M_PI*semiMajorAxisLength*semiMinorAxisLength;
                // qDebug()<<"\t\t index_ft: "<<index_ft <<" "<<old_index_ft ;

                /*
                    if (index_ft!= old_index_ft)
                    {
                        area=0;

                    }
                    if (M_PI*semiMajorAxisLength*semiMinorAxisLength>area)
                    {
                        maxSemiMajorAxisLength=semiMajorAxisLength;
                        maxSemiMinorAxisLength=semiMinorAxisLength;
                        maxAngle=angle;
                        area=M_PI*semiMajorAxisLength*semiMinorAxisLength;

                    }
*/

                // }
                ra = atof(m_VisIVOTable
                                  ->getTableData()[m_VisIVOTable->getColId(
                                          "GLON" + wavelen[i].toStdString())][j]
                                  .c_str());
                dec = atof(m_VisIVOTable
                                   ->getTableData()[m_VisIVOTable->getColId(
                                           "GLAT" + wavelen[i].toStdString())][j]
                                   .c_str());

                numidtree =
                        atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("numidtree")][j]
                                     .c_str());
                numid_intree = atof(
                        m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("numid_intree")][j]
                                .c_str());

                if (AstroUtils().sky2xy(filenameWithPath, ra, dec, coord)) {

                    qDebug() << "insert: " << sourceName;

                    if (wavelen[i].compare("ft") == 0) {
                        ft_ellipse_list.insert(
                                sourceName,
                                new vtkEllipse(semiMajorAxisLength, semiMinorAxisLength, angle,
                                               coord[0], coord[1], coord[2], 0, j, sourceName,
                                               m_VisIVOTable, numidtree, numid_intree));
                    } else
                        ellipse_list_local.insert(
                                sourceName,
                                new vtkEllipse(semiMajorAxisLength, semiMinorAxisLength, angle,
                                               coord[0], coord[1], coord[2], 0, j, sourceName,
                                               m_VisIVOTable, numidtree, numid_intree));
                }
            }
        }
        // drawEllipse(ellipse_list,  QString::fromStdString(m_VisIVOTable->getName()) );

        QString utype = stringDictWidget->getColUtypeStringDict().value(
                "compactsources.sed_view_final.designation" + wavelen[i]);
        if (utype.isEmpty()) {
            utype = wavelen[i];
        }

        qDebug() << "\t draw " << wavelen[i] << " - " << utype;

        auto ellipse_list = wavelen[i].compare("ft") == 0 ? ft_ellipse_list : ellipse_list_local;
        auto sourcePath = QString::fromStdString(m_VisIVOTable->getName());
        drawEllipse(ellipse_list, utype, sourcePath);

        /*
        if(wavelen[i].compare("ft")==0)
            drawEllipse(ft_ellipse_list,
        stringDictWidget->getColUtypeStringDict().value("vlkb_compactsources.sed_view_final.designation"+wavelen[i]),
        m_VisIVOTable->getName()); else drawEllipse(ellipse_list_local,
        stringDictWidget->getColUtypeStringDict().value("vlkb_compactsources.sed_view_final.designation"+wavelen[i]),
        m_VisIVOTable->getName());
        */
        /*

        if(wavelen[i].compare("ft")==0)
            drawEllipse(ft_ellipse_list,  wavelen[i] );
        else
            drawEllipse(ellipse_list_local,  wavelen[i] );
*/
    }

    file_wavelength.insert(QString::fromStdString(m_VisIVOTable->getName()),
                           m_VisIVOTable->getWavelength());

    // test window resize
    ui->qVTK1->update();
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
    this->update();
    sessionModified();
}

void vtkwindow_new::createInfoWindow()
{

    info = new FitsImageStatisiticInfo(this);
}

void vtkwindow_new::setSelectedActor(vtkSmartPointer<vtkActor> sel)
{
    selectedActor = sel;
}

void vtkwindow_new::setSliceDatacube(int i)
{

    vtkSmartPointer<vtkLookupTable> lutSlice = vtkSmartPointer<vtkLookupTable>::New();
    lutSlice->SetTableRange(myfits->GetRangeSlice(i)[0], myfits->GetRangeSlice(i)[1]);
    SelectLookTable("Gray", lutSlice);
    viewer->GetWindowLevel()->SetOutputFormatToRGB();
    viewer->GetWindowLevel()->SetLookupTable(lutSlice);
    viewer->GetImageActor()->InterpolateOff();
    viewer->SetSlice(i);

    if (ui->contourCheckBox->isChecked())
        goContour();

    ui->minSliceLineEdit->setText(QString::number(myfits->GetRangeSlice(i)[0]));
    ui->maxSliceLineEdit->setText(QString::number(myfits->GetRangeSlice(i)[1]));

    ui->isocontourVtkWin->update();
    viewer->GetRenderer()->ResetCamera();
    viewer->Render();
}

void vtkwindow_new::changeFitsPalette(std::string palette)
{

    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->SetTableRange(myfits->GetMin(), myfits->GetMax());

    SelectLookTable(palette.c_str(), lut);

    imageViewer->GetWindowLevel()->SetLookupTable(lut);

    ui->qVTK1->update();
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::drawSingleEllipse(vtkSmartPointer<vtkLODActor> ellipseActor)
{
    ellipseActor->GetProperty()->SetLighting(true);
    ellipseActor->GetProperty()->SetLineWidth(3);
    m_Ren1->AddActor(ellipseActor);

    ui->qVTK1->update();
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::removeSingleEllipse(vtkSmartPointer<vtkLODActor> ellipseActor)
{
    m_Ren1->RemoveActor(ellipseActor);
    ui->qVTK1->update();
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::loadObservedObject(VisPoint *vis)
{
    setWindowTitle(vis->getName());

    vispoint = vis;
    table = vispoint->getOrigin();
    QString workingPath = QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp");
    QString OutputFilenamePoints = workingPath;
    QString OutputFilenameImage = workingPath;
    OutputFilenamePoints.append(QDir::separator()).append(table->getName().c_str());
    OutputFilenameImage.append(QDir::separator()).append(table->getName().c_str());

    pp = new PointsPipe(table);

    QStringList fieldList;
    fieldList << vis->getX() << vis->getY() << vis->getZ();

    pp->setVtkWindow(this);

    pp->createPipeFor3dSelection();

    showMaximized();
    activateWindow();
}

QHash<QString, vtkSmartPointer<vtkLODActor>> vtkwindow_new::getVisualizedActorList()
{
    return visualized_actor_list;
}

QHash<QString, vtkSmartPointer<vtkLODActor>> vtkwindow_new::getEllipseActorList()
{
    return ellipse_actor_list;
}

QStringList vtkwindow_new::getSourcesLoadedFromFile(const QString &sourcePath)
{
    QStringList list;
    foreach (const auto &it, elementLayerList) {
        if (it->getType() == 1 && it->getPath() == sourcePath)
            list << it->getName();
    }
    return list;
}

void vtkwindow_new::setCuttingPlaneValue(int arg1)
{
    /*
    sliceA->SetPosition(0,0,arg1);
    ui->qVTK1->update();
    if(!isDatacube)
        scale(scaleActivate);
    else
        this->updateScene();
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
    */
    ui->cuttingPlane_Slider->setValue(arg1);
}

void vtkwindow_new::on_actionInfo_triggered()
{
    if (fitsViewer) {

        info->setFilename();
        info->show();
    }
}

void vtkwindow_new::on_actionTools_triggered()
{
    if (fitsViewer) {

        vtkfitstoolsw->show();
        vtkfitstoolsw->activateWindow();

    } else {
        vtktoolswidget *vtktoolsw = new vtktoolswidget(this);
        vtktoolsw->show();
    }
}

void vtkwindow_new::on_resetPushButton_clicked()
{
    setVtkInteractorStyle3DPicker(pp->getPolyData());
    ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(selectedActor);
}

void vtkwindow_new::on_confirmPushButton_clicked()
{
    setVtkInteractorStyle3DFreehand(pp->getPolyData());
}

void vtkwindow_new::on_rectSelection_clicked()
{
    setVtkInteractorStyle3DPicker(pp->getPolyData());
}

void vtkwindow_new::on_freehandPushButton_clicked()
{
    setVtkInteractorStyle3DFreehand(pp->getPolyData());
}

void vtkwindow_new::setVtkInteractorStyleImageContour()
{
    /*
    Left Mouse button triggers window level events
    CTRL Left Mouse spins the camera around its view plane normal
    SHIFT Left Mouse pans the camera
    CTRL SHIFT Left Mouse dollys (a positional zoom) the camera
    Middle mouse button pans the camera
    Right mouse button dollys the camera.
    SHIFT Right Mouse triggers pick events
    */

    vtkSmartPointer<myVtkInteractorStyleImage> style =
            vtkSmartPointer<myVtkInteractorStyleImage>::New();

    ui->isocontourVtkWin->renderWindow()->GetInteractor()->SetInteractorStyle(style);
    ui->isocontourVtkWin->renderWindow()->GetInteractor()->SetRenderWindow(
            ui->isocontourVtkWin->renderWindow());
    style->setVtkWin(this);
    style->setIsSlice();
    ui->isocontourVtkWin->setCursor(Qt::ArrowCursor);
}

void vtkwindow_new::setVtkInteractorStyleImage()
{
    /*
    Left Mouse button triggers window level events
    CTRL Left Mouse spins the camera around its view plane normal
    SHIFT Left Mouse pans the camera
    CTRL SHIFT Left Mouse dollys (a positional zoom) the camera
    Middle mouse button pans the camera
    Right mouse button dollys the camera.
    SHIFT Right Mouse triggers pick events
    */

    vtkSmartPointer<myVtkInteractorStyleImage> style =
            vtkSmartPointer<myVtkInteractorStyleImage>::New();

    ui->qVTK1->renderWindow()->GetInteractor()->SetInteractorStyle(style);
    ui->qVTK1->renderWindow()->GetInteractor()->SetRenderWindow(ui->qVTK1->renderWindow());
    ui->rectangularSelectionCS->setStyleSheet("");

    style->setVtkWin(this);
    qDebug() << "Cursor is " << ui->qVTK1->cursor();
    if (ui->qVTK1->cursor() != Qt::ArrowCursor)
        ui->qVTK1->setCursor(ui->qVTK1->defaultCursor());
    qDebug() << "Cursor is " << ui->qVTK1->cursor();
}

void vtkwindow_new::setSkyRegionSelectorInteractorStyleFor3D()
{

    vtkSmartPointer<SkyRegionSelector> style = vtkSmartPointer<SkyRegionSelector>::New();
    style->setVtkWin(this);
    style->setIs3D();
    ui->qVTK1->renderWindow()->GetInteractor()->SetInteractorStyle(style);
    ui->qVTK1->setCursor(Qt::CrossCursor);
}

void vtkwindow_new::setSkyRegionSelectorInteractorStyle()
{

    ui->rectangularSelectionCS->setStyleSheet(
            "background-color: rgb(0, 0, 250);border-radius: 3px; border-width: 1px;width:100%;");
    //    ui->rectangularSelectionCS->setStyleSheet("");

    ui->bubblePushButton->setStyleSheet("");
    ui->fil_rectPushButton->setStyleSheet("");
    ui->tdRectPushButton->setStyleSheet("");

    // on_actionTools_triggered();

    vtkSmartPointer<SkyRegionSelector> style = vtkSmartPointer<SkyRegionSelector>::New();
    style->setVtkWin(this);
    ui->qVTK1->renderWindow()->GetInteractor()->SetInteractorStyle(style);
    ui->qVTK1->setCursor(Qt::CrossCursor);
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::on_PVPlotPushButton_clicked()
{
    setVtkInteractorContourWindow();
}

void vtkwindow_new::on_PVPlot_radioButton_clicked(bool checked)
{
    if (checked == true)
        setVtkInteractorContourWindow();
    else
        setVtkInteractorStyleImage();
}

void vtkwindow_new::on_contour_pushButton_clicked()
{
    // if (contourWin==0)
    // contourWinActivated=true;
    // ui->isocontourVtkWin->
    contourWin->setFitsReader(myfits, this);
    contourWin->show();
}

void vtkwindow_new::setVtkInteractorContourWindow()
{
    /*
     Left Mouse button triggers window level events
     SHIFT Right Mouse triggers pick events
     */

    vtkSmartPointer<myVtkInteractorContourWindow> style =
            vtkSmartPointer<myVtkInteractorContourWindow>::New();

    // style->AutoAdjustCameraClippingRangeOn();
    ui->qVTK1->renderWindow()->GetInteractor()->SetInteractorStyle(style);
    ui->qVTK1->renderWindow()->GetInteractor()->SetRenderWindow(ui->qVTK1->renderWindow());
    style->setVtkWin(this);
    ui->qVTK1->setCursor(Qt::ArrowCursor);
}

void vtkwindow_new::setVtkInteractorStyle3DPicker(vtkSmartPointer<vtkPolyData> points)
{
    /*
    vtkSmartPointer<InteractorStyle> style =zaz vtkSmartPointer<InteractorStyle>::New();
    style->SetPoints(input);
    renderWindowInteractor->SetInteractorStyle( style );
*/

    vtkSmartPointer<vtkAreaPicker> areaPicker = vtkSmartPointer<vtkAreaPicker>::New();
    ui->qVTK1->renderWindow()->GetInteractor()->SetPicker(areaPicker);

    vtkSmartPointer<InteractorStyleSelctionPointOn3DVisualization> style =
            vtkSmartPointer<InteractorStyleSelctionPointOn3DVisualization>::New();
    style->setVtkWin(this);
    style->SetPoints(points);

    ui->qVTK1->renderWindow()->GetInteractor()->SetInteractorStyle(style);

    update();
}

void vtkwindow_new::setVtkInteractorStyleFreehand()
{

    vtkSmartPointer<vtkInteractorStyleDrawPolygon> style =
            vtkSmartPointer<vtkInteractorStyleDrawPolygon>::New();

    ui->qVTK1->renderWindow()->GetInteractor()->SetInteractorStyle(style);
    ui->qVTK1->renderWindow()->GetInteractor()->SetRenderWindow(ui->qVTK1->renderWindow());
    // style->setVtkWin(this);
    //  ui->qVTK1->setCursor(Qt::ArrowCursor);
}

void vtkwindow_new::extractSourcesInsideRect(int *rect)
{
    setVtkInteractorStyleImage();

    if (!catalogue || catalogue->getSources().isEmpty()) {
        return;
    }

    double X0 = (rect[0] < rect[2]) ? rect[0] : rect[2];
    double Y0 = (rect[1] < rect[3]) ? rect[1] : rect[3];
    double X1 = (rect[0] > rect[2]) ? rect[0] : rect[2];
    double Y1 = (rect[1] > rect[3]) ? rect[1] : rect[3];

    if (X0 == X1) {
        X1 += 1.0;
    }
    if (Y0 == Y1) {
        Y1 += 1.0;
    }

    auto renderer = ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer();
    double verts[32];
    renderer->SetDisplayPoint(X0, Y0, 0);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(&verts[0]);

    renderer->SetDisplayPoint(X0, Y0, 1);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(&verts[4]);

    renderer->SetDisplayPoint(X0, Y1, 0);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(&verts[8]);

    renderer->SetDisplayPoint(X0, Y1, 1);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(&verts[12]);

    renderer->SetDisplayPoint(X1, Y0, 0);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(&verts[16]);

    renderer->SetDisplayPoint(X1, Y0, 1);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(&verts[20]);

    renderer->SetDisplayPoint(X1, Y1, 0);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(&verts[24]);

    renderer->SetDisplayPoint(X1, Y1, 1);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(&verts[28]);

    auto frustumExtractor = vtkSmartPointer<vtkExtractSelectedFrustum>::New();
    frustumExtractor->CreateFrustum(verts);

    auto extractPolyData = vtkSmartPointer<vtkExtractPolyDataGeometry>::New();
    extractPolyData->SetInputConnection(catalogue->GetPolyDataFilter()->GetOutputPort());
    extractPolyData->SetImplicitFunction(frustumExtractor->GetFrustum());
    extractPolyData->ExtractInsideOn();
    extractPolyData->PassPointsOn();

    auto removePolyData = vtkSmartPointer<vtkRemovePolyData>::New();
    removePolyData->AddInputConnection(catalogue->GetPolyDataFilter()->GetOutputPort());
    removePolyData->AddInputConnection(extractPolyData->GetOutputPort());
    catalogue->SetPolyDataFilter(removePolyData);

    auto masterMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    masterMapper->SetInputConnection(removePolyData->GetOutputPort());

    auto masterActor = catalogue->GetActor();
    masterActor->GetProperty()->SetColor(1, 0, 0);
    masterActor->SetMapper(masterMapper);

    auto coordinate = vtkSmartPointer<vtkCoordinate>::New();
    coordinate->SetCoordinateSystemToDisplay();
    coordinate->SetValue(X0, Y0);
    double *p1 = coordinate->GetComputedWorldValue(renderer);
    coordinate = vtkSmartPointer<vtkCoordinate>::New();
    coordinate->SetCoordinateSystemToDisplay();
    coordinate->SetValue(X1, Y1);
    double *p2 = coordinate->GetComputedWorldValue(renderer);
    double rectInWorldCoords[] = { p1[0], p1[1], p2[0], p2[1] };
    catalogue->ExtractSourceInsideRect(rectInWorldCoords,
                                       AstroUtils::arcsecPixel(myfits->GetFileName()));

    showSourceDockWidget();

    ui->qVTK1->renderWindow()->Render();
}

void vtkwindow_new::setVtkInteractorExtractSources()
{
    auto style = vtkSmartPointer<InteractorStyleExtractSources>::New();
    style->Callback = [this](int *rect) -> void { extractSourcesInsideRect(rect); };

    ui->qVTK1->renderWindow()->GetInteractor()->SetInteractorStyle(style);
    ui->qVTK1->renderWindow()->GetInteractor()->SetRenderWindow(ui->qVTK1->renderWindow());
    ui->qVTK1->setCursor(Qt::CrossCursor);
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::setVtkInteractorEditSource(
        const QPair<vtkSmartPointer<vtkPoints>, vtkSmartPointer<vtkLODActor>> &source)
{
    auto style = vtkSmartPointer<InteractorStyleEditSource>::New();
    style->SetRenderWindow(ui->qVTK1->renderWindow());
    style->SetSource(source.first, source.second);

    ui->qVTK1->renderWindow()->GetInteractor()->SetInteractorStyle(style);
    ui->qVTK1->renderWindow()->GetInteractor()->SetRenderWindow(ui->qVTK1->renderWindow());
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::on_spinBox_contour_valueChanged(int arg1)
{

    setSliceDatacube(arg1 - 1);
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::setVtkInteractorStyle3DFreehand(vtkSmartPointer<vtkPolyData> points)
{

    vtkSmartPointer<vtkCellPicker> areaPicker = vtkSmartPointer<vtkCellPicker>::New();
    ui->qVTK1->renderWindow()->GetInteractor()->SetPicker(areaPicker);

    vtkSmartPointer<InteractorStyleFreeHandOn3DVisualization> style =
            vtkSmartPointer<InteractorStyleFreeHandOn3DVisualization>::New();
    style->setVtkWin(this);
    style->SetPoints(points);

    ui->qVTK1->renderWindow()->GetInteractor()->SetInteractorStyle(style);

    update();
}

void vtkwindow_new::on_cuttingPlane_Slider_sliderMoved(int position) { }

void vtkwindow_new::setSelectionFitsViewerInteractorStyle()
{

    vtkSmartPointer<MyRubberBand> style = vtkSmartPointer<MyRubberBand>::New();

    style->setVtkWin(this);

    ui->qVTK1->renderWindow()->GetInteractor()->SetInteractorStyle(style);
    ui->qVTK1->setCursor(Qt::CrossCursor);

    // update();
}

void vtkwindow_new::on_horizontalSlider_threshold_valueChanged(int value) { }

void vtkwindow_new::showBox(bool checked)
{
    if (checked)
        pp->outlineActor->SetVisibility(true);
    else
        pp->outlineActor->SetVisibility(false);

    ui->qVTK1->update();
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::addActor(vtkProp *actor)
{
    // back_ren->AddActor2D(actor);
    ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer()->AddActor2D(actor);
    ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer()->GetActors2D();

    ui->qVTK1->renderWindow()->Render();
}

void vtkwindow_new::showAxes(bool checked)
{
    if (checked)
        pp->axesActor->SetVisibility(true);
    else
        pp->axesActor->SetVisibility(false);

    ui->qVTK1->update();
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::showGrid(bool checked)
{
    pp->activateGrid(checked);
}

void vtkwindow_new::changeWCS(bool galaptic)
{

    if (galaptic) {
        vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
        transform->RotateY(180);
        vtkAxes->SetUserTransform(transform);
    } else {
        vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
        transform->RotateY(-180);
        transform->RotateZ(-23.5);
        vtkAxes->SetUserTransform(transform);
    }
    update();
}

void vtkwindow_new::plotSlice(vtkSmartPointer<vtkFitsReader> visvis, int arg1) { }

void vtkwindow_new::on_rectangularSelectionCS_clicked()
{
    setSkyRegionSelectorInteractorStyle();
}

void vtkwindow_new::on_colorPushButton_clicked()
{

    vtkfitstoolwindow->show();
}

void vtkwindow_new::addLayer(vtkfitstoolwidgetobject *o, bool enabled)
{
    QList<QTableWidgetItem *> elements = ui->ElementListWidget->selectedItems();

    qDebug() << "tableWidget " << o->getName();
    if (elements.size() > 3 && o->getSurvey().compare("") == 0) {
        o->setSurvey(elements.at(0)->text());
        o->setSpecies(elements.at(1)->text());
        o->setTransition(elements.at(2)->text());
    }

    if (o->getType() == 0) {

        addImageToList(o);
    } else if (o->getType() == 1) {
        addToList(o, enabled);
    } else if (o->getType() == 2) {
        addToList(o, enabled);
    } else if (o->getType() == 3) {
        addToList(o, enabled);
    }

    // ui->tableWidget->horizontalHeader()->sectionResizeMode(QHeaderView::Stretch);

    ui->tableWidget->resizeColumnsToContents();
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

QTreeWidgetItem *vtkwindow_new::addTreeRoot(QString name)
{
    QTreeWidgetItem *topLevel = new QTreeWidgetItem();

    return topLevel;
}

void vtkwindow_new::addTreeChild(QTreeWidgetItem *parent, QString name, QBrush brush)
{

    QTreeWidgetItem *treeItem = new QTreeWidgetItem();

    treeItem->setBackground(0, brush);
    treeItem->setText(1, name);
    // mod
    treeItem->setFlags(treeItem->flags() | Qt::ItemIsUserCheckable);
    treeItem->setCheckState(0, Qt::Checked);

    // end
    parent->addChild(treeItem);
}

void vtkwindow_new::addImageToList(vtkfitstoolwidgetobject *o)
{
    imgLayerList.append(o);
    o->setLayerNumber(imageStack->GetImages()->GetNumberOfItems() - 1);

    QString text;
    if ((o->getSurvey() == "") && (o->getSpecies() == "") && (o->getTransition() == "")) {
        // We are loading a local file, use the filename
        text = o->getName();
        if (o->getFits()->isMoment3D) {
            text += "_mom" + QString::number(o->getFits()->getMomentOrder());
        }
    } else {
        text = o->getSurvey() + "_" + o->getSpecies() + "_" + o->getTransition();
    }

    QListWidgetItem *item = new QListWidgetItem(text, ui->listWidget);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
    item->setCheckState(Qt::Checked); // AND initialize check state

    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::showSourceDockWidget()
{
    if (!catalogue) {
        return;
    }

    if (!dock) {
        dock = new QDockWidget(this);
        dock->setAllowedAreas(Qt::LeftDockWidgetArea);
        dock->setAttribute(Qt::WA_DeleteOnClose);
        dock->setFloating(true);

        auto sourceWidget = new SourceWidget(dock, catalogue, this);
        dock->setWidget(sourceWidget);
        dock->show();
    }
}

// QUI FV - Aggiunta livelli per sorgenti

void vtkwindow_new::addToList(vtkfitstoolwidgetobject *o, bool enabled)
{
    elementLayerList.append(o);

    QSignalMapper *mapper = new QSignalMapper(this);
    QSignalMapper *mapper_slider = new QSignalMapper(this);

    int row = ui->tableWidget->model()->rowCount();
    ui->tableWidget->insertRow(row);

    QCheckBox *cb1 = new QCheckBox();

    if (enabled)
        cb1->setChecked(true);
    else
        cb1->setChecked(false);

    if (o->getType() != 3) {
        double r = o->getActor()->GetProperty()->GetColor()[0] * 255;
        double g = o->getActor()->GetProperty()->GetColor()[1] * 255;
        double b = o->getActor()->GetProperty()->GetColor()[2] * 255;

        cb1->setStyleSheet("background-color: rgb(" + QString::number(r) + "," + QString::number(g)
                           + " ," + QString::number(b) + ")");

        /*
        //FV MODIFICARE PER AGGIUNGERE COLORE E GESTIONE CHECKBOX
        QListWidgetItem* item = new QListWidgetItem(o->getName());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        item->setCheckState(Qt::Checked); // AND initialize check state
        QColor c1(r,g,b);
        QBrush b1(c1);
        item->setForeground(b1);
        ui->listWidget->addItem(item);

*/
    }
    ui->tableWidget->setCellWidget(row, 0, cb1);

    connect(cb1, SIGNAL(stateChanged(int)), mapper, SLOT(map()));
    mapper->setMapping(cb1, row);

    QTableWidgetItem *item_1 = new QTableWidgetItem();
    item_1->setFlags(item_1->flags() ^ Qt::ItemIsEditable);

    item_1->setText(o->getName());

    ui->tableWidget->setItem(row, 1, item_1);

    connect(mapper, SIGNAL(mapped(int)), this, SLOT(checkboxClicked(int)));
}

void vtkwindow_new::checkboxImageClicked(int cb, bool status)
{
    auto image = vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(cb));
    if (!image)
        return;

    if (status)
        image->VisibilityOn();
    else
        image->VisibilityOff();

    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::checkboxClicked(int cb, bool status)
{

    /*
    if (!status)
        getVisualizedActorList().value(ui->listWidget->item(cb)->text())->VisibilityOff();
    else
        getVisualizedActorList().value(ui->listWidget->item(cb)->text())->VisibilityOn();

    ui->qVTK1->update();
*/

    if (getVisualizedActorList().value(ui->tableWidget->item(cb, 1)->text())->GetVisibility())
        getVisualizedActorList().value(ui->tableWidget->item(cb, 1)->text())->VisibilityOff();
    else
        getVisualizedActorList().value(ui->tableWidget->item(cb, 1)->text())->VisibilityOn();

    ui->qVTK1->renderWindow()->GetInteractor()->Render();
    sessionModified();
}

void vtkwindow_new::on_tableWidget_doubleClicked(const QModelIndex &index)
{
    if (elementLayerList.at(index.row())->getType() != 0) {
        // Initial color
        double r = getVisualizedActorList()
                           .value(ui->tableWidget->item(index.row(), 1)->text())
                           ->GetProperty()
                           ->GetColor()[0]
                * 255;
        double g = getVisualizedActorList()
                           .value(ui->tableWidget->item(index.row(), 1)->text())
                           ->GetProperty()
                           ->GetColor()[1]
                * 255;
        double b = getVisualizedActorList()
                           .value(ui->tableWidget->item(index.row(), 1)->text())
                           ->GetProperty()
                           ->GetColor()[2]
                * 255;

        QColor color = QColorDialog::getColor(QColor(r, g, b), this);
        if (color.isValid()) {
            // Update actor color
            getVisualizedActorList()
                    .value(ui->tableWidget->item(index.row(), 1)->text())
                    ->GetProperty()
                    ->SetColor(color.redF(), color.greenF(), color.blueF());
            ui->qVTK1->update();
            ui->qVTK1->renderWindow()->GetInteractor()->Render();

            // Update color on table
            ui->tableWidget->cellWidget(index.row(), 0)
                    ->setStyleSheet("background-color: rgb(" + QString::number(color.redF() * 255)
                                    + "," + QString::number(color.greenF() * 255) + " ,"
                                    + QString::number(color.blueF() * 255) + ")");
            sessionModified();
        }
    }
}

void vtkwindow_new::on_fil_rectPushButton_clicked()
{
    ui->fil_rectPushButton->setStyleSheet(
            "background-color: rgb(0, 0, 250);border-radius: 3px; border-width: 1px;");
    ui->rectangularSelectionCS->setStyleSheet("");
    ui->tdRectPushButton->setStyleSheet("");
    ui->bubblePushButton->setStyleSheet("");

    vtkSmartPointer<SkyRegionSelector> style = vtkSmartPointer<SkyRegionSelector>::New();
    style->setVtkWin(this);
    style->setIsFilament();
    ui->qVTK1->renderWindow()->GetInteractor()->SetInteractorStyle(style);
    ui->qVTK1->setCursor(Qt::CrossCursor);
}

void vtkwindow_new::setDbElements(QList<QMap<QString, QString>> elementsOnDb)
{

    classElementsOnDb = elementsOnDb;

    int i = 0;
    while (!elementsOnDb.isEmpty()) {

        QMap<QString, QString> datacube = elementsOnDb.takeFirst();

        qDebug() << datacube << "\n\n\n";

        // datacubes_list->append(datacube);

        ui->ElementListWidget->insertRow(i);
        QTableWidgetItem *item_0 = new QTableWidgetItem();
        item_0->setFlags(item_0->flags() ^ Qt::ItemIsEditable);
        item_0->setText(datacube["Survey"] + "\n" + datacube["Species"]);
        ui->ElementListWidget->setItem(i, 0, item_0);

        QTableWidgetItem *item_1 = new QTableWidgetItem();
        item_1->setFlags(item_1->flags() ^ Qt::ItemIsEditable);
        QString codeString = "";
        switch (datacube["code"].toInt()) {
        case 2:
            codeString = "The Region is completely inside the input";
            break;
        case 3:
            codeString = "Full Overlap";
            break;
        case 4:
            codeString = "Partial Overlap";
            break;
        case 5:
            codeString = "The Regions are identical ";
            break;
        default:
            codeString = "Merge";
            break;
        }

        item_1->setText(datacube["Transition"] + "\n" + codeString);

        //   item_1->setText(datacube["Species"]);
        ui->ElementListWidget->setItem(i, 1, item_1);

        /*   QTableWidgetItem *item_2 = new QTableWidgetItem();
           item_2->setFlags(item_2->flags() ^ Qt::ItemIsEditable);
          // item_2->setText(datacube["Transition"]);
           ui->ElementListWidget->setItem(i,2,item_2);

           QTableWidgetItem *item_3 = new QTableWidgetItem();
           item_3->setFlags(item_3->flags() ^ Qt::ItemIsEditable);
           item_3->setText(datacube["code"]);
           ui->ElementListWidget->setItem(i,3,item_3);
   */
        if (datacube["code"].toDouble() == 3) {
            ui->ElementListWidget->item(i, 0)->setBackground(Qt::green);
            ui->ElementListWidget->item(i, 1)->setBackground(Qt::green);
            //     ui->ElementListWidget->item(i, 2)->setBackground(Qt::green);
            //    ui->ElementListWidget->item(i, 3)->setBackground(Qt::green);
        }

        item_0->setToolTip(datacube["Description"]);
        item_1->setToolTip(datacube["Description"]);
        //  item_2->setToolTip(datacube["Description"]);
        //  item_3->setToolTip(datacube["Description"]);
        i++;
    }

    ui->ElementListWidget->setWordWrap(true);

    ui->ElementListWidget->setTextElideMode(Qt::ElideMiddle);

    ui->ElementListWidget->resizeColumnsToContents();
    ui->ElementListWidget->resizeRowsToContents();
}

void vtkwindow_new::addLayerImage(vtkSmartPointer<vtkFitsReader> vis, QString survey,
                                  QString species, QString transition)
{
    vtkSmartPointer<vtkImageShiftScale> resultScale = vtkSmartPointer<vtkImageShiftScale>::New();
    resultScale->SetOutputScalarTypeToUnsignedChar();
    resultScale->SetInputData(vis->GetOutput());

    resultScale->Update();

    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();

    double min = vis->GetMin();
    double max = vis->GetMax();

    if (min < 0) {
        min = 0;
    }

    lut->SetTableRange(min, max);
    lut->SetScaleToLog10();

    SelectLookTable("Gray", lut);
    imageObject->setLutScale("Log");
    imageObject->setLutType("Gray");

    vtkfitstoolwidgetobject *img = new vtkfitstoolwidgetobject(0);

    qDebug() << "AGGIUNG_" << QString::fromUtf8(vis->GetFileName().c_str()) << " s " << survey
             << " sp " << species << " t " << transition;
    img->setName(QString::fromUtf8(vis->GetFileName().c_str()));
    img->setFitsReader(vis);
    if (survey.compare("") != 0 && species.compare("") != 0 && transition.compare("") != 0) {
        img->setSpecies(species);
        img->setTransition(transition);
        img->setSurvey(survey);
    }

    double scaledPixel = AstroUtils().arcsecPixel(vis->GetFileName())
            / AstroUtils().arcsecPixel(myfits->GetFileName());
    vis->GetOutput()->SetSpacing(scaledPixel, scaledPixel, 1);

    double *sky_coord_gal = new double[2];
    AstroUtils().xy2sky(vis->GetFileName(), 0, 0, sky_coord_gal, 3);
    qDebug() << "0.0 xy2sky " << sky_coord_gal[0] << sky_coord_gal[1];

    double *coord = new double[3];
    AstroUtils().sky2xy(myfits->GetFileName(), sky_coord_gal[0], sky_coord_gal[1], coord);
    vis->GetOutput()->SetOrigin(coord[0], coord[1], 0);
    qDebug() << "0.0 sky2xy " << coord[0] << coord[1];

    vtkSmartPointer<vtkImageMapToColors> colors = vtkSmartPointer<vtkImageMapToColors>::New();
    colors->SetInputData(vis->GetOutput());
    colors->SetLookupTable(lut);
    colors->Update();

    vtkSmartPointer<vtkImageSliceMapper> imageSliceMapperLayer =
            vtkSmartPointer<vtkImageSliceMapper>::New();
    imageSliceMapperLayer->SetInputData(colors->GetOutput());

    vtkSmartPointer<vtkImageSlice> imageSliceLayer = vtkSmartPointer<vtkImageSlice>::New();
    imageSliceLayer->SetMapper(imageSliceMapperLayer);
    imageSliceLayer->GetProperty()->SetOpacity(0.5);
    imageSliceLayer->GetProperty()->SetInterpolationTypeToNearest();

    double angle = 0;

    double x1 = coord[0];
    double y1 = coord[1];

    AstroUtils().xy2sky(vis->GetFileName(), 0, 100, sky_coord_gal, 3);
    AstroUtils().sky2xy(myfits->GetFileName(), sky_coord_gal[0], sky_coord_gal[1], coord);

    qDebug() << "0.100 xy2sky " << sky_coord_gal[0] << sky_coord_gal[1];
    qDebug() << "0.100 sky2xy " << coord[0] << coord[1];

    if (x1 != coord[0]) {
        double m = fabs((coord[1] - y1) / (coord[0] - x1));
        angle = 1 * (90 - atan(m) * 180 / M_PI);
        qDebug() << "m=" << m << " angle " << angle;
    }

    double bounds[6];

    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    vis->GetOutput()->GetBounds(bounds);

    // Rotate about the origin point (world coordinates)
    transform->Translate(bounds[0], bounds[2], bounds[4]);
    transform->RotateWXYZ(angle, 0, 0, 1);
    transform->Translate(-bounds[0], -bounds[2], -bounds[4]);
    imageSliceLayer->SetUserTransform(transform);

    imageSliceLayer->GetProperty()->SetLayerNumber(imageStack->GetImages()->GetNumberOfItems());

    imageStack->AddImage(imageSliceLayer);

    addLayer(img);

    if (rectangleActor != 0) {
        m_Ren1->RemoveActor(rectangleActor);
    }

    this->update();
    activateWindow();
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
    sessionModified();
}

void vtkwindow_new::downloadStartingLayers(QList<QPair<QString, QString>> selectedSurvey)
{

    for (int i = 1; i < selectedSurvey.size(); i++) {
        qDebug() << selectedSurvey.at(i).first << " " << selectedSurvey.at(i).second;
        VialacteaInitialQuery *vq = new VialacteaInitialQuery();
        vq->setCallingVtkWindow(this);

        vq->setL(called_l);
        vq->setB(called_b);

        QString sn = selectedSurvey.at(i).first;

        qDebug() << "SN: " << sn;
        if (sn.compare("Hi-GAL") == 0) {
            // http://ia2-vialactea.oats.inaf.it:8080/libjnifitsdb-1.0.2/vlkb_search?surveyname=Hi-GAL%20Tiles%22%20OR%20name%20=%20%22Hi-GAL%20Mosaic&l=40&b=0&species=Continuum&transition=250%20um&r=0.3&vl=0&vu=0

            sn = "Hi-GAL Tiles\" OR name = \"Hi-GAL Mosaic";
        }
        qDebug() << "Richiedo Layer";

        QString urlString = vlkbUrl + "/vlkb_search?surveyname=" + QUrl::toPercentEncoding(sn)
                + "&l=" + called_l + "&b=" + called_b + "&species=Continuum&transition="
                + QUrl::toPercentEncoding(selectedSurvey.at(i).second);
        if (called_dl != "" && called_db != "") {
            urlString += "&dl=" + called_dl + "&db=" + called_db;
            vq->setDeltaRect(called_dl, called_db);

        } else {
            urlString += "&r=" + called_r;
            vq->setR(called_r);
        }
        urlString += "&vl=0&vu=0";

        qDebug() << urlString;
        QUrl url(urlString);

        vq->setTransition(selectedSurvey.at(i).second);
        vq->setSpecies("Continuum");
        vq->setSurveyname(selectedSurvey.at(i).first);

        vq->selectedStartingLayersRequest(url);

        this->activateWindow();
    }
}

void vtkwindow_new::handleButton(int i)
{
    QMap<QString, QString> datacube = classElementsOnDb[i];

    qDebug() << datacube;

    QString url_string = datacube["URL"];

    QUrl url(url_string);

    VialacteaInitialQuery *vq = new VialacteaInitialQuery();
    vq->setCallingVtkWindow(this);
    vq->cutoutRequest(url_string, classElementsOnDb, i);
    this->activateWindow();
}

void vtkwindow_new::on_lutComboBox_activated(const QString &arg1)
{
    changeFitsScale(ui->lutComboBox->currentText().toStdString().c_str(),
                    selected_scale.toStdString().c_str());
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
    sessionModified();
}

void vtkwindow_new::on_logRadioButton_toggled(bool checked)
{
    if (checked)
        selected_scale = "Log";
    else
        selected_scale = "Linear";

    changeFitsScale(ui->lutComboBox->currentText().toStdString().c_str(),
                    selected_scale.toStdString().c_str());
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
    sessionModified();
}

void vtkwindow_new::on_tdRectPushButton_clicked()
{
    ui->tdRectPushButton->setStyleSheet(
            "background-color: rgb(0, 0, 250);border-radius: 3px; border-width: 1px;");
    ui->fil_rectPushButton->setStyleSheet("");
    ui->rectangularSelectionCS->setStyleSheet("");
    ui->bubblePushButton->setStyleSheet("");

    vtkSmartPointer<SkyRegionSelector> style = vtkSmartPointer<SkyRegionSelector>::New();
    style->setVtkWin(this);
    style->setIs3dSelections();
    ui->qVTK1->renderWindow()->GetInteractor()->SetInteractorStyle(style);
    ui->qVTK1->setCursor(Qt::CrossCursor);
}

void vtkwindow_new::on_ElementListWidget_doubleClicked(const QModelIndex &index)
{
    handleButton(index.row());
}

void vtkwindow_new::on_thresholdValueLineEdit_editingFinished()
{
    shellE->SetValue(0, ui->thresholdValueLineEdit->text().toFloat());
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
    if (myParentVtkWindow != 0)
        myParentVtkWindow->sessionModified();
}

void vtkwindow_new::on_upperBoundLineEdit_editingFinished()
{
    if (ui->contourCheckBox->isChecked())
        goContour();
}

void vtkwindow_new::on_lowerBoundLineEdit_editingFinished()
{
    if (ui->contourCheckBox->isChecked())
        goContour();
}

bool vtkwindow_new::getContoursInfo(int &level, double &lowerBound, double &upperBound)
{
    level = ui->levelLineEdit->text().toInt();
    lowerBound = ui->lowerBoundLineEdit->text().toDouble();
    upperBound = ui->upperBoundLineEdit->text().toDouble();
    return ui->contourCheckBox->isChecked();
}

void vtkwindow_new::setContoursInfo(const int &level, const double &lowerBound,
                                    const double &upperBound, const bool &enabled)
{
    ui->levelLineEdit->setText(QString::number(level));
    ui->lowerBoundLineEdit->setText(QString::number(lowerBound, 'f', 4));
    ui->upperBoundLineEdit->setText(QString::number(upperBound, 'f', 4));
    if (enabled)
        goContour();
}

void vtkwindow_new::goContour()
{
    removeContour();

    vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(0, 0, viewer->GetSlice());
    plane->SetNormal(0, 0, 1);

    vtkSmartPointer<vtkCutter> cutter = vtkSmartPointer<vtkCutter>::New();
    cutter->SetCutFunction(plane);
    cutter->SetInputData(myfits->GetOutput());
    cutter->Update();

    vtkSmartPointer<vtkPolyDataMapper> cutterMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    cutterMapper->SetInputConnection(cutter->GetOutputPort());

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData = cutter->GetOutput();

    vtkSmartPointer<vtkContourFilter> contoursFilter = vtkSmartPointer<vtkContourFilter>::New();
    contoursFilter->GenerateValues(ui->levelLineEdit->text().toInt(),
                                   ui->lowerBoundLineEdit->text().toDouble(),
                                   ui->upperBoundLineEdit->text().toDouble());
    contoursFilter->SetInputConnection(cutter->GetOutputPort());

    vtkSmartPointer<vtkPolyDataMapper> contourLineMapperer =
            vtkSmartPointer<vtkPolyDataMapper>::New();
    contourLineMapperer->SetInputConnection(contoursFilter->GetOutputPort());
    contourLineMapperer->SetScalarRange(ui->lowerBoundLineEdit->text().toDouble(),
                                        ui->upperBoundLineEdit->text().toDouble());
    contourLineMapperer->ScalarVisibilityOn();
    contourLineMapperer->SetScalarModeToUsePointData();
    contourLineMapperer->SetColorModeToMapScalars();

    currentContourActor->SetMapper(contourLineMapperer);
    currentContourActor->GetProperty()->SetLineWidth(1);
    ui->contourCheckBox->setChecked(true);
    ui->isocontourVtkWin->renderWindow()->GetRenderers()->GetFirstRenderer()->AddActor2D(
            currentContourActor);
    ui->isocontourVtkWin->renderWindow()->GetInteractor()->Render();

    if (myParentVtkWindow != 0) {
        // Riporto i contorni su visualizzazione principale
        double *sky_coord_gal = new double[2];
        AstroUtils().xy2sky(myfits->GetFileName(), 0, 0, sky_coord_gal, 3);
        double *coord = new double[3];
        AstroUtils().sky2xy(myParentVtkWindow->myfits->GetFileName(), sky_coord_gal[0],
                            sky_coord_gal[1], coord);

        double angle = 0;
        double x1 = coord[0];
        double y1 = coord[1];

        AstroUtils().xy2sky(myfits->GetFileName(), 0, 100, sky_coord_gal, 3);
        AstroUtils().sky2xy(myParentVtkWindow->myfits->GetFileName(), sky_coord_gal[0],
                            sky_coord_gal[1], coord);

        if (x1 != coord[0]) {
            double m = fabs((coord[1] - y1) / (coord[0] - x1));
            angle = 90 - atan(m) * 180 / M_PI;
        }

        double bounds[6];
        vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
        myfits->GetOutput()->GetBounds(bounds);

        // Rotate about the origin point (world coordinates)
        transform->Translate(0, 0, -1 * viewer->GetSlice());
        transform->Translate(bounds[0], bounds[2], 0);
        transform->RotateWXYZ(angle, 0, 0, 1);
        transform->Translate(-bounds[0], -bounds[2], 0);
        // transform->Translate( 0,0, -1*viewer->GetSlice());
        double scaledPixel = AstroUtils().arcsecPixel(myfits->GetFileName())
                / AstroUtils().arcsecPixel(myParentVtkWindow->myfits->GetFileName());

        vtkSmartPointer<vtkPolyDataMapper> mapperForMainWindow =
                vtkSmartPointer<vtkPolyDataMapper>::New();
        mapperForMainWindow->ShallowCopy(contourLineMapperer);

        currentContourActorForMainWindow->ShallowCopy(currentContourActor);
        currentContourActorForMainWindow->SetMapper(mapperForMainWindow);
        currentContourActorForMainWindow->SetScale(scaledPixel, scaledPixel, 1);
        // currentContourActorForMainWindow-> SetOrigin(x1,y1,0);
        currentContourActorForMainWindow->SetPosition(x1, y1, 1);
        currentContourActorForMainWindow->SetUserTransform(transform);

        myParentVtkWindow->m_Ren1->AddActor2D(currentContourActorForMainWindow);
        myParentVtkWindow->ui->qVTK1->update();
        myParentVtkWindow->ui->qVTK1->renderWindow()->GetInteractor()->Render();
        myParentVtkWindow->sessionModified();
    }
}

void vtkwindow_new::on_levelLineEdit_editingFinished()
{
    if (ui->contourCheckBox->isChecked())
        goContour();
}

void vtkwindow_new::removeContour()
{
    // ui->isocontourVtkWin->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor2D(currentContourActor);
    m_Ren2->RemoveActor2D(currentContourActor);
    if (myParentVtkWindow != 0) {
        myParentVtkWindow->m_Ren1->RemoveActor2D(currentContourActorForMainWindow);
        myParentVtkWindow->ui->qVTK1->update();
        myParentVtkWindow->ui->qVTK1->renderWindow()->GetInteractor()->Render();
        myParentVtkWindow->sessionModified();
    }
    ui->contourCheckBox->setChecked(false);
    ui->isocontourVtkWin->update();
    ui->isocontourVtkWin->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::on_contourCheckBox_clicked(bool checked)
{
    if (checked)
        goContour();
    else
        removeContour();
}

void vtkwindow_new::on_lut3dActivateCheckBox_clicked(bool checked)
{
    ui->linear3dRadioButton->setEnabled(checked);
    ui->log3dRadioButton->setEnabled(checked);
    ui->lut3dComboBox->setEnabled(checked);
    ui->scalarComboBox->setEnabled(checked);
    ui->toolButton->setEnabled(checked);
    // ui->glyphActivateCheckBox->setEnabled(checked);
    selected_scale = "Linear";
    changeScalar(ui->scalarComboBox->currentText().toStdString().c_str());
    // changePalette(ui->lut3dComboBox->currentText().toStdString().c_str());

    showColorbar(checked);

    if (ui->glyphActivateCheckBox->isChecked()) {
        ui->glyphShapeComboBox->activated(ui->glyphShapeComboBox->currentIndex());
    }
}

void vtkwindow_new::on_scalarComboBox_activated(const QString &arg1)
{
    changeScalar(arg1.toStdString());
    if (ui->glyphShapeComboBox->isEnabled()) {
        ui->glyphShapeComboBox->activated(ui->glyphShapeComboBox->currentIndex());
    }
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::on_lut3dComboBox_activated(const QString &arg1)
{
    changePalette(ui->lut3dComboBox->currentText().toStdString().c_str());
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::on_toolButton_clicked()
{
    LutCustomize *lcustom = new LutCustomize(this);
    lcustom->show();
}

void vtkwindow_new::on_glyphActivateCheckBox_clicked(bool checked)
{

    //    glyphLineEdit;

    QSettings settings(QDir::homePath()
                               .append(QDir::separator())
                               .append("VisIVODesktopTemp")
                               .append("/setting.ini"),
                       QSettings::NativeFormat);

    int maxpoint = settings.value("glyphmax", "2147483647").toString().toInt();

    if (checked) {
        int nPoints = pp->getRows();
        if (nPoints <= maxpoint) {
            ui->glyphShapeComboBox->setEnabled(true);
            ui->glyphScalarComboBox->setEnabled(true);
            ui->glyphScalingLineEdit->setEnabled(true);
            ui->glyphShapeComboBox->activated(ui->glyphShapeComboBox->currentIndex());
        } else {
            QMessageBox::critical(NULL, QObject::tr("Error"),
                                  QObject::tr("Too many points. Glyph will not be displayed."));
            ui->glyphActivateCheckBox->setChecked(false);
        }
    } else {
        if (glyph_actor != 0) {

            ui->glyphShapeComboBox->setEnabled(false);
            ui->glyphScalarComboBox->setEnabled(false);
            ui->glyphScalingLineEdit->setEnabled(false);
            m_Ren1->RemoveActor(glyph_actor);
            glyph_actor = 0;
            ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer()->Render();
            ui->qVTK1->renderWindow()->GetInteractor()->Render();
        }
    }
}

void vtkwindow_new::on_linear3dRadioButton_toggled(bool checked)
{
    if (checked)
        selected_scale = "Linear";
    else
        selected_scale = "Log";

    pp->scale = selected_scale.toStdString();

    pp->colorScalar = ui->scalarComboBox->currentText().toStdString();
    pp->palette = ui->lut3dComboBox->currentText().toStdString();

    pp->setLookupTableScale();

    //  changeFitsScale(ui->lutComboBox->currentText().toStdString().c_str(),
    //  selected_scale.toStdString().c_str());
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::drawGlyphs(int index)
{
    vtkSmartPointer<vtkGlyph3D> glyph3D = vtkSmartPointer<vtkGlyph3D>::New();

    if (index == 0) {

        vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
        sphereSource->SetThetaResolution(20);
        sphereSource->SetPhiResolution(10);
        sphereSource->SetRadius(1);
        glyph3D->SetSourceConnection(sphereSource->GetOutputPort());
    }
    if (index == 1) {
        vtkSmartPointer<vtkConeSource> coneSource = vtkSmartPointer<vtkConeSource>::New();
        coneSource->SetResolution(10);
        coneSource->SetRadius(1);
        coneSource->SetHeight(1);
        glyph3D->SetSourceConnection(coneSource->GetOutputPort());
    }
    if (index == 2) {
        vtkSmartPointer<vtkCylinderSource> sourceCylinder = vtkCylinderSource::New();
        sourceCylinder->SetResolution(10);
        sourceCylinder->SetRadius(1);
        sourceCylinder->SetHeight(1);
        glyph3D->SetSourceConnection(sourceCylinder->GetOutputPort());
    }
    if (index == 3) {
        vtkSmartPointer<vtkCubeSource> cubeSource = vtkSmartPointer<vtkCubeSource>::New();
        cubeSource->SetXLength(1);
        cubeSource->SetYLength(1);
        cubeSource->SetZLength(1);
        glyph3D->SetSourceConnection(cubeSource->GetOutputPort());
    }
    std::string glyph_scalar = ui->glyphScalarComboBox->currentText().toStdString();
    std::string color_scalar = ui->scalarComboBox->currentText().toStdString().c_str();

    pp->addScalar(glyph_scalar, false);
    if (ui->lut3dActivateCheckBox->isChecked()) {
        // pp->colorScalar=color_scalar;
        // pp->initLut();
        pp->addScalar(color_scalar, true);
    }

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(pp->getPolyData()->GetPoints());
    qDebug() << "Glyph Scalar " << QString::fromStdString(glyph_scalar);
    vtkSmartPointer<vtkDataArray> data =
            pp->getPolyData()->GetPointData()->GetScalars("scaleGlyph");
    if (data != 0) {
        double range[2];
        data->GetRange(range);
        qDebug() << range[0] << " " << range[1];
    }

    polyData->GetPointData()->SetScalars(
            pp->getPolyData()->GetPointData()->GetScalars("scaleGlyph"));
    if (ui->lut3dActivateCheckBox->isChecked()) {
        qDebug() << "Color Scalar " << QString::fromStdString(color_scalar);
        vtkSmartPointer<vtkDataArray> data =
                pp->getPolyData()->GetPointData()->GetArray(color_scalar.c_str());
        if (data != 0) {
            double range[2];
            data->GetRange(range);
            qDebug() << range[0] << " " << range[1];
        }
        polyData->GetPointData()->AddArray(
                pp->getPolyData()->GetPointData()->GetArray(color_scalar.c_str()));
        pp->getPolyData()->GetPointData()->RemoveArray(color_scalar.c_str());
    }

    // polyData->GetPointData()->SetActiveScalars(glyph_scalar.c_str());
    glyph3D->SetInputData(polyData);

    // glyph3D->SetInputData(pp->getPolyData());

    // glyph3D->ClampingOn();
    // glyph3D->SetRange(-0.5, 100.0);

    glyph3D->ScalingOn();
    // glyph3D->SetScaleModeToScaleByVector();

    glyph3D->SetScaleModeToScaleByScalar();
    glyph3D->SetInputArrayToProcess(0, 0, 0, 0, "scaleGlyph");

    if (ui->lut3dActivateCheckBox->isChecked()) {
        glyph3D->SetColorModeToColorByScalar();
        glyph3D->SetInputArrayToProcess(3, 0, 0, 0, color_scalar.c_str());
    }

    double scaleFactor = ui->glyphScalingLineEdit->text().toDouble();
    glyph3D->SetScaleFactor(scaleFactor);

    glyph3D->Update();

    // Visualize
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(glyph3D->GetOutputPort());

    if (ui->lut3dActivateCheckBox->isChecked()) {
        // ES
        changeScalar(ui->scalarComboBox->currentText().toStdString().c_str());
        changePalette(ui->lut3dComboBox->currentText().toStdString().c_str());
        // End ES
        vtkSmartPointer<vtkLookupTable> lut = pp->getLookupTable();
        // double *prevRange=new double[2];
        // prevRange=lut->GetRange();
        // lut->SetRange(-0.5,100.0);

        // mapper->SetColorModeToMapScalars();
        // mapper->ScalarVisibilityOn();
        // mapper->ColorByArrayComponent(ui->scalarComboBox->currentText().toStdString().c_str(),1);

        mapper->SetLookupTable(lut);
        mapper->SetScalarRange(lut->GetRange());

        // mapper->SetScalarRange(prevRange);
    }

    mapper->Update();

    if (glyph_actor != 0) {
        m_Ren1->RemoveActor(glyph_actor);
    }
    glyph_actor = vtkSmartPointer<vtkActor>::New();
    glyph_actor->SetMapper(mapper);

    m_Ren1->AddActor(glyph_actor);

    // FV

    // ui->qVTK1->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->Render();//QVTKOpenGLWindow::GetRenderWindow()
    // is deprecated, use renderWindow() instead. ui->qVTK1->update();
}

/*
void vtkwindow_new::drawGlyphs(int index){
    vtkSmartPointer<vtkGlyph3D> glyph3D =  vtkSmartPointer<vtkGlyph3D>::New();
    if(index==0){
        qDebug()<<"Sphere ";
        vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
        sphereSource->SetThetaResolution(20);
        sphereSource->SetPhiResolution(10);
        sphereSource->SetRadius(1);
        glyph3D->SetSourceConnection(sphereSource->GetOutputPort());
    }
    if(index==1){
        vtkSmartPointer<vtkConeSource> coneSource   = vtkSmartPointer<vtkConeSource>::New();
        coneSource->SetResolution(10);
        coneSource->SetRadius(1);
        coneSource->SetHeight(1);
        glyph3D->SetSourceConnection(coneSource->GetOutputPort());
    }
    if(index==2){
        vtkSmartPointer<vtkCylinderSource> sourceCylinder   = vtkCylinderSource::New();
        sourceCylinder->SetResolution(10);
        sourceCylinder->SetRadius(1);
        sourceCylinder->SetHeight(1);
        glyph3D->SetSourceConnection(sourceCylinder->GetOutputPort());
    }
    if(index==3){
        vtkSmartPointer<vtkCubeSource> cubeSource =  vtkSmartPointer<vtkCubeSource>::New();
        cubeSource->SetXLength (1);
        cubeSource->SetYLength (1);
        cubeSource->SetZLength (1);
        glyph3D->SetSourceConnection(cubeSource->GetOutputPort());
    }

    //vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    //polyData->SetPoints(pp->getPolyData()->GetPoints());
    //std::string glyph_scalar = ui->glyphScalarComboBox->currentText().toStdString();
    //qDebug()<<"Glyph Scalar "<<QString::fromStdString(glyph_scalar);
    //polyData->GetPointData()->SetScalars(pp->getPolyData()->GetPointData()->GetScalars(glyph_scalar.c_str()));
    ////polyData->GetPointData()->SetActiveScalars(glyph_scalar.c_str());
    //glyph3D->SetInputData(polyData);

    glyph3D->SetInputData(pp->getPolyData());


    glyph3D->ScalingOn();
    //glyph3D->SetScaleModeToScaleByVector();

    glyph3D->SetScaleModeToScaleByScalar();

    glyph3D->SetColorModeToColorByScalar();
    //glyph3D->SetVectorModeToUseVector();
    //glyph3D->SetScaleModeToDataScalingOff();
    //glyph3D->OrientOn();

    double scaleFactor=ui->glyphScalingLineEdit->text().toDouble();
    glyph3D->SetScaleFactor(scaleFactor);
    glyph3D->Update();

    // Visualize
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(glyph3D->GetOutputPort());
    vtkSmartPointer<vtkLookupTable> lut=pp->getLookupTable();

    //        pp->setLookupTableScale();
    //lut->SetValueRange(myfits->GetMin(), myfits->GetMax());

    mapper->SetLookupTable(lut);
    mapper->ScalarVisibilityOn();
    mapper->SetScalarRange(lut->GetRange());
    mapper->Update();

    if(glyph_actor!=0){
        m_Ren1->RemoveActor(glyph_actor);
    }
    glyph_actor = vtkSmartPointer<vtkActor>::New();
    glyph_actor->SetMapper(mapper);


    m_Ren1->AddActor ( glyph_actor );
    ui->qVTK1->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->Render();//QVTKOpenGLWindow::GetRenderWindow()
is deprecated, use renderWindow() instead. ui->qVTK1->update();

}
*/

// slider opacità
void vtkwindow_new::on_horizontalSlider_valueChanged(int value)
{

    int pos = 0;

    // Se non è selezionata un immagine nella tabella in basso a dx cambio il settaggio
    // dell'immagine base
    if (ui->listWidget->selectionModel()->selectedRows().count() != 0
        && imgLayerList.at(ui->listWidget->selectionModel()->selectedRows().at(0).row())->getType()
                == 0) {
        pos = ui->listWidget->selectionModel()->selectedRows().at(0).row();
        //  pos= imgLayerList.at(ui->listWidget->selectionModel()->selectedRows().at(0).row()
        //  )->getLayerNumber() ;
    }

    vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(pos))
            ->GetProperty()
            ->SetOpacity(value / 100.0);
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
    sessionModified();
}

void vtkwindow_new::on_glyphShapeComboBox_activated(int index)
{

    this->drawGlyphs(index);
    ui->qVTK1->renderWindow()->GetInteractor()->Render();

    // FV
    //  m_Ren1->ResetCamera();
}

void vtkwindow_new::on_glyphScalarComboBox_activated(const QString &arg1)
{
    // pp->colorScalar=arg1.toStdString();
    // pp->setActiveScalar();
    // changeScalar(arg1.toStdString());
    // if(ui->glyphShapeComboBox->isEnabled()){
    ui->glyphShapeComboBox->activated(ui->glyphShapeComboBox->currentIndex());
    ui->qVTK1->renderWindow()->GetInteractor()->Render();

    //}
}

void vtkwindow_new::on_glyphScalingLineEdit_editingFinished()
{
    ui->glyphShapeComboBox->activated(ui->glyphShapeComboBox->currentIndex());
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::on_ElementListWidget_clicked(const QModelIndex &index)
{
    int row = index.row();

    QMap<QString, QString> datacube = classElementsOnDb[row];

    double points[8];
    double longFrom, longTo, latFrom, latTo;
    longFrom = datacube["longitudeFrom"].toDouble();
    if (longFrom > 180)
        longFrom = longFrom - 360;
    else if (longFrom < -180)
        longFrom = longFrom + 360;
    longTo = datacube["longitudeTo"].toDouble();
    if (longTo > 180)
        longTo = longTo - 360;
    else if (longTo < -180)
        longTo = longTo + 360;

    latFrom = datacube["latitudeFrom"].toDouble();
    latTo = datacube["latitudeTo"].toDouble();
    double deltal, deltab;

    if (called_dl != "" && called_db != "") {
        deltal = called_dl.toDouble() / 2;
        deltab = called_db.toDouble() / 2;
    } else {
        deltal = called_r.toDouble();
        deltab = called_r.toDouble();
    }

    /*
    qDebug()<<"longFrom"<<called_l.toDouble()-called_r.();
    qDebug()<<"longTo"<<called_l.toDouble()+called_r.toDouble();

    if(longFrom<(called_l.toDouble()-called_r.toDouble())){
        longFrom=called_l.toDouble()-called_r.toDouble();
    }

    if(longTo>(called_l.toDouble()+called_r.toDouble())){
        longTo=called_l.toDouble()+called_r.toDouble();
    }
    if(latFrom<(called_b.toDouble()-called_r.toDouble())){
        latFrom=called_b.toDouble()-called_r.toDouble();
    }
    if(latTo>(called_b.toDouble()+called_r.toDouble())){
        latTo=called_b.toDouble()+called_r.toDouble();
    }
*/

    qDebug() << "deltal " << deltal << " deltab " << deltab;
    qDebug() << "called_l.toDouble()-deltal " << called_l.toDouble() - deltal
             << " called_l.toDouble()+deltal  " << called_l.toDouble() + deltal;
    qDebug() << "called_b.toDouble()-deltab " << called_b.toDouble() - deltab
             << " called_b.toDouble()+deltab " << called_b.toDouble() + deltab;

    if (longFrom < (called_l.toDouble() - deltal)) {
        longFrom = called_l.toDouble() - deltal;
    }

    if (longTo > (called_l.toDouble() + deltal)) {
        longTo = called_l.toDouble() + deltal;
    }
    if (latFrom < (called_b.toDouble() - deltab)) {
        latFrom = called_b.toDouble() - deltab;
    }
    if (latTo > (called_b.toDouble() + deltab)) {
        latTo = called_b.toDouble() + deltab;
    }
    /*
    points[0]=longFrom;
    points[1]=latFrom;
    points[2]=longTo;
    points[3]=latFrom;
    points[4]=longTo;
    points[5]=latTo;
    points[6]=longFrom;
    points[7]=latTo;
*/

    points[0] = datacube["longitudeP1"].toDouble();
    points[1] = datacube["latitudeP1"].toDouble();
    points[2] = datacube["longitudeP2"].toDouble();
    ;
    points[3] = datacube["latitudeP2"].toDouble();
    points[4] = datacube["longitudeP3"].toDouble();
    ;
    points[5] = datacube["latitudeP3"].toDouble();
    points[6] = datacube["longitudeP4"].toDouble();
    points[7] = datacube["latitudeP4"].toDouble();

    qDebug() << " P1: " << points[0] << " - " << points[1];
    qDebug() << " P2: " << points[2] << " - " << points[3];
    qDebug() << " P3: " << points[4] << " - " << points[5];
    qDebug() << " P4: " << points[6] << " - " << points[7];

    double *coord = new double[3];
    double xypoints[8];

    for (int i = 0; i < 8; i = i + 2) {
        AstroUtils().sky2xy(myfits->GetFileName(), points[i], points[i + 1], coord);
        xypoints[i] = coord[0];
        xypoints[i + 1] = coord[1];
    }
    drawRectangleFootprint(xypoints);
}

void vtkwindow_new::drawRectangleFootprint(double points[8])
{
    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    pts->InsertNextPoint(points[0], points[1], 0);
    pts->InsertNextPoint(points[2], points[3], 0);
    pts->InsertNextPoint(points[4], points[5], 0);
    pts->InsertNextPoint(points[6], points[7], 0);

    vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata->Allocate();
    polydata->SetPoints(pts);
    vtkIdType connectivity[8];
    connectivity[0] = 0;
    connectivity[1] = 1;
    connectivity[2] = 1;
    connectivity[3] = 2;
    connectivity[4] = 2;
    connectivity[5] = 3;
    connectivity[6] = 3;
    connectivity[7] = 0;

    polydata->InsertNextCell(
            VTK_LINE, 8,
            connectivity); // Connects the first and fourth point we inserted into a line

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polydata);

    if (rectangleActor != 0) {
        m_Ren1->RemoveActor(rectangleActor);
    }
    rectangleActor = vtkSmartPointer<vtkLODActor>::New();
    rectangleActor->SetMapper(mapper);
    rectangleActor->GetProperty()->SetColor(1.0, 0.0, 0.0);
    m_Ren1->AddActor(rectangleActor);

    ui->qVTK1->update();
    ui->qVTK1->interactor()->Render();
}

bool vtkwindow_new::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);

    if (event->type() == QEvent::FocusOut) {
        if (rectangleActor != 0) {
            m_Ren1->RemoveActor(rectangleActor);
            ui->qVTK1->renderWindow()->GetInteractor()->Render();
        }
    }
    return false;
}

void vtkwindow_new::on_listWidget_clicked(const QModelIndex &index)
{
    if (ui->listWidget->selectionModel()->selectedRows().count() != 0
        && imgLayerList.at(index.row())->getType() == 0) {
        int row = ui->listWidget->selectionModel()->selectedRows().at(0).row();
        imageStack->SetActiveLayer(row);
        ui->horizontalSlider->setValue(
                vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(row))
                        ->GetProperty()
                        ->GetOpacity()
                * 100.0);
        ui->lutComboBox->setCurrentText(imgLayerList.at(row)->getLutType());
        auto radioBtn = imgLayerList.at(row)->getLutScale() == "Linear" ? ui->linearadioButton
                                                                        : ui->logRadioButton;
        radioBtn->setChecked(true);
    }
}

void vtkwindow_new::on_listWidget_itemChanged(QListWidgetItem *item)
{
    // checkbox img
    checkboxImageClicked(item->listWidget()->row(item), item->checkState() == Qt::Checked);
    sessionModified();
}

void vtkwindow_new::movedLayersRow(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
                                   const QModelIndex &destinationParent, int destinationRow)
{

    if (sourceStart > destinationRow) { // down

        for (int i = sourceStart - 1; i >= destinationRow; i--) {
            vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(i))
                    ->GetProperty()
                    ->SetLayerNumber(i + 1);
            imgLayerList.swapItemsAt(i, i + 1);
        }

        vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(sourceStart))
                ->GetProperty()
                ->SetLayerNumber(destinationRow);

    } else { // up

        for (int i = sourceStart + 1; i < destinationRow; i++) {
            vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(i))
                    ->GetProperty()
                    ->SetLayerNumber(i - 1);
            imgLayerList.swapItemsAt(i, i - 1);
        }
        vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(sourceStart))
                ->GetProperty()
                ->SetLayerNumber(destinationRow - 1);
    }

    ui->qVTK1->update();
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
    sessionModified();
}

void vtkwindow_new::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    /*
    // if(index.column()==0)
    if (layerList.at(item->listWidget()->row(item))->getType()==0)
    {
        //settaggi dell'immagine selezionata
    }
    else
    {

        //Initial color
        double r=getVisualizedActorList().value(ui->listWidget->item(
    item->listWidget()->row(item))->text())->GetProperty()->GetColor()[0]*255; double
    g=getVisualizedActorList().value(ui->listWidget->item(item->listWidget()->row(item))->text())->GetProperty()->GetColor()[1]*255;
        double b=getVisualizedActorList().value(ui->listWidget->item(
    item->listWidget()->row(item))->text())->GetProperty()->GetColor()[2]*255;

        QColor color = QColorDialog::getColor(QColor(r,g,b), this);
        getVisualizedActorList().value(ui->listWidget->item(
    item->listWidget()->row(item))->text())->GetProperty()->SetColor(color.redF(), color.greenF(),
    color.blueF()); ui->qVTK1->update();

        //update color on table
        ui->listWidget->item(item->listWidget()->row(item))->setForeground(QBrush(color));
        //>setStyleSheet("background-color:
    rgb("+QString::number(color.redF()*255)+","+QString::number(color.greenF()*255)+"
    ,"+QString::number(color.blueF()*255)+")");
    }
*/
}

void vtkwindow_new::on_listWidget_customContextMenuRequested(const QPoint &pos)
{
    /*
    vosamp *samp = &Singleton<vosamp>::Instance();
    char* clist=samp->getClientsList();
   //char* clist;
    QStringList clientList=
   QString::fromLocal8Bit(clist).split(QRegExp("[\r\n]"),QString::SkipEmptyParts);

    QHash<QString, QString> clientHashMap;

    foreach (QString str, clientList)
    {
        clientHashMap.insert( str.section(" ",1), str.section(" ",0,0));
    }


    QSignalMapper* signalMapper = new QSignalMapper (this);

    QPoint globalPos = ui->listWidget->mapToGlobal(pos);	// Map the global position to the
   userlist QModelIndex t = ui->listWidget->indexAt(pos);
    ui->listWidget->item(t.row())->setSelected(true);		// even a right click will select
   the item

    QMenu *myMenu=new QMenu(this);
    // connect (myMenu, SIGNAL(triggered()), signalMapper, SLOT(map())) ;

    bool none=true;
    if (clientHashMap.contains("SAOImage DS9"))
    {
        qDebug()<<clientHashMap.value("SAOImage DS9");
        none=false;
        // myMenu.addAction("Send to DS9", this, SLOT(sendImageTo()));
        //    signalMapper -> setMapping ( myMenu->addAction("Send to
   DS9"),clientHashMap.value("SAOImage DS9")  ) ;

        QAction *send= new QAction("Send to DS9",this);
        connect (send, SIGNAL(triggered()), signalMapper, SLOT(map())) ;
        signalMapper -> setMapping (send, clientHashMap.value("SAOImage DS9") ) ;
        myMenu->addAction(send);


    }
    if (clientHashMap.contains("Aladin"))
    {
        none=false;

        QAction *send= new QAction("Send to Aladin",this);
        connect (send, SIGNAL(triggered()), signalMapper, SLOT(map())) ;
        signalMapper -> setMapping (send, clientHashMap.value("Aladin") ) ;
        myMenu->addAction(send);

    }

    if (none)
        myMenu->addAction("No SAMP client available");

    connect (signalMapper, SIGNAL(mapped(QString)), this, SLOT(sendImageTo(QString))) ;
    //  myMenu->exec(globalPos);
    myMenu->popup(globalPos);
*/
}

void vtkwindow_new::sendImageTo(QString id)
{

    /*
    qDebug()<<"CLIENT ID: "<<id;

    char *file =
    strdup(imgLayerList.at(ui->listWidget->selectionModel()->selectedRows().at(0).row())->getFits()->GetFileName().c_str());
    char *to = strdup(id.toStdString().c_str());

    vosamp *samp = &Singleton<vosamp>::Instance();
    samp->sendFitsImage(file,to);
    free (file);

*/
}

void vtkwindow_new::on_bubblePushButton_clicked()
{
    ui->bubblePushButton->setStyleSheet(
            "background-color: rgb(0, 0, 250);border-radius: 3px; border-width: 1px;");
    ui->rectangularSelectionCS->setStyleSheet("");
    ui->tdRectPushButton->setStyleSheet("");
    ui->fil_rectPushButton->setStyleSheet("");

    vtkSmartPointer<SkyRegionSelector> style = vtkSmartPointer<SkyRegionSelector>::New();
    style->setVtkWin(this);
    style->setIsBubble();
    ui->qVTK1->renderWindow()->GetInteractor()->SetInteractorStyle(style);
    ui->qVTK1->setCursor(Qt::CrossCursor);
}

void vtkwindow_new::on_filterMoreButton_clicked()
{
    FilterCustomize *filterCustomize = new FilterCustomize(this);
    filterCustomize->show();
}

void vtkwindow_new::on_actionCalculate_order_0_triggered()
{
    if (myParentVtkWindow != 0) {
        auto moment = vtkSmartPointer<vtkFitsReader>::New();
        moment->SetFileName(myfits->GetFileName());
        moment->isMoment3D = true;
        moment->setMomentOrder(0);
        myParentVtkWindow->addLayerImage(moment);
    }
}

void vtkwindow_new::on_actionCalculate_order_1_triggered()
{
    if (myParentVtkWindow != 0) {
        auto moment = vtkSmartPointer<vtkFitsReader>::New();
        moment->SetFileName(myfits->GetFileName());
        moment->isMoment3D = true;
        moment->setMomentOrder(1);
        myParentVtkWindow->addLayerImage(moment);
    }
}

void vtkwindow_new::on_actionFront_triggered()
{
    setCameraAzimuth(0);
}

void vtkwindow_new::on_actionBack_triggered()
{
    setCameraAzimuth(-180);
}

void vtkwindow_new::on_actionTop_triggered()
{
    setCameraElevation(90);
}

void vtkwindow_new::on_actionRight_triggered()
{
    setCameraAzimuth(90);
}

void vtkwindow_new::on_actionBottom_triggered()
{
    setCameraElevation(-90);
}

void vtkwindow_new::on_actionLeft_triggered()
{
    setCameraAzimuth(-90);
}

void vtkwindow_new::on_actionCAESAR_triggered()
{
    auto caesarAuth = &NeaniasCaesarAuth::Instance();
    if (!caesarAuth->isAuthenticated()) {
        QMessageBox::warning(this, "Authentication required",
                             "This service is restricted to authenticated users.\n"
                             "Sign in to the service through the Settings window to continue.");
        return;
    }

    QString imagePath = QString::fromStdString(myfits->GetFileName());
    auto caesar = new CaesarWindow(this, imagePath);
    caesar->show();
    caesar->activateWindow();
    caesar->raise();
}

void vtkwindow_new::loadSession(const QString &sessionFile, const QDir &sessionRootFolder)
{
    auto loadingWindow = new LoadingWidget(this);
    loadingWindow->setWindowFlag(Qt::Window);
    loadingWindow->setWindowFlag(Qt::WindowStaysOnTopHint);
    loadingWindow->setWindowTitle("Loading session");
    loadingWindow->setText("Loading the session, please wait...");
    loadingWindow->setButtonStatus(false);
    loadingWindow->show();

    QFile f(sessionFile);
    f.open(QFile::ReadOnly);
    QJsonObject session = QJsonDocument::fromJson(f.readAll()).object();
    f.close();
    this->sessionFile = sessionFile;

    auto layers = session["image"].toObject()["layers"].toArray();
    setImageLayers(layers, sessionRootFolder);

    auto compactSources = session["image"].toObject()["compact_sources"].toArray();
    if (!compactSources.isEmpty())
        setSources(compactSources, sessionRootFolder);

    auto filaments = session["image"].toObject()["filaments"].toArray();
    if (!filaments.isEmpty())
        setFilaments(filaments, sessionRootFolder);

    auto datacubes = session["datacubes"].toArray();
    if (!datacubes.isEmpty())
        loadDatacubes(datacubes, sessionRootFolder);

    sessionSaved = true;
    loadingWindow->deleteLater();
}

void vtkwindow_new::sessionModified()
{
    sessionSaved = false;
}

bool vtkwindow_new::isSessionSaved() const
{
    return sessionSaved;
}

bool vtkwindow_new::confirmSaveAndExit()
{
    auto res = QMessageBox::question(this, "Unsaved changes",
                                     "Do you want to save your work before leaving?",
                                     QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (res == QMessageBox::Cancel) {
        // Do nothing
        return false;
    }

    if (res == QMessageBox::Yes) {
        on_actionSave_session_triggered();
    }

    return true;
}

void vtkwindow_new::setImageLayers(const QJsonArray &layers, const QDir &sessionRootFolder)
{
    if (layers.count() == 1) {
        return;
    }

    // Clear current layers
    ui->listWidget->clear();
    imgLayerList.clear();
    imageStack->GetImages()->RemoveAllItems();

    foreach (const auto &it, layers) {
        auto layer = it.toObject();
        if (!layer["enabled"].toBool()) {
            continue;
        }

        QString filepath = sessionRootFolder.absoluteFilePath(layer["fits"].toString());

        vtkSmartPointer<vtkFitsReader> fitsReader;
        // If this layer is origin, then we already have the fitsReader
        if (layer["origin"].toBool()) {
            fitsReader = myfits;
        } else {
            fitsReader = vtkSmartPointer<vtkFitsReader>::New();
            fitsReader->SetFileName(filepath.toStdString());
            if (layer["type"].toString() == "Moment") {
                fitsReader->isMoment3D = true;
                fitsReader->setMomentOrder(layer["moment_order"].toInt());
            }
        }

        // Setup
        QString filename = QFileInfo(filepath).baseName();
        addLayerImage(fitsReader);
        ui->listWidget->setCurrentRow(ui->listWidget->count() - 1);
        auto listItem = ui->listWidget->currentItem();
        listItem->setText(layer["text"].toString(filename));
        changeFitsScale(layer["lutType"].toString("Gray").toStdString(),
                        layer["lutScale"].toString("Log").toStdString());
        vtkImageSlice::SafeDownCast(
                imageStack->GetImages()->GetItemAsObject(ui->listWidget->row(listItem)))
                ->GetProperty()
                ->SetOpacity(layer["opacity"].toInt(99) / 100.0);
        listItem->setCheckState(layer["show"].toBool(false) ? Qt::Checked : Qt::Unchecked);
    }
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
    ui->listWidget->setCurrentRow(0);
    emit ui->listWidget->clicked(ui->listWidget->selectionModel()->currentIndex());
}

bool vtkwindow_new::getTableItemInfo(const QString &text, int &row, bool &enabled, double *color)
{
    auto list = ui->tableWidget->findItems(text, Qt::MatchExactly);
    if (list.count() > 0) {
        row = list.first()->row();
        enabled = qobject_cast<QCheckBox *>(ui->tableWidget->cellWidget(row, 0))->isChecked();
        getVisualizedActorList().value(text)->GetProperty()->GetColor(color);
        return true;
    }
    return false;
}

void vtkwindow_new::setTableItemInfo(const QString &text, const bool &enabled, const double *color)
{
    auto list = ui->tableWidget->findItems(text, Qt::MatchExactly);
    if (list.count() > 0) {
        auto row = list.first()->row();
        auto cb = qobject_cast<QCheckBox *>(ui->tableWidget->cellWidget(row, 0));
        auto ellipseActor = getVisualizedActorList().value(text);
        ellipseActor->GetProperty()->SetColor(color[0], color[1], color[2]);
        cb->setStyleSheet("background-color: rgb(" + QString::number(color[0] * 255) + ","
                          + QString::number(color[1] * 255) + " ," + QString::number(color[2] * 255)
                          + ")");
        if (enabled) {
            cb->setChecked(true);
            ellipseActor->VisibilityOn();
        } else {
            cb->setChecked(false);
            ellipseActor->VisibilityOff();
        }
    }
}

void vtkwindow_new::setSources(const QJsonArray &sources, const QDir &sessionRootFolder)
{
    auto fileLoad = new Vialactea_FileLoad("", true);
    fileLoad->setVtkWin(this);
    fileLoad->setCatalogueActive();

    foreach (const auto &it, sources) {
        auto compactSource = it.toObject();
        QString filepath = sessionRootFolder.absoluteFilePath(compactSource["file"].toString());
        QString filename = QFileInfo(filepath).baseName();

        QString format = compactSource["format"].toString("tsv");

        if (format == "tsv") {
            fileLoad->init(filepath);

            if (compactSource["bandmerged"].toBool()) {
                fileLoad->setWavelength("all");
                fileLoad->on_okPushButton_clicked();
                foreach (const auto &item, compactSource["tableItems"].toArray()) {
                    auto text = item["text"].toString();
                    auto enabled = item["enabled"].toBool(false);
                    auto color = item["color"].toArray();
                    double rgb[3] = { color.at(0).toDouble(), color.at(1).toDouble(),
                                      color.at(2).toDouble() };
                    setTableItemInfo(text, enabled, rgb);
                }
            } else {
                fileLoad->setWavelength(QString::number(compactSource["wavelength"].toInt()));
                fileLoad->on_okPushButton_clicked();
                auto text = compactSource["text"].toString(filename);
                auto enabled = compactSource["enabled"].toBool(false);
                auto color = compactSource["color"].toArray();
                double rgb[3] = { color.at(0).toDouble(), color.at(1).toDouble(),
                                  color.at(2).toDouble() };
                setTableItemInfo(text, enabled, rgb);
            }
        } else if (format == "ds9") {
            addDS9Regions(filepath);
        } else if (format == "json") {
            addSourcesFromJson(filepath);
        }
    }

    fileLoad->deleteLater();
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::setFilaments(const QJsonArray &filaments, const QDir &sessionRootFolder)
{
    auto fileLoad = new Vialactea_FileLoad("", true);
    fileLoad->setVtkWin(this);

    foreach (const auto &it, filaments) {
        auto filament = it.toObject();
        QString filepath = sessionRootFolder.absoluteFilePath(filament["file"].toString());
        QString filename = QFileInfo(filepath).baseName();
        fileLoad->init(filepath);
        fileLoad->importFilaments();

        foreach (const auto &item, filament["tableItems"].toArray()) {
            auto text = item["text"].toString(filename);
            auto enabled = item["enabled"].toBool(false);
            auto color = item["color"].toArray();
            double rgb[3] = { color.at(0).toDouble(), color.at(1).toDouble(),
                              color.at(2).toDouble() };
            setTableItemInfo(text, enabled, rgb);
        }
    }

    fileLoad->deleteLater();
    ui->qVTK1->renderWindow()->GetInteractor()->Render();
}

void vtkwindow_new::loadDatacubes(const QJsonArray &datacubes, const QDir &sessionRootFolder)
{
    foreach (const auto &dc, datacubes) {
        auto enabled = dc["enabled"].toBool();
        if (!enabled) {
            continue;
        }

        auto fitsPath = sessionRootFolder.absoluteFilePath(dc["fits"].toString());
        auto slice = dc["slice"].toInt(1);

        auto fitsReader = vtkSmartPointer<vtkFitsReader>::New();
        fitsReader->SetFileName(fitsPath.toStdString());
        fitsReader->is3D = true;

        auto win = new vtkwindow_new(this, fitsReader, 1, this);
        win->setCuttingPlaneValue(slice);

        if (dc.toObject().contains("threshold")) {
            win->setThresholdValue(dc["threshold"].toInt());
        }

        auto contours = dc["contours"].toObject();
        if (!contours.isEmpty()) {
            auto level = contours["level"].toInt(15);
            auto lowerB = contours["lowerBound"].toDouble(3 * fitsReader->GetRMS());
            auto upperB = contours["upperBound"].toDouble(fitsReader->GetMax());
            auto enabled = contours["enabled"].toBool(false);
            win->setContoursInfo(level, lowerB, upperB, enabled);
        }
    }
}

void vtkwindow_new::on_actionSave_session_triggered()
{
    bool overwriting = false;
    if (!this->sessionFile.isEmpty()) {
        auto res = QMessageBox::question(this, "Save session",
                                         "Do you want to overwrite the current session?");
        overwriting = res == QMessageBox::Yes;
    }

    QString dir;
    if (overwriting) {
        // The session folder already exists and contains the json file
        dir = QFileInfo(this->sessionFile).absolutePath();
    } else {
        dir = QFileDialog::getExistingDirectory(this, "Save session", QDir::homePath(),
                                                QFileDialog::ShowDirsOnly);
        if (dir.isEmpty())
            return;

        QDir tmp(dir);
        if (!tmp.isEmpty()) {
            QMessageBox::warning(this, "Save session",
                                 "The directory is not empty, please select an empty folder.");
            return;
        }
        this->sessionFile = tmp.absoluteFilePath("session.json");
    }

    QDir sessionFolder(dir);

    const auto fileInSession = [&sessionFolder](const QFileInfo &file, QString subfolder) {
        QString src = file.absoluteFilePath();
        QString dst = src;
        if (!dst.startsWith(sessionFolder.absolutePath())) {
            sessionFolder.mkdir(subfolder);
            dst = sessionFolder.absoluteFilePath(
                    subfolder.append(QDir::separator()).append(file.fileName()));
            QFile::copy(src, dst);
        }
        return dst;
    };

    QJsonObject image;
    // Layers
    QJsonArray layers;
    for (int row = 0; row < ui->listWidget->count(); ++row) {
        auto listItem = ui->listWidget->item(row);
        auto img = imgLayerList.at(row);
        auto fits = QFileInfo(QString::fromStdString(img->getFits()->GetFileName()));

        QString dst = fileInSession(fits, "fits");

        QJsonObject layer;
        if (img->getFits()->isMoment3D) {
            layer["type"] = QString("Moment");
            layer["moment_order"] = img->getFits()->getMomentOrder();
        } else {
            layer["type"] = QString("Continuum");
        }
        layer["origin"] = img->getFits() == myfits;
        layer["text"] = listItem->text();
        layer["fits"] = sessionFolder.relativeFilePath(dst);
        layer["lutType"] = img->getLutType().isEmpty() ? "Gray" : img->getLutType();
        layer["lutScale"] = img->getLutScale().isEmpty() ? "Log" : img->getLutScale();
        layer["opacity"] =
                vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(row))
                        ->GetProperty()
                        ->GetOpacity()
                * 100;
        layer["show"] = listItem->checkState() == Qt::Checked;
        layer["enabled"] = true;
        layers.append(layer);
    }
    image["layers"] = layers;

    // Compact sources
    QJsonArray sources;
    for (auto i = file_wavelength.constBegin(); i != file_wavelength.constEnd(); ++i) {
        auto srcInfo = QFileInfo(i.key());
        auto bandmerged = i.value() == 0;

        QString src = srcInfo.absoluteFilePath();
        QString dst = fileInSession(srcInfo, "compact_sources");

        QJsonObject compactSource;
        compactSource["file"] = sessionFolder.relativeFilePath(dst);
        compactSource["bandmerged"] = bandmerged;
        compactSource["format"] = "tsv";

        if (bandmerged) {
            QJsonArray tableItems;
            auto items = getSourcesLoadedFromFile(src);
            foreach (const auto &item, items) {
                int row;
                bool enabled;
                double ellipseColor[3];
                getTableItemInfo(item, row, enabled, ellipseColor);
                QJsonArray rgb;
                rgb << ellipseColor[0] << ellipseColor[1] << ellipseColor[2];

                QJsonObject obj;
                obj["text"] = item;
                obj["enabled"] = enabled;
                obj["color"] = rgb;
                tableItems.append(obj);
            }
            compactSource["tableItems"] = tableItems;
        } else {
            auto text = getSourcesLoadedFromFile(src).first();
            int row;
            bool enabled;
            double ellipseColor[3];
            getTableItemInfo(text, row, enabled, ellipseColor);
            QJsonArray rgb;
            rgb << ellipseColor[0] << ellipseColor[1] << ellipseColor[2];

            compactSource["wavelength"] = i.value();
            compactSource["text"] = text;
            compactSource["enabled"] = enabled;
            compactSource["color"] = rgb;
        }

        sources.append(compactSource);
    }
    foreach (auto &&file, ds9RegionFiles) {
        auto srcInfo = QFileInfo(file);

        QString src = srcInfo.absoluteFilePath();
        QString dst = fileInSession(srcInfo, "ds9_regions");

        QJsonObject compactSource;
        compactSource["file"] = sessionFolder.relativeFilePath(dst);
        compactSource["format"] = "ds9";
        sources.append(compactSource);
    }
    foreach (auto &&file, jsonRegionFiles) {
        auto srcInfo = QFileInfo(file);

        QString src = srcInfo.absoluteFilePath();
        QString dst = fileInSession(srcInfo, "json_catalogues");

        QJsonObject compactSource;
        compactSource["file"] = sessionFolder.relativeFilePath(dst);
        compactSource["format"] = "json";
        sources.append(compactSource);
    }
    image["compact_sources"] = sources;

    // Filaments
    QJsonArray filaments;
    for (auto i = filamentsList.constBegin(); i != filamentsList.constEnd(); ++i) {
        auto srcInfo = QFileInfo(i.key());

        QString dst = fileInSession(srcInfo, "filaments");

        QJsonObject filament;
        filament["file"] = sessionFolder.relativeFilePath(dst);
        QJsonArray tableItems;
        foreach (const auto &text, i.value()) {
            int row;
            bool enabled;
            double ellipseColor[3];
            getTableItemInfo(text, row, enabled, ellipseColor);
            QJsonArray rgb;
            rgb << ellipseColor[0] << ellipseColor[1] << ellipseColor[2];

            QJsonObject obj;
            obj["text"] = text;
            obj["enabled"] = enabled;
            obj["color"] = rgb;
            tableItems.append(obj);
        }
        filament["tableItems"] = tableItems;
        filaments.append(filament);
    }
    image["filaments"] = filaments;

    // Datacubes
    QJsonArray datacubes;
    foreach (const auto &win, this->findChildren<vtkwindow_new *>()) {
        if (!win->isDatacube)
            continue;

        auto fits = QFileInfo(QString::fromStdString(win->getFitsImage()->GetFileName()));
        QString dst = fileInSession(fits, "fits");

        QJsonObject dc;
        dc["fits"] = sessionFolder.relativeFilePath(dst);
        dc["threshold"] = win->getThresholdValue();
        dc["slice"] = win->getCuttingPlaneValue();
        dc["enabled"] = win->isVisible();

        QJsonObject contours;
        int level;
        double lowerB, upperB;
        bool enabled = win->getContoursInfo(level, lowerB, upperB);
        contours["level"] = level;
        contours["lowerBound"] = lowerB;
        contours["upperBound"] = upperB;
        contours["enabled"] = enabled;
        dc["contours"] = contours;
        datacubes.append(dc);
    }

    // Save the session.json configuration file
    QJsonObject root;
    root["image"] = image;
    root["datacubes"] = datacubes;
    root["saved"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    QJsonDocument session(root);
    QFile sessionFile(this->sessionFile);
    sessionFile.open(QFile::WriteOnly);
    sessionFile.write(session.toJson());
    sessionFile.close();
    sessionSaved = true;
    QMessageBox::information(this, tr("Save session"),
                             tr("Session saved in ") + sessionFolder.absolutePath());
}
