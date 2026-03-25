#ifndef MomentMapComputeTask_h
#define MomentMapComputeTask_h

#include <vtkSmartPointer.h>

#include <QString>

#include <array>

class vtkImageData;

struct MomentMapComputeRequest
{
    QString filepath;
    int momentOrder;
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
