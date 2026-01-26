#include "vtkInteractorStyleProfile.h"

#include <vtkActor.h>
#include <vtkCoordinate.h>
#include <vtkLineSource.h>
#include <vtkNamedColors.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

#include <algorithm>

vtkStandardNewMacro(vtkInteractorStyleProfile);

//----------------------------------------------------------------------------
vtkInteractorStyleProfile::vtkInteractorStyleProfile()
{
    this->Draw = true;
    this->LiveMode = false;
    this->Lengths[0] = this->Lengths[1] = 0;
    this->WorldCoords[0] = this->WorldCoords[1] = 0.;
    this->Callback = nullptr;

    vtkNew<vtkNamedColors> colors;
    double *color = colors->GetColor3d("Peacock").GetData();

    this->Coordinate->SetCoordinateSystemToDisplay();

    // Line through X
    this->LineX->SetPoint1(0., 0., 0.);
    this->LineX->SetPoint2(1., 0., 0.);
    vtkNew<vtkPolyDataMapper> mapperX;
    mapperX->SetInputConnection(this->LineX->GetOutputPort());
    this->ActorX->SetMapper(mapperX);
    this->ActorX->GetProperty()->SetLineWidth(1.f);
    this->ActorX->GetProperty()->SetColor(color);

    // Line through Y
    this->LineY->SetPoint1(0., 0., 0.);
    this->LineY->SetPoint2(0., 1., 0.);
    vtkNew<vtkPolyDataMapper> mapperY;
    mapperY->SetInputConnection(this->LineY->GetOutputPort());
    this->ActorY->SetMapper(mapperY);
    this->ActorY->GetProperty()->SetLineWidth(1.f);
    this->ActorY->GetProperty()->SetColor(color);
}

//----------------------------------------------------------------------------
vtkInteractorStyleProfile::~vtkInteractorStyleProfile()
{
    const int n = this->ActorX->GetNumberOfConsumers();
    for (int i = 0; i < n; ++i) {
        auto ren = vtkRenderer::SafeDownCast(this->ActorX->GetConsumer(i));
        if (ren) {
            ren->RemoveActor(this->ActorX);
            ren->RemoveActor(this->ActorY);
        }
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleProfile::PrintSelf(ostream &os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);
    os << std::boolalpha;
    os << indent << "Draw: " << this->Draw << "\n";
    os << indent << "Live Mode: " << this->LiveMode << "\n";
    os << indent << "Lengths: " << this->Lengths[0] << " " << this->Lengths[1] << "\n";
    os << indent << "Coords: " << this->WorldCoords[0] << " " << this->WorldCoords[1] << "\n";
    os << std::noboolalpha;
}

//----------------------------------------------------------------------------
void vtkInteractorStyleProfile::OnMouseMove()
{
    this->Superclass::OnMouseMove();

    if (this->Draw || this->LiveMode) {
        const int *displayCoords = this->GetLastPos();
        auto renderer = this->Interactor->FindPokedRenderer(displayCoords[0], displayCoords[1]);

        this->Coordinate->SetValue(displayCoords[0], displayCoords[1], 0.);
        const double *coords = this->Coordinate->GetComputedWorldValue(renderer);
        this->WorldCoords[0] = std::clamp(coords[0], 0., static_cast<double>(this->Lengths[0]));
        this->WorldCoords[1] = std::clamp(coords[1], 0., static_cast<double>(this->Lengths[1]));

        this->LineX->SetPoint1(0., this->WorldCoords[1], 0.);
        this->LineX->SetPoint2(this->Lengths[0], this->WorldCoords[1], 0.);

        this->LineY->SetPoint1(this->WorldCoords[0], 0., 0.);
        this->LineY->SetPoint2(this->WorldCoords[0], this->Lengths[1], 0.);

        renderer->AddViewProp(this->ActorX);
        renderer->AddViewProp(this->ActorY);
        this->Interactor->Render();

        if (this->LiveMode) {
            this->Callback(this->WorldCoords[0], this->WorldCoords[1], this->LiveMode);
        }
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleProfile::OnLeftButtonDown()
{
    this->Superclass::OnLeftButtonDown();
    this->Draw = false;
    this->LiveMode = false;

    this->Callback(this->WorldCoords[0], this->WorldCoords[1], this->LiveMode);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleProfile::SetCallback(const std::function<void(double, double, bool)> &cb)
{
    this->Callback = cb;
}
