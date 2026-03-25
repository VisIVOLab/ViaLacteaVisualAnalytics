#include "CubeViewController.h"

#include "AstroUtils.h"
#include "ColorMaps.h"

#include <vtkActor.h>
#include <vtkExtractVOI.h>
#include <vtkFlyingEdges2D.h>
#include <vtkFlyingEdges3D.h>
#include <vtkImageData.h>
#include <vtkImageReslice.h>
#include <vtkLegendScaleActorWCS.h>
#include <vtkLookupTable.h>
#include <vtkMomentMapFilter.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderer.h>
#include <vtkTrivialProducer.h>
#include <vtkVolume.h>

CubeViewController::CubeViewController(const CubeViewContext &context) : context(context) { }

namespace {
vtkImageData *currentCubeData(const CubeViewContext &context)
{
    return vtkImageData::SafeDownCast(context.cubeDisplaySource->GetOutputDataObject(0));
}
}

void CubeViewController::updateCube(double threshold) const
{
    auto *cubeData = currentCubeData(this->context);
    if (!cubeData) {
        return;
    }

    double cubeRange[2];
    cubeData->GetScalarRange(cubeRange);

    this->context.isosurfaceFilter->SetValue(0, threshold);
    this->context.volumeOpacity->RemoveAllPoints();
    this->context.volumeOpacity->AddPoint(cubeRange[0], 0.0);
    this->context.volumeOpacity->AddPoint(threshold, 0.05);
    this->context.volumeOpacity->AddPoint(cubeRange[1], 0.3);
}

CubeViewController::SliceUpdateResult CubeViewController::updateSlice(int sliceIndex) const
{
    auto *cubeData = currentCubeData(this->context);
    if (!cubeData) {
        return { false, 0., { 0., 0. } };
    }

    int extentData[6];
    cubeData->GetExtent(extentData);
    if (sliceIndex < extentData[4] || sliceIndex > extentData[5]) {
        return { false, 0., { 0., 0. } };
    }

    int extent[6] = { extentData[0], extentData[1], extentData[2],
                      extentData[3], extentData[4], extentData[5] };
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

CubeViewController::MomentUpdateResult CubeViewController::updateMomentOrder(int order) const
{
    switch (order) {
    case 0:
    case 1:
    case 2:
    case 6:
    case 8:
    case 10:
        break;
    default:
        return { false, { 0., 0. } };
    }

    this->context.moment->SetMomentOrder(order);
    this->context.moment->Update();
    const double *range = this->context.moment->GetOutput()->GetScalarRange();
    this->context.lutMoment->SetTableRange(range);
    return { true, { range[0], range[1] } };
}

void CubeViewController::syncSlicesLut() const
{
    double range[2];
    this->context.lutSlice->GetTableRange(range);

    ColorMaps::SetColorMap(this->context.lutSliceOnCube, this->context.lutSlice->GetObjectName());
    this->context.lutSliceOnCube->SetTableRange(range);
    this->context.lutSliceOnCube->SetScale(this->context.lutSlice->GetScale());
}

void CubeViewController::setCubeRenderMode(bool isosurfaceMode) const
{
    if (isosurfaceMode) {
        this->context.cubeRenderer->AddViewProp(this->context.isosurface);
        this->context.cubeRenderer->RemoveViewProp(this->context.volume);
    } else {
        this->context.cubeRenderer->AddViewProp(this->context.volume);
        this->context.cubeRenderer->RemoveViewProp(this->context.isosurface);
    }
}
