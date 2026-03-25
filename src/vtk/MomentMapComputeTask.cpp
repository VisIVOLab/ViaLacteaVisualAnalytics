#include "MomentMapComputeTask.h"

#include "MomentProcessingService.h"
#include "vtkFITSReader.h"
#include "vtkMomentMapFilter.h"

#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>

MomentMapComputeResult computeMomentMap(const MomentMapComputeRequest &request)
{
    MomentMapComputeResult result;

    if (request.filepath.isEmpty()) {
        result.errorMessage = "Empty dataset path.";
        return result;
    }

    vtkNew<vtkFITSReader> reader;
    reader->SetFileName(request.filepath.toUtf8());
    reader->Update();

    vtkNew<vtkMomentMapFilter> moment;
    moment->SetInputConnection(reader->GetOutputPort());
    moment->Init(request.filepath.toStdString());

    vtkNew<vtkLookupTable> lutMoment;
    MomentProcessingService processing(moment, lutMoment);
    const auto processed = processing.process(MomentMapRequest { request.momentOrder });
    if (!processed.valid) {
        result.errorMessage = "Unsupported moment order.";
        return result;
    }

    result.valid = true;
    result.imageRange = processed.imageRange;
    result.imageData = vtkSmartPointer<vtkImageData>::New();
    result.imageData->DeepCopy(moment->GetOutput());
    return result;
}
