#include "CubeViewController.h"

#include "AstroUtils.h"
#include "vtkFITSReader.h"

#include <vtkActor.h>
#include <vtkExtractVOI.h>
#include <vtkFlyingEdges2D.h>
#include <vtkFlyingEdges3D.h>
#include <vtkImageData.h>
#include <vtkImageReslice.h>
#include <vtkLegendScaleActorWCS.h>
#include <vtkLookupTable.h>
#include <vtkPiecewiseFunction.h>

CubeViewController::CubeViewController(const CubeViewContext &context) : context(context) { }

void CubeViewController::updateCube(double threshold) const
{
    this->context.isosurfaceFilter->SetValue(0, threshold);
    this->context.volumeOpacity->RemoveAllPoints();
    this->context.volumeOpacity->AddPoint(this->context.reader->GetMin(), 0.0);
    this->context.volumeOpacity->AddPoint(threshold, 0.05);
    this->context.volumeOpacity->AddPoint(this->context.reader->GetMax(), 0.3);
}

CubeViewController::SliceUpdateResult CubeViewController::updateSlice(int sliceIndex) const
{
    const int *extentData = this->context.reader->GetDataExtent();
    if (sliceIndex < extentData[4] || sliceIndex > extentData[5]) {
        return { false, 0., { 0., 0. } };
    }

    int extent[6];
    this->context.reader->GetDataExtent(extent);
    extent[4] = extent[5] = sliceIndex;
    this->context.sliceOnCube->SetVOI(extent);
    this->context.sliceOnCube->Update();

    const double *imgRange = this->context.sliceOnCube->GetOutput()->GetScalarRange();
    this->context.lutSliceOnCube->SetTableRange(imgRange);

    this->context.slice->SetResliceAxesOrigin(0., 0., sliceIndex);
    this->context.lutSlice->SetTableRange(this->context.lutSliceOnCube->GetTableRange());

    const double spectralValue = this->context.astro.getInitialSpectralValue()
            + this->context.astro.getIncrements()[2] * sliceIndex;

    return { true, spectralValue, { imgRange[0], imgRange[1] } };
}

void CubeViewController::setContoursVisible(bool visible) const
{
    this->context.contoursActor->SetVisibility(visible);
}

void CubeViewController::updateContours(int levels, double lowerBound, double upperBound) const
{
    this->context.contours->GenerateValues(levels, lowerBound, upperBound);
}

void CubeViewController::setLegendWcs(int wcs) const
{
    this->context.legendSlice->SetWCS(wcs);
    this->context.legendMoment->SetWCS(wcs);
}
