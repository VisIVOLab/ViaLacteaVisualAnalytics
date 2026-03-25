#include "MomentProcessingService.h"

#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkMomentMapFilter.h>

MomentProcessingService::MomentProcessingService(vtkMomentMapFilter *moment,
                                                 vtkLookupTable *lutMoment)
    : moment(moment), lutMoment(lutMoment)
{
}

MomentMapResult MomentProcessingService::process(const MomentMapRequest &request) const
{
    switch (request.momentOrder) {
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

    this->moment->SetMomentOrder(request.momentOrder);
    this->moment->Update();

    const double *range = this->moment->GetOutput()->GetScalarRange();
    this->lutMoment->SetTableRange(range);

    return { true, { range[0], range[1] } };
}
