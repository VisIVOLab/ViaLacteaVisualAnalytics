#ifndef CubeOpenPreviewTask_h
#define CubeOpenPreviewTask_h

#include <vtkSmartPointer.h>

#include <QString>

#include <array>

class vtkImageData;

struct CubeOpenStageResult
{
    bool valid{ false };
    QString errorMessage;
    vtkSmartPointer<vtkImageData> cubeImageData;
    vtkSmartPointer<vtkImageData> momentImageData;
    std::array<double, 2> cubeRange{ 0., 0. };
    std::array<double, 2> momentRange{ 0., 0. };
    std::array<int, 6> dataExtent{ 0, -1, 0, -1, 0, -1 };
    double cubeMean{ 0. };
    double cubeRms{ 0. };
};

CubeOpenStageResult loadCubeOpenPreview(const QString &filepath);
CubeOpenStageResult loadCubeOpenFull(const QString &filepath);

#endif
