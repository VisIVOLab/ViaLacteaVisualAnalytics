#include "vtkwindow_interactors.h"

#include "vtkwindow_new.h"
#include "dbquery.h"
#include "vlkbsimplequerycomposer.h"
#include "vtkellipse.h"

#include <vtkActor.h>
#include <vtkAreaPicker.h>
#include <vtkCommand.h>
#include <vtkCoordinate.h>
#include <vtkExtractGeometry.h>
#include <vtkExtractSelection.h>
#include <vtkGeometryFilter.h>
#include <vtkHardwareSelector.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkIntArray.h>
#include <vtkInteractorStyleRubberBand2D.h>
#include <vtkLODActor.h>
#include <vtkMath.h>
#include <vtkPlanes.h>
#include <vtkPointPicker.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPropPicker.h>
#include <vtkProperty.h>
#include <vtkRect.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSelection.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVector.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkObjectFactory.h>

#include <QStatusBar>
#include <QString>

vtkStandardNewMacro(InteractorStyleFreeHandOn3DVisualization);
vtkStandardNewMacro(InteractorStyleSelctionPointOn3DVisualization);
vtkStandardNewMacro(MyRubberBand);
vtkStandardNewMacro(myVtkInteractorStyleImage);
vtkStandardNewMacro(SkyRegionSelector);
vtkStandardNewMacro(InteractorStyleExtractSources);
vtkStandardNewMacro(InteractorStyleEditSource);

InteractorStyleFreeHandOn3DVisualization::InteractorStyleFreeHandOn3DVisualization()
{
    this->SelectedMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    this->SelectedActor = vtkSmartPointer<vtkActor>::New();
    this->SelectedActor->SetMapper(SelectedMapper);
}

void InteractorStyleFreeHandOn3DVisualization::OnLeftButtonUp()
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
        hardSel->SetArea(origin[0], origin[1], origin[0] + wsize[0] - 1, origin[1] + wsize[1] - 1);
        hardSel->SetFieldAssociation(vtkDataObject::FIELD_ASSOCIATION_POINTS);

        if (hardSel->CaptureBuffers()) {

            vtkSelection *psel = hardSel->GeneratePolygonSelection(
                    polygonPointsArray->GetPointer(0), polygonPointsArray->GetNumberOfTuples() * 2);
            hardSel->ClearBuffers();

            vtkSmartPointer<vtkSelection> sel;
            sel.TakeReference(psel);

            vtkSmartPointer<vtkExtractSelection> extractSelection =
                    vtkSmartPointer<vtkExtractSelection>::New();

            extractSelection->SetInputData(this->Points);
            extractSelection->SetInputData(1, sel);
            extractSelection->Update();

            vtkSmartPointer<vtkUnstructuredGrid> selected = vtkSmartPointer<vtkUnstructuredGrid>::New();
            selected->ShallowCopy(extractSelection->GetOutput());

            vtkSmartPointer<vtkGeometryFilter> geometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
            geometryFilter->SetInputData(selected);
            geometryFilter->Update();
            vtkPolyData *selected_poly = geometryFilter->GetOutput();

            this->SelectedMapper->SetInputData(selected_poly);
            this->SelectedMapper->ScalarVisibilityOff();

            double r = vtkMath::Random(0.0, 1.0);
            double g = vtkMath::Random(0.0, 1.0);
            double b = vtkMath::Random(0.0, 1.0);

            this->SelectedActor->GetProperty()->SetColor(r, g, b); //(R,G,B)
            this->SelectedActor->GetProperty()->SetPointSize(3);
            this->GetInteractor()
                    ->GetRenderWindow()
                    ->GetRenderers()
                    ->GetFirstRenderer()
                    ->AddActor(SelectedActor);
            this->GetInteractor()->GetRenderWindow()->Render();
            this->HighlightProp(NULL);
        }
    }
}

void InteractorStyleFreeHandOn3DVisualization::SetPoints(vtkSmartPointer<vtkPolyData> points)
{
    this->Points = points;
    this->Points_ori = points;
}

