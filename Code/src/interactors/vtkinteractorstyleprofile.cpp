#include "vtkinteractorstyleprofile.h"

#include <vtkActor.h>
#include <vtkLineSource.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

vtkStandardNewMacro(vtkInteractorStyleProfile);

//----------------------------------------------------------------------------
vtkInteractorStyleProfile::vtkInteractorStyleProfile()
{
    this->LiveMode = false;

    vtkNew<vtkNamedColors> colors;

    // Line X
    lineX = vtkSmartPointer<vtkLineSource>::New();
    vtkNew<vtkPolyDataMapper> mapperX;
    mapperX->SetInputConnection(lineX->GetOutputPort());
    actorX = vtkSmartPointer<vtkActor>::New();
    actorX->SetMapper(mapperX);
    actorX->GetProperty()->SetLineWidth(1);
    actorX->GetProperty()->SetColor(colors->GetColor3d("Peacock").GetData());

    // Line Y
    lineY = vtkSmartPointer<vtkLineSource>::New();
    vtkNew<vtkPolyDataMapper> mapperY;
    mapperY->SetInputConnection(lineY->GetOutputPort());
    actorY = vtkSmartPointer<vtkActor>::New();
    actorY->SetMapper(mapperY);
    actorY->GetProperty()->SetLineWidth(1);
    actorY->GetProperty()->SetColor(colors->GetColor3d("Peacock").GetData());
}

//----------------------------------------------------------------------------
vtkInteractorStyleProfile::~vtkInteractorStyleProfile()
{
    std::for_each(renderers.begin(), renderers.end(), [=](vtkRenderer *r) {
        r->RemoveActor(actorX);
        r->RemoveActor(actorY);
        if (r->GetRenderWindow()) {
            r->GetRenderWindow()->Render();
        }
    });
}

//----------------------------------------------------------------------------
void vtkInteractorStyleProfile::PrintSelf(std::ostream &os, vtkIndent indent)
{
    Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleProfile::OnMouseMove()
{
    Superclass::OnMouseMove();

    int *coords = this->Interactor->GetEventPosition();
    this->Coordinate->SetValue(coords[0], coords[1], 0);
    auto renderer = this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
    double *worldCoords = this->Coordinate->GetComputedWorldValue(renderer);

    double *bounds = this->reader->GetBounds();
    bool xInside = worldCoords[0] >= bounds[0] && worldCoords[0] <= bounds[1];
    bool yInside = worldCoords[1] >= bounds[2] && worldCoords[1] <= bounds[3];
    if (xInside && yInside) {
        lineX->SetPoint1(0, worldCoords[1], 0);
        lineX->SetPoint2(bounds[1], worldCoords[1], 0);
        lineX->Update();

        lineY->SetPoint1(worldCoords[0], 0, 0);
        lineY->SetPoint2(worldCoords[0], bounds[3], 0);
        lineY->Update();

        // This is necessary because the renderer may have changed in the meantime
        renderers.insert(renderer);
        renderer->AddActor(actorX);
        renderer->AddActor(actorY);

        if (LiveMode) {
            this->ProfileCb(worldCoords[0], worldCoords[1], LiveMode);
        }
    }

    this->Interactor->GetRenderWindow()->Render();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleProfile::OnLeftButtonDown()
{
    Superclass::OnLeftButtonDown();

    int *coords = this->Interactor->GetEventPosition();
    this->Coordinate->SetValue(coords[0], coords[1], 0);
    auto renderer = this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
    double *worldCoords = this->Coordinate->GetComputedWorldValue(renderer);

    double *bounds = this->reader->GetBounds();
    bool xInside = worldCoords[0] >= bounds[0] && worldCoords[0] <= bounds[1];
    bool yInside = worldCoords[1] >= bounds[2] && worldCoords[1] <= bounds[3];
    if (xInside && yInside) {
        this->LiveMode = false;
        this->ProfileCb(worldCoords[0], worldCoords[1], LiveMode);
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleProfile::SetProfileCallback(
        const std::function<void(double, double, bool)> &ProfileCb)
{
    this->ProfileCb = ProfileCb;
}
