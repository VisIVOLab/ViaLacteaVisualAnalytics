#ifndef MomentProcessingService_h
#define MomentProcessingService_h

#include <array>

class vtkLookupTable;
class vtkMomentMapFilter;

struct MomentMapRequest
{
    // Requested moment order for the current local VTK processing context.
    int momentOrder;
};

struct MomentMapResult
{
    bool valid;
    std::array<double, 2> imageRange;
};

class MomentProcessingService
{
public:
    // Binds the service to the current local VTK runtime objects used by the cube view.
    MomentProcessingService(vtkMomentMapFilter *moment, vtkLookupTable *lutMoment);

    // Recomputes the moment map in the local VTK runtime and updates the associated LUT range.
    MomentMapResult process(const MomentMapRequest &request) const;

private:
    vtkMomentMapFilter *moment;
    vtkLookupTable *lutMoment;
};

#endif
