#include "vtkLegendScaleActorWCS.h"

#include "AstroUtils.h"
#include "wcs.h"

#include <vtkAxisActor2D.h>
#include <vtkObjectFactory.h>
#include <vtkTextProperty.h>

vtkStandardNewMacro(vtkLegendScaleActorWCS);

//----------------------------------------------------------------------------
vtkLegendScaleActorWCS::vtkLegendScaleActorWCS()
{
    this->Astro = nullptr;
    this->WCS = 0;

    this->SetLabelModeToCoordinates();
    this->LegendVisibilityOff();
    this->TopAxisVisibilityOff();
    this->RightAxisVisibilityOff();
    this->SetTopBorderOffset(100);
    this->SetBottomBorderOffset(100);
    this->SetLeftBorderOffset(100);
    this->SetRightBorderOffset(100);

    this->BottomAxis->GetTitleTextProperty()->SetFontSize(20);
    this->BottomAxis->UseFontSizeFromPropertyOn();

    this->LeftAxis->GetTitleTextProperty()->SetOrientation(90.);
    this->LeftAxis->GetTitleTextProperty()->SetFontSize(20);
    this->LeftAxis->UseFontSizeFromPropertyOn();
}

//----------------------------------------------------------------------------
vtkLegendScaleActorWCS::~vtkLegendScaleActorWCS() = default;

//----------------------------------------------------------------------------
void vtkLegendScaleActorWCS::PrintSelf(ostream &os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);
    os << indent << "WCS: " << this->WCS << "\n";
}

//----------------------------------------------------------------------------
void vtkLegendScaleActorWCS::SetWCS(int wcs)
{
    if (this->Astro->isSimulation()) {
        return;
    }

    if (this->WCS != wcs) {
        this->WCS = wcs;
        if (this->WCS == WCS_GALACTIC || this->WCS == WCS_ECLIPTIC) {
            this->BottomAxis->SetTitle("Longitude");
            this->LeftAxis->SetTitle("Latitude");

        } else {
            this->BottomAxis->SetTitle("Right Ascension");
            this->LeftAxis->SetTitle("Declination");
        }
        this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkLegendScaleActorWCS::Init(const std::string &filepath)
{
    this->Astro = std::make_unique<AstroUtils>(filepath);
}

//----------------------------------------------------------------------------
void vtkLegendScaleActorWCS::BuildRepresentation(vtkViewport *viewport)
{
    this->Superclass::BuildRepresentation(viewport);

    if (this->Astro->isSimulation()) {
        return;
    }

    double pos1[2];
    double pos2[2];

    const double *xL = this->BottomAxis->GetPositionCoordinate()->GetComputedWorldValue(viewport);
    this->Astro->xy2sky(xL, pos1, this->WCS);
    const double *xR = this->BottomAxis->GetPosition2Coordinate()->GetComputedWorldValue(viewport);
    this->Astro->xy2sky(xR, pos2, this->WCS);
    this->BottomAxis->SetRange(pos1[0], pos2[0]);

    xL = this->LeftAxis->GetPositionCoordinate()->GetComputedWorldValue(viewport);
    this->Astro->xy2sky(xL, pos1, this->WCS);
    xR = this->LeftAxis->GetPosition2Coordinate()->GetComputedWorldValue(viewport);
    this->Astro->xy2sky(xR, pos2, this->WCS);
    this->LeftAxis->SetRange(pos1[1], pos2[1]);

    this->BuildTime.Modified();
}
