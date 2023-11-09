#include "vtkdrawlineinteractorstyleimage.h"

#include <vtkNamedColors.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindowInteractor.h>

vtkStandardNewMacro(vtkDrawLineInteractorStyleUser);

vtkDrawLineInteractorStyleUser::vtkDrawLineInteractorStyleUser() : isDrawingPVSliceLine(false), Start { 0, 0, 0 }, End { 0, 0, 0 }
{
    vtkNew<vtkNamedColors> colors;
    this->Coordinate->SetCoordinateSystemToDisplay();
    this->Actor->SetArrowPlacementToPoint2();
    this->Actor->GetProperty()->SetColor(colors->GetColor3d("Red").GetData());
    this->Actor->GetPositionCoordinate()->SetCoordinateSystemToWorld();
    this->Actor->GetPosition2Coordinate()->SetCoordinateSystemToWorld();
    isDrawingPVSliceLine = false;
    startPVSliceLine = endPVSliceLine = QPointF(0, 0);
}

vtkDrawLineInteractorStyleUser::~vtkDrawLineInteractorStyleUser()
{
    std::for_each(this->renderers.begin(), this->renderers.end(), [=](vtkRenderer *r) {
        r->RemoveActor(this->Actor);
        if (r->GetRenderWindow()) {
            r->GetRenderWindow()->Render();
        }
    });
}

void vtkDrawLineInteractorStyleUser::setLineAbortCallback(const std::function<void ()> &newLineAbortCallback)
{
    lineAbortCallback = newLineAbortCallback;
}

void vtkDrawLineInteractorStyleUser::PrintSelf(ostream &os, vtkIndent indent)
{
    Superclass::PrintSelf(os, indent);
}

void vtkDrawLineInteractorStyleUser::OnMouseMove()
{
    Superclass::OnMouseMove();

    if (!this->isDrawingPVSliceLine) {
        return;
    }

    int *coords = this->Interactor->GetEventPosition();
    this->Coordinate->SetValue(coords[0], coords[1], 0);
    auto renderer = this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
    double *worldCoords = this->Coordinate->GetComputedWorldValue(renderer);
    std::copy_n(worldCoords, 3, this->End);
    this->Actor->GetPosition2Coordinate()->SetValue(this->End);

    this->renderers.insert(renderer);
    renderer->AddActor(this->Actor);
    this->Interactor->GetRenderWindow()->Render();
}

void vtkDrawLineInteractorStyleUser::OnLeftButtonDown()
{
    Superclass::OnLeftButtonDown();

    int *coords = this->Interactor->GetEventPosition();
    this->Coordinate->SetValue(coords[0], coords[1], 0);
    auto renderer = this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
    double *worldCoords = this->Coordinate->GetComputedWorldValue(renderer);
    std::copy_n(worldCoords, 3, this->Start);
    this->Actor->GetPositionCoordinate()->SetValue(this->Start);
    this->isDrawingPVSliceLine = true;
}

void vtkDrawLineInteractorStyleUser::OnLeftButtonUp()
{
    Superclass::OnLeftButtonUp();
    this->isDrawingPVSliceLine = false;
    this->Interactor->GetRenderWindow()->Render();
}

void vtkDrawLineInteractorStyleUser::OnKeyPress()
{
    Superclass::OnKeyPress();
    std::string key = this->Interactor->GetKeySym();

    if (key == "Escape") {
        this->lineAbortCallback();
        return;
    }

    if (key == "Return") {
        this->LinePointCallback(this->Start[0], this->Start[1], this->End[0], this->End[1]);
        this->lineAbortCallback();
        return;
    }
}

void vtkDrawLineInteractorStyleUser::setLineValsCallback(const std::function<void (float, float, float, float)> &callback)
{
    LinePointCallback = callback;
}