void InteractorStyleFreeHandOn3DVisualization::setVtkWin(vtkwindow_new *w) { vtkwin = w; }

InteractorStyleSelctionPointOn3DVisualization::InteractorStyleSelctionPointOn3DVisualization()
{
    this->SelectedMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    this->SelectedActor = vtkSmartPointer<vtkActor>::New();
    this->SelectedActor->SetMapper(SelectedMapper);
}

void InteractorStyleSelctionPointOn3DVisualization::OnLeftButtonUp()
{

    vtkInteractorStyleRubberBandPick::OnLeftButtonUp();
    vtkPlanes *frustum =
            static_cast<vtkAreaPicker *>(this->GetInteractor()->GetPicker())->GetFrustum();

    vtkSmartPointer<vtkExtractGeometry> extractGeometry = vtkSmartPointer<vtkExtractGeometry>::New();
    extractGeometry->SetImplicitFunction(frustum);
    extractGeometry->SetInputData(this->Points);
    extractGeometry->Update();

    vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
    glyphFilter->SetInputConnection(extractGeometry->GetOutputPort());
    glyphFilter->Update();

    vtkPolyData *selected = glyphFilter->GetOutput();
    this->SelectedMapper->SetInputData(selected);
    this->SelectedMapper->ScalarVisibilityOff();
    double r = vtkMath::Random(0.0, 1.0);
    double g = vtkMath::Random(0.0, 1.0);
    double b = vtkMath::Random(0.0, 1.0);
    this->SelectedActor->GetProperty()->SetColor(r, g, b); //(R,G,B)
    this->SelectedActor->GetProperty()->SetPointSize(3);
    this->CurrentRenderer->AddActor(SelectedActor);
    this->HighlightProp(NULL);
    if (selected->GetNumberOfPoints() > 0) {
        this->CurrentRenderer->RemoveActor(vtkwin->selectedActor);
        this->GetInteractor()->GetRenderWindow()->Render();
        vtkwin->setSelectedActor(SelectedActor);
        vtkwin->setVtkInteractorStyle3DPicker(selected);
    }
}

void InteractorStyleSelctionPointOn3DVisualization::SetPoints(vtkSmartPointer<vtkPolyData> points)
{
    this->Points = points;
    this->Points_ori = points;
}

void InteractorStyleSelctionPointOn3DVisualization::setVtkWin(vtkwindow_new *w) { vtkwin = w; }

void MyRubberBand::setVtkWin(vtkwindow_new *w) { vtkwin = w; }

void MyRubberBand::OnMouseMove()
{
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

    sky_coord[0] = world_coord[0];
    sky_coord[1] = world_coord[1];
    sky_coord_gal[0] = world_coord[0];
    sky_coord_gal[1] = world_coord[1];
    sky_coord_fk5[0] = world_coord[0];
    sky_coord_fk5[1] = world_coord[1];

    statusBarText += " <sky> lon: ";
    statusBarText += QString::number(sky_coord[0]);
    statusBarText += " lat: ";
    statusBarText += QString::number(sky_coord[1]);
    statusBarText += " FK5 ";
    statusBarText += QString::number(sky_coord_fk5[0]);
    statusBarText += " ";
    statusBarText += QString::number(sky_coord_fk5[1]);
    statusBarText += " <gal> lon: ";
    statusBarText += QString::number(sky_coord_gal[0]);
    statusBarText += " lat: ";
    statusBarText += QString::number(sky_coord_gal[1]);

    if (sky_coord != NULL && sky_coord_gal != NULL && sky_coord_fk5 != NULL) {
        vtkwin->statusBar()->showMessage(statusBarText, 3);
    }
    delete[] sky_coord;
    delete[] sky_coord_gal;
    delete[] sky_coord_fk5;

    vtkInteractorStyleRubberBand2D::OnMouseMove();
}

myVtkInteractorStyleImage::myVtkInteractorStyleImage()
{
    mouseButtonDown = false;
    window_init = -1;
    level_init = -1;
    isSlice = false;
}

void myVtkInteractorStyleImage::setVtkWin(vtkwindow_new *w) { vtkwin = w; }
void myVtkInteractorStyleImage::setIsSlice(bool value) { isSlice = value; }

