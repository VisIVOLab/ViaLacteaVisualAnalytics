#include "MomentMapComputeTask.h"

#include "AstroUtils.h"
#include "MomentProcessingService.h"
#include "vtkFITSReader.h"
#include "vtkMomentMapFilter.h"

#include <QDebug>
#include <QElapsedTimer>

#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace {
bool isSupportedMomentOrder(int order)
{
    switch (order) {
    case 0:
    case 1:
    case 2:
    case 6:
    case 8:
    case 10:
        return true;
    default:
        return false;
    }
}

MomentMapComputeResult computeMomentMapLocalWithConfig(const MomentMapComputeRequest &request)
{
    QElapsedTimer totalTimer;
    totalTimer.start();
    MomentMapComputeResult result;

    vtkNew<vtkFITSReader> reader;
    reader->SetFileName(request.filepath.toUtf8());
    QElapsedTimer readTimer;
    readTimer.start();
    reader->Update();
    qDebug().noquote()
            << QStringLiteral("[perf][moment] FITS read: %1 ms").arg(readTimer.elapsed());

    auto *input = reader->GetOutput();
    if (!input) {
        result.errorMessage = QStringLiteral("Could not read cube data.");
        return result;
    }

    const int *dimensions = input->GetDimensions();
    if (!dimensions || dimensions[2] <= 0) {
        result.errorMessage = QStringLiteral("Invalid spectral dimension for moment map.");
        return result;
    }

    const int channelStart = std::clamp(request.channelStart, 0, dimensions[2] - 1);
    const int channelEnd = std::clamp(request.channelEnd, 0, dimensions[2] - 1);
    if (channelStart > channelEnd) {
        result.errorMessage = QStringLiteral("Invalid channel range.");
        return result;
    }

    AstroUtils astro(request.filepath.toStdString());
    const double spectralDelta = std::abs(astro.getIncrements()[2]);
    const double initialSpectralValue = astro.getInitialSpectralValue();

    vtkNew<vtkImageData> image;
    image->SetExtent(0, dimensions[0] - 1, 0, dimensions[1] - 1, 0, 0);
    image->AllocateScalars(VTK_FLOAT, 1);

    float *outPtr = static_cast<float *>(image->GetScalarPointer());
    const auto nels = static_cast<vtkIdType>(dimensions[0]) * dimensions[1];
    std::fill_n(outPtr, nels, std::numeric_limits<float>::quiet_NaN());

    double minValue = std::numeric_limits<double>::infinity();
    double maxValue = -std::numeric_limits<double>::infinity();
    bool anyValid = false;

    for (int y = 0; y < dimensions[1]; ++y) {
        for (int x = 0; x < dimensions[0]; ++x) {
            const vtkIdType outIndex = static_cast<vtkIdType>(y) * dimensions[0] + x;
            double sum0 = 0.0;
            double weighted1 = 0.0;
            double sumSq = 0.0;
            double maxPixel = -std::numeric_limits<double>::infinity();
            double minPixel = std::numeric_limits<double>::infinity();
            int validCount = 0;

            for (int z = channelStart; z <= channelEnd; ++z) {
                const double value = input->GetScalarComponentAsDouble(x, y, z, 0);
                if (!std::isfinite(value)) {
                    continue;
                }
                if (request.maskEnabled && value < request.thresholdValue) {
                    continue;
                }

                const double spectralValue = initialSpectralValue + spectralDelta * z;
                sum0 += value * spectralDelta;
                weighted1 += spectralValue * value * spectralDelta;
                sumSq += value * value;
                maxPixel = std::max(maxPixel, value);
                minPixel = std::min(minPixel, value);
                ++validCount;
            }

            if (validCount <= 0) {
                continue;
            }

            double outputValue = std::numeric_limits<double>::quiet_NaN();
            switch (request.momentOrder) {
            case 0:
                outputValue = sum0;
                break;
            case 1:
                outputValue = sum0 != 0.0 ? (weighted1 / sum0) : std::numeric_limits<double>::quiet_NaN();
                break;
            case 2: {
                if (sum0 == 0.0) {
                    break;
                }
                const double moment1 = weighted1 / sum0;
                double varianceAccumulator = 0.0;
                for (int z = channelStart; z <= channelEnd; ++z) {
                    const double value = input->GetScalarComponentAsDouble(x, y, z, 0);
                    if (!std::isfinite(value)) {
                        continue;
                    }
                    if (request.maskEnabled && value < request.thresholdValue) {
                        continue;
                    }
                    const double spectralValue = initialSpectralValue + spectralDelta * z;
                    varianceAccumulator += value * (spectralValue - moment1) * (spectralValue - moment1)
                            * spectralDelta;
                }
                outputValue = varianceAccumulator / sum0;
                break;
            }
            case 6:
                outputValue = std::sqrt(sumSq / static_cast<double>(validCount));
                break;
            case 8:
                outputValue = maxPixel;
                break;
            case 10:
                outputValue = minPixel;
                break;
            default:
                break;
            }

            if (!std::isfinite(outputValue)) {
                continue;
            }

            outPtr[outIndex] = static_cast<float>(outputValue);
            minValue = std::min(minValue, outputValue);
            maxValue = std::max(maxValue, outputValue);
            anyValid = true;
        }
    }

    if (!anyValid) {
        result.errorMessage = QStringLiteral("No valid voxels found for the selected moment parameters.");
        return result;
    }

    result.valid = true;
    result.imageData = image;
    result.imageRange = { minValue, maxValue };
    qDebug().noquote()
            << QStringLiteral("[perf][moment] worker total: %1 ms").arg(totalTimer.elapsed());
    return result;
}
}

MomentMapComputeResult computeMomentMap(const MomentMapComputeRequest &request)
{
    QElapsedTimer totalTimer;
    totalTimer.start();
    MomentMapComputeResult result;

    const bool remoteMode = !request.datasetId.isEmpty();
    const auto mode = remoteMode ? MomentComputeMode::Remote : MomentComputeMode::Local;

    if (!remoteMode && request.filepath.isEmpty()) {
        result.errorMessage = "Empty dataset path.";
        return result;
    }

    if (!isSupportedMomentOrder(request.momentOrder)) {
        result.errorMessage = "Unsupported moment order.";
        return result;
    }

    if (remoteMode) {
        vtkLookupTable *lutMoment = nullptr;
        vtkMomentMapFilter *moment = nullptr;
        MomentProcessingService processing(moment, lutMoment, mode);
        QElapsedTimer computeTimer;
        computeTimer.start();
        const auto processed = processing.computeMoment(
                MomentRequest { request.filepath, request.datasetId, request.backendUrl,
                                request.sessionId, request.backendToken,
                                request.momentOrder, request.channelStart, request.channelEnd,
                                request.maskEnabled, request.thresholdValue });
        qDebug().noquote()
                << QStringLiteral("[perf][moment] worker compute: %1 ms").arg(computeTimer.elapsed());
        if (!processed.valid || !processed.image) {
            result.errorMessage = processed.error.isEmpty() ? QStringLiteral("Remote moment failed.")
                                                            : processed.error;
            return result;
        }

        result.valid = true;
        result.imageRange = processed.imageRange;
        result.imageData = processed.image;
        qDebug().noquote()
                << QStringLiteral("[perf][moment] worker total: %1 ms").arg(totalTimer.elapsed());
        return result;
    }

    return computeMomentMapLocalWithConfig(request);
}
