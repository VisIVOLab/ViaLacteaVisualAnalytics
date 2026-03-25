#ifndef MomentProcessingService_h
#define MomentProcessingService_h

#include <array>

class vtkLookupTable;
class vtkMomentMapFilter;

struct MomentMapRequest
{
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
    MomentProcessingService(vtkMomentMapFilter *moment, vtkLookupTable *lutMoment);

    MomentMapResult process(const MomentMapRequest &request) const;

private:
    vtkMomentMapFilter *moment;
    vtkLookupTable *lutMoment;
};

#endif
