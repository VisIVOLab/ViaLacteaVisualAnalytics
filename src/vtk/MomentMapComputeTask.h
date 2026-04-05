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
    // Scientific metadata from the server-side WCS pipeline (remote mode only).
    QString momentUnit;       // e.g. "Jy/beam Hz", "km/s", "km^2/s^2"
    QString bunit;            // raw BUNIT from the FITS header
    QString spectralAxisType; // CTYPE3 value
    QString spectralAxisUnit; // CUNIT3 value
    QString wcsStatus{ QStringLiteral("ok") }; // "ok", "sanitized", or "degraded"
    QString wcsWarningMessage;                  // human-readable WCS warning, empty when ok
};

MomentMapComputeResult computeMomentMap(const MomentMapComputeRequest &request);

#endif
