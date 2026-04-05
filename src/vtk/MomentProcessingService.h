#ifndef MomentProcessingService_h
#define MomentProcessingService_h

#include <vtkSmartPointer.h>

#include <QString>

#include <array>

class vtkImageData;
class vtkLookupTable;
class vtkMomentMapFilter;

enum class MomentComputeMode
{
    Local,
    Remote
};

struct MomentRequest
{
    QString datasetPath;
    QString datasetId;
    QString backendUrl;
    QString sessionId;
    QString backendToken;
    int order;
    int channelStart{ 0 };
    int channelEnd{ 0 };
    bool maskEnabled{ false };
    double thresholdValue{ 0.0 };
};

struct MomentResult
{
    vtkSmartPointer<vtkImageData> image;
    std::array<double, 2> imageRange{ 0., 0. };
    bool valid{ false };
    QString error;
    // Scientific metadata from the server-side WCS pipeline.
    QString momentUnit;       // e.g. "Jy/beam Hz", "km/s", "km^2/s^2"
    QString bunit;            // raw BUNIT from the FITS header
    QString spectralAxisType; // CTYPE3 value
    QString spectralAxisUnit; // CUNIT3 value
    QString wcsStatus{ QStringLiteral("ok") }; // "ok", "sanitized", or "degraded"
    QString wcsWarningMessage;                  // human-readable WCS warning, empty when ok
};

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
    MomentProcessingService(vtkMomentMapFilter *moment, vtkLookupTable *lutMoment,
                            MomentComputeMode mode = MomentComputeMode::Local);

    // Dispatches moment computation. Currently always resolves to the local implementation.
    MomentResult computeMoment(const MomentRequest &request) const;

    // Computes the moment using the current local VTK runtime.
    MomentResult computeMomentLocal(const MomentRequest &request) const;

    // Future remote hook. For now this intentionally reuses the local implementation.
    MomentResult computeMomentRemote(const MomentRequest &request) const;

    // Recomputes the moment map in the local VTK runtime and updates the associated LUT range.
    MomentMapResult process(const MomentMapRequest &request) const;

private:
    vtkMomentMapFilter *moment;
    vtkLookupTable *lutMoment;
    MomentComputeMode mode;
};

#endif
