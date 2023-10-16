#include "vtkinteractorstyledrawarrow.h"

#include <vtkCoordinate.h>
#include <vtkLeaderActor2D.h>
#include <vtkNamedColors.h>
#include <vtkObjectFactory.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

vtkStandardNewMacro(vtkInteractorStyleDrawArrow);

//----------------------------------------------------------------------------
vtkInteractorStyleDrawArrow::vtkInteractorStyleDrawArrow()
    : Drawing(false), Start { 0, 0, 0 }, End { 0, 0, 0 }
{
    vtkNew<vtkNamedColors> colors;
    this->Coordinate->SetCoordinateSystemToDisplay();
    this->Actor->SetArrowPlacementToPoint2();
    this->Actor->GetProperty()->SetColor(colors->GetColor3d("Red").GetData());
    this->Actor->GetPositionCoordinate()->SetCoordinateSystemToWorld();
    this->Actor->GetPosition2Coordinate()->SetCoordinateSystemToWorld();
}

//----------------------------------------------------------------------------
vtkInteractorStyleDrawArrow::~vtkInteractorStyleDrawArrow()
{
    std::for_each(this->Renderers.begin(), this->Renderers.end(), [=](vtkRenderer *r) {
        r->RemoveActor(this->Actor);
        if (r->GetRenderWindow()) {
            r->GetRenderWindow()->Render();
        }
    });
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDrawArrow::PrintSelf(std::ostream &os, vtkIndent indent)
{
    Superclass::PrintSelf(os, indent);
    os << indent << "Drawing:" << std::boolalpha << this->Drawing << '\n';
    os << indent << "Start:\n";
    os << indent << indent << this->Start[0] << ' ' << this->Start[1] << ' ' << this->Start[2]
       << '\n';
    os << indent << "End:\n";
    os << indent << indent << this->End[0] << ' ' << this->End[1] << ' ' << this->End[2];
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDrawArrow::OnLeftButtonDown()
{
    Superclass::OnLeftButtonDown();

    int *coords = this->Interactor->GetEventPosition();
    this->Coordinate->SetValue(coords[0], coords[1], 0);
    auto renderer = this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
    double *worldCoords = this->Coordinate->GetComputedWorldValue(renderer);
    std::copy_n(worldCoords, 3, this->Start);
    this->Actor->GetPositionCoordinate()->SetValue(this->Start);
    this->Drawing = true;
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDrawArrow::OnMouseMove()
{
    Superclass::OnMouseMove();

    if (!this->Drawing) {
        return;
    }

    int *coords = this->Interactor->GetEventPosition();
    this->Coordinate->SetValue(coords[0], coords[1], 0);
    auto renderer = this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
    double *worldCoords = this->Coordinate->GetComputedWorldValue(renderer);
    std::copy_n(worldCoords, 3, this->End);
    this->Actor->GetPosition2Coordinate()->SetValue(this->End);

    this->Renderers.insert(renderer);
    renderer->AddActor(this->Actor);
    this->Interactor->GetRenderWindow()->Render();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDrawArrow::OnLeftButtonUp()
{
    Superclass::OnLeftButtonUp();
    this->Drawing = false;
    this->Interactor->GetRenderWindow()->Render();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDrawArrow::OnKeyPress()
{
    Superclass::OnKeyPress();
    std::string key = this->Interactor->GetKeySym();

    if (key == "Escape") {
        this->AbortCallback();
        return;
    }

    if (key == "Return") {
        this->Callback(this->Start[0], this->Start[1], this->End[0], this->End[1]);
        return;
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDrawArrow::SetAbortCallback(const std::function<void()> &cb)
{
    this->AbortCallback = cb;
}

//----------------------------------------------------------------------------
void vtkInteractorStyleDrawArrow::SetCallback(
        const std::function<void(float, float, float, float)> &cb)
{
    this->Callback = cb;
}
