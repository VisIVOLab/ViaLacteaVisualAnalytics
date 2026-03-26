#include "MomentProcessingService.h"

#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkMomentMapFilter.h>

MomentProcessingService::MomentProcessingService(vtkMomentMapFilter *moment,
                                                 vtkLookupTable *lutMoment)
    : moment(moment), lutMoment(lutMoment)
{
}

MomentResult MomentProcessingService::computeMoment(const MomentRequest &request) const
{
    return this->computeMomentLocal(request);
}

MomentResult MomentProcessingService::computeMomentLocal(const MomentRequest &request) const
{
    switch (request.order) {
    case 0:
    case 1:
    case 2:
    case 6:
    case 8:
    case 10:
        break;
    default:
        return { nullptr, { 0., 0. }, false, QStringLiteral("Unsupported moment order.") };
    }

    this->moment->SetMomentOrder(request.order);
    this->moment->Update();

    const double *range = this->moment->GetOutput()->GetScalarRange();
    this->lutMoment->SetTableRange(range);

    MomentResult result;
    result.valid = true;
    result.imageRange = { range[0], range[1] };
    result.image = vtkSmartPointer<vtkImageData>::New();
    result.image->DeepCopy(this->moment->GetOutput());
    return result;
}

MomentResult MomentProcessingService::computeMomentRemote(const MomentRequest &request) const
{
    return this->computeMomentLocal(request);
}

MomentMapResult MomentProcessingService::process(const MomentMapRequest &request) const
{
    const auto result = this->computeMoment(MomentRequest { {}, request.momentOrder });
    return { result.valid, result.imageRange };
}