void myVtkInteractorStyleImage::OnMouseMove()
{
    vtkRenderWindowInteractor *rwi = this->Interactor;

    if (this->State == VTKIS_WINDOW_LEVEL || mouseButtonDown) {
        vtkInteractorStyleImage::OnMouseMove();
    } else {
        mouseButtonDown = false;
    }

    if (this->State != VTKIS_NONE) {
        mouseButtonDown = true;
        if (this->CurrentImageProperty) {
            vtkImageProperty *property = this->CurrentImageProperty;
            if (!vtkwin->image_init_window_level.contains(property)) {
                vtkwin->image_init_window_level.insert(property, property->GetColorWindow());
            }
            if (!vtkwin->image_init_color_level.contains(property)) {
                vtkwin->image_init_color_level.insert(property, property->GetColorLevel());
            }
        }
        vtkInteractorStyleImage::OnMouseMove();
    }

    vtkSmartPointer<vtkCoordinate> coordinate = vtkSmartPointer<vtkCoordinate>::New();
    coordinate->SetCoordinateSystemToDisplay();
    coordinate->SetValue(rwi->GetEventPosition()[0], rwi->GetEventPosition()[1], 0);

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

    sky_coord[0] = world_coord[0];
    sky_coord[1] = world_coord[1];
    sky_coord_gal[0] = world_coord[0];
    sky_coord_gal[1] = world_coord[1];
    sky_coord_fk5[0] = world_coord[0];
    sky_coord_fk5[1] = world_coord[1];

    statusBarText += " <sky> lon: ";
    statusBarText += QString::number(sky_coord[0]);
    statusBarText += " lat: ";
    statusBarText += QString::number(sky_coord[1]);
    statusBarText += " FK5 ";
    statusBarText += QString::number(sky_coord_fk5[0]);
    statusBarText += " ";
    statusBarText += QString::number(sky_coord_fk5[1]);
    statusBarText += " <gal> lon: ";
    statusBarText += QString::number(sky_coord_gal[0]);
    statusBarText += " lat: ";
    statusBarText += QString::number(sky_coord_gal[1]);

    if (sky_coord != NULL && sky_coord_gal != NULL && sky_coord_fk5 != NULL) {
        vtkwin->statusBar()->showMessage(statusBarText, 3);
    }
    delete[] sky_coord;
    delete[] sky_coord_gal;
    delete[] sky_coord_fk5;
}

void myVtkInteractorStyleImage::OnLeftButtonDown()
{
    mouseButtonDown = true;
    vtkInteractorStyleImage::OnLeftButtonDown();
}

void myVtkInteractorStyleImage::OnLeftButtonUp()
{
    mouseButtonDown = false;
    vtkInteractorStyleImage::OnLeftButtonUp();
}

void myVtkInteractorStyleImage::OnMiddleButtonDown()
{
    mouseButtonDown = true;
    vtkInteractorStyleImage::OnMiddleButtonDown();
}

void myVtkInteractorStyleImage::OnMiddleButtonUp()
{
    mouseButtonDown = false;
    vtkInteractorStyleImage::OnMiddleButtonUp();
}

void myVtkInteractorStyleImage::OnChar()
{
    vtkRenderWindowInteractor *rwi = this->Interactor;

    switch (rwi->GetKeyCode()) {
    case 'r':
    case 'R':
        if (rwi->GetShiftKey() || rwi->GetControlKey()) {
            this->Superclass::OnChar();
        } else if (this->HandleObservers && this->HasObserver(vtkCommand::ResetWindowLevelEvent)) {
            this->InvokeEvent(vtkCommand::ResetWindowLevelEvent, this);
        } else if (this->CurrentImageProperty) {
            vtkImageProperty *property = this->CurrentImageProperty;
            property->SetColorWindow(vtkwin->image_init_window_level.value(property));
            property->SetColorLevel(vtkwin->image_init_color_level.value(property));
            this->Interactor->Render();
        }
        break;
    }
}

