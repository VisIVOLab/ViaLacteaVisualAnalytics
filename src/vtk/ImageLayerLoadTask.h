#ifndef ImageLayerLoadTask_h
#define ImageLayerLoadTask_h

#include <vtkSmartPointer.h>

#include <array>
#include <string>

class vtkImageData;

struct ImageLayerLoadRequest
{
    std::string masterFilepath;
    std::string layerFilepath;
};

struct ImageLayerLoadResult
{
    bool valid{ false };
    std::string errorMessage;
    std::string filepath;
    vtkSmartPointer<vtkImageData> imageData;
    std::array<double, 2> scalarRange{ 0., 0. };
    std::array<double, 3> spacing{ 1., 1., 1. };
    std::array<double, 3> origin{ 0., 0., 0. };
    double rotationDegrees{ 0. };
    bool isPreview{ false };
    int fullWidth{ 0 };
    int fullHeight{ 0 };
    double previewScaleFactor{ 1.0 };
    int requestGeneration{ 0 };
};

ImageLayerLoadResult loadImageLayer(const ImageLayerLoadRequest &request);

#endif
