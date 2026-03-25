#ifndef CubeViewController_h
#define CubeViewController_h

#include <array>

class AstroUtils;
class vtkActor;
class vtkExtractVOI;
class vtkFlyingEdges2D;
class vtkFlyingEdges3D;
class vtkFITSReader;
class vtkImageReslice;
class vtkLegendScaleActorWCS;
class vtkLookupTable;
class vtkPiecewiseFunction;

struct CubeViewContext
{
    vtkFITSReader *reader;
    const AstroUtils &astro;
    vtkFlyingEdges3D *isosurfaceFilter;
    vtkPiecewiseFunction *volumeOpacity;
    vtkImageReslice *slice;
    vtkExtractVOI *sliceOnCube;
    vtkLookupTable *lutSlice;
    vtkLookupTable *lutSliceOnCube;
    vtkFlyingEdges2D *contours;
    vtkActor *contoursActor;
    vtkLegendScaleActorWCS *legendSlice;
    vtkLegendScaleActorWCS *legendMoment;
};

class CubeViewController
{
public:
    struct SliceUpdateResult
    {
        bool valid;
        double spectralValue;
        std::array<double, 2> imageRange;
    };

    explicit CubeViewController(const CubeViewContext &context);

    void updateCube(double threshold) const;
    SliceUpdateResult updateSlice(int sliceIndex) const;
    void setContoursVisible(bool visible) const;
    void updateContours(int levels, double lowerBound, double upperBound) const;
    void setLegendWcs(int wcs) const;

private:
    CubeViewContext context;
};

#endif
