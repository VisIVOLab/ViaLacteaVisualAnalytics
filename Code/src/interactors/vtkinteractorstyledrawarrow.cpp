#include "vtkinteractorstyledrawarrow.h"

#include <vtkActor.h>
#include <vtkArrowSource.h>
#include <vtkCoordinate.h>
#include <vtkMath.h>
#include <vtkMinimalStandardRandomSequence.h>
#include <vtkNamedColors.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTransform.h>

vtkStandardNewMacro(vtkInteractorStyleDrawArrow);

//----------------------------------------------------------------------------
vtkInteractorStyleDrawArrow::vtkInteractorStyleDrawArrow() : Drawing(false)
{
    vtkNew<vtkNamedColors> colors;
    this->Coordinate->SetCoordinateSystemToDisplay();
    this->Actor->GetProperty()->SetColor(colors->GetColor3d("Red").GetData());
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

    // Compute a basis
    double normalizedX[3];
    double normalizedY[3];
    double normalizedZ[3];

    vtkNew<vtkMinimalStandardRandomSequence> rng;
    // The X axis is a vector from start to end
    vtkMath::Subtract(this->End, this->Start, normalizedX);
    double length = vtkMath::Norm(normalizedX);
    vtkMath::Normalize(normalizedX);

    // The Z axis is an arbitrary vector cross X
    double arbitrary[3];
    for (auto i = 0; i < 3; ++i) {
        rng->Next();
        arbitrary[i] = rng->GetRangeValue(-10, 10);
    }
    vtkMath::Cross(normalizedX, arbitrary, normalizedZ);
    vtkMath::Normalize(normalizedZ);

    // The Y axis is Z cross X
    vtkMath::Cross(normalizedZ, normalizedX, normalizedY);
    vtkNew<vtkMatrix4x4> matrix;

    // Create the direction cosine matrix
    matrix->Identity();
    for (auto i = 0; i < 3; i++) {
        matrix->SetElement(i, 0, normalizedX[i]);
        matrix->SetElement(i, 1, normalizedY[i]);
        matrix->SetElement(i, 2, normalizedZ[i]);
    }

    // Apply the transforms
    vtkNew<vtkTransform> transform;
    transform->Translate(this->Start);
    transform->Concatenate(matrix);
    transform->Scale(length, length, length);

    vtkNew<vtkArrowSource> arrow;
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(arrow->GetOutputPort());

    this->Actor->SetMapper(mapper);
    this->Actor->SetUserMatrix(transform->GetMatrix());

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
void vtkInteractorStyleDrawArrow::SetCallback(const std::function<void(int, int, int, int)> &cb)
{
    this->Callback = cb;
}
