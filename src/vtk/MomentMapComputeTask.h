#ifndef MomentMapComputeTask_h
#define MomentMapComputeTask_h

#include <vtkSmartPointer.h>

#include <QString>

#include <array>

class vtkImageData;

struct MomentMapComputeRequest
{
    QString filepath;
    QString datasetId;
    QString backendUrl;
    QString sessionId;
    QString backendToken;
    int momentOrder;
    int channelStart{ 0 };
    int channelEnd{ 0 };
    bool maskEnabled{ false };
    double thresholdValue{ 0.0 };
};

struct MomentMapComputeResult
{
    bool valid{ false };
    QString errorMessage;
    vtkSmartPointer<vtkImageData> imageData;
    std::array<double, 2> imageRange{ 0., 0. };
};

MomentMapComputeResult computeMomentMap(const MomentMapComputeRequest &request);

#endif