SkyRegionSelector::SkyRegionSelector()
{
    is3D = false;
    isFilament = false;
    is3dSelections = false;
    isBubble = false;
}

void SkyRegionSelector::setIsFilament() { isFilament = true; }
void SkyRegionSelector::setIsBubble() { isBubble = true; }
void SkyRegionSelector::setIs3dSelections() { is3dSelections = true; }
void SkyRegionSelector::setIs3D() { is3D = true; }
void SkyRegionSelector::setVtkWin(vtkwindow_new *w) { vtkwin = w; }
vtkwindow_new *SkyRegionSelector::getVtkWin() { return vtkwin; }

void SkyRegionSelector::OnLeftButtonUp()
{
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

    double coor_start[2] = { world_start[0], world_start[1] };
    double coor_end[2] = { world_end[0], world_end[1] };

    if (vtkwin != NULL) {
        // In assenza di API dedicate salviamo solo via query composer oppure DB query
        vtkwin->setVtkInteractorStyleImage();

        if (isFilament || is3dSelections || isBubble) {
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
}

void InteractorStyleExtractSources::OnLeftButtonUp()
{
    this->Superclass::OnLeftButtonUp();

    if (this->Callback) {
        int rect[4] = { this->StartPosition[0], this->StartPosition[1], this->EndPosition[0],
                        this->EndPosition[1] };
        Callback(rect);
    }
}

InteractorStyleEditSource::InteractorStyleEditSource()
{
    this->ActorPicker = vtkSmartPointer<vtkPropPicker>::New();
    this->PointPicker = vtkSmartPointer<vtkPointPicker>::New();
    this->Points = vtkSmartPointer<vtkPoints>::New();
    this->Actor = nullptr;
    this->ActorFilter = nullptr;
    this->Renderer = nullptr;
    this->moving = false;
    this->PointId = -1;
}

void InteractorStyleEditSource::OnMouseMove()
{
    if (!this->Interactor || !moving) {
        return;
    }

    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];
    this->Coordinate = vtkSmartPointer<vtkCoordinate>::New();
    this->Coordinate->SetCoordinateSystemToDisplay();
    this->Coordinate->SetValue(x, y);
    double *coords = this->Coordinate->GetComputedWorldValue(this->CurrentRenderer);
    this->Points->SetPoint(PointId, coords);
    this->Points->Modified();
    this->Interactor->Render();
}

void InteractorStyleEditSource::OnLeftButtonDown()
{
    this->moving = false;

    if (!this->Interactor) {
        return;
    }

    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];
    this->FindPokedRenderer(x, y);
    if (!this->CurrentRenderer) {
        return;
    }
    this->ActorPicker->PickProp(x, y, this->CurrentRenderer);
    auto actor = vtkLODActor::SafeDownCast(this->ActorPicker->GetViewProp());
    if (!actor || (actor != this->ActorFilter && actor != this->Actor)) {
        return;
    }
    if (this->PointPicker->Pick(x, y, 0, this->CurrentRenderer) == 0) {
        return;
    }
    PointId = this->PointPicker->GetPointId();
    this->moving = true;
}

void InteractorStyleEditSource::OnLeftButtonUp()
{
    this->moving = false;
    if (!this->Interactor) {
        return;
    }
}

void InteractorStyleEditSource::SetSource(vtkSmartPointer<vtkPoints> points,
                                          vtkSmartPointer<vtkLODActor> actor)
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
    this->ActorFilter->SetMapper(mapperFilter);
}

void InteractorStyleEditSource::setActorFilter(vtkLODActor *act) { this->ActorFilter = act; }

void InteractorStyleEditSource::setActorPicker(vtkPropPicker *actorPicker)
{
    this->ActorPicker = actorPicker;
}

void InteractorStyleEditSource::setPointPicker(vtkPointPicker *pointPicker)
{
    this->PointPicker = pointPicker;
}

void InteractorStyleEditSource::setRenderer(vtkRenderer *ren) { this->Renderer = ren; }

void InteractorStyleEditSource::SetRenderWindow(vtkRenderWindow *rw)
{
    this->RenderWindow = rw;
}
