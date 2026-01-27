#include "vtkMomentMapFilter.h"

#include "AstroUtils.h"

#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkSMPTools.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <cmath>
#include <iterator>
#include <limits>
#include <mutex>

vtkStandardNewMacro(vtkMomentMapFilter);

//----------------------------------------------------------------------------
vtkMomentMapFilter::vtkMomentMapFilter()
{
    this->Astro = nullptr;
    this->CurrentMoment = nullptr;
    this->MaxId = 0ll;
    this->PointerInc[0] = this->PointerInc[1] = this->PointerInc[2] = 0ll;
    this->MomentOrder = 0;
}

//----------------------------------------------------------------------------
vtkMomentMapFilter::~vtkMomentMapFilter() = default;

//----------------------------------------------------------------------------
void vtkMomentMapFilter::PrintSelf(ostream &os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);
    os << indent << "Order: " << this->MomentOrder << "\n";
    os << indent << "Internals:\n";
    os << indent << indent << "Current Moment: " << this->CurrentMoment << "\n";
    os << indent << indent << "MaxId: " << this->MaxId << "\n";
    os << indent << indent << "PointerIncs:\n";
    os << indent << indent << indent << "X: " << this->PointerInc[0] << "\n";
    os << indent << indent << indent << "Y: " << this->PointerInc[1] << "\n";
    os << indent << indent << indent << "Z: " << this->PointerInc[2] << "\n";

    os << indent << indent << "Moments: ";
    std::transform(this->Moments.cbegin(), this->Moments.cend(),
                   std::ostream_iterator<int>(os, " "), [](const auto &it) { return it.first; });
    os << "\n";
}

//----------------------------------------------------------------------------
void vtkMomentMapFilter::Init(const std::string &filepath)
{
    this->Astro = std::make_unique<AstroUtils>(filepath);
}

//----------------------------------------------------------------------------
float vtkMomentMapFilter::GetValue(int x, int y) const
{
    const vtkIdType idx = 1ll * x * this->PointerInc[0] + y * this->PointerInc[1];
    if (!this->CurrentMoment || idx < 0ll || idx > this->MaxId) {
        return std::numeric_limits<float>::quiet_NaN();
    }
    return this->CurrentMoment[idx];
}

//----------------------------------------------------------------------------
void vtkMomentMapFilter::GenerateMomentMap(int order, const float *inPtr, const int *dimensions)
{
    if (this->Moments.find(order) != this->Moments.end()) {
        return;
    }

    const vtkIdType nels = 1ll * dimensions[0] * dimensions[1];

    switch (order) {
    case 0: {
        std::vector<float> out(nels, std::numeric_limits<float>::quiet_NaN());
        const double spectralDelta = std::abs(this->Astro->getIncrements()[2]);

        std::mutex m;
        vtkSMPTools::For(0, dimensions[2],
                         [this, &m, inPtr, &out, dimensions, nels, spectralDelta](vtkIdType start,
                                                                                  vtkIdType end) {
                             std::vector<float> tmp(nels, std::numeric_limits<float>::quiet_NaN());

                             for (int k = start; k < end; ++k) {
                                 for (int j = 0; j < dimensions[1]; ++j) {
                                     for (int i = 0; i < dimensions[0]; ++i) {
                                         const vtkIdType outIdx = 1ll * i * this->PointerInc[0]
                                                 + j * this->PointerInc[1];
                                         const vtkIdType inIdx = outIdx + k * this->PointerInc[2];
                                         const float pixelValue = inPtr[inIdx];

                                         if (std::isfinite(pixelValue)) {
                                             if (std::isnan(tmp[outIdx])) {
                                                 tmp[outIdx] = 0.f;
                                             }
                                             tmp[outIdx] += pixelValue * spectralDelta;
                                         }
                                     }
                                 }
                             }

                             for (vtkIdType i = 0; i < nels; ++i) {
                                 if (std::isfinite(tmp[i])) {
                                     const std::lock_guard<std::mutex> lk(m);
                                     if (std::isnan(out[i])) {
                                         out[i] = 0.f;
                                     }
                                     out[i] += tmp[i];
                                 }
                             }
                         });

        this->Moments.emplace(0, std::move(out));
        break;
    }
    case 1: {
        this->GenerateMomentMap(0, inPtr, dimensions);

        std::vector<float> out(nels, std::numeric_limits<float>::quiet_NaN());

        const double initSpectralValue = this->Astro->getInitialSpectralValue();
        const double spectralDelta = std::abs(this->Astro->getIncrements()[2]);

        std::mutex m;
        vtkSMPTools::For(0, dimensions[2],
                         [this, &m, inPtr, &out, dimensions, nels, initSpectralValue,
                          spectralDelta](vtkIdType start, vtkIdType end) {
                             std::vector<float> tmp(nels, std::numeric_limits<float>::quiet_NaN());
                             const auto &moment0 = this->Moments.at(0);

                             for (int k = start; k < end; ++k) {
                                 const double spectralValue = initSpectralValue + spectralDelta * k;

                                 for (int j = 0; j < dimensions[1]; ++j) {
                                     for (int i = 0; i < dimensions[0]; ++i) {
                                         const vtkIdType outIdx = 1ll * i * this->PointerInc[0]
                                                 + j * this->PointerInc[1];
                                         const vtkIdType inIdx = outIdx + k * this->PointerInc[2];
                                         const float pixelValue = inPtr[inIdx];

                                         if (std::isfinite(pixelValue)
                                             && std::isfinite(moment0[outIdx])
                                             && moment0[outIdx] != 0.f) {
                                             if (std::isnan(tmp[outIdx])) {
                                                 tmp[outIdx] = 0.f;
                                             }
                                             tmp[outIdx] += spectralValue * pixelValue
                                                     * spectralDelta / moment0[outIdx];
                                         }
                                     }
                                 }
                             }

                             for (vtkIdType i = 0; i < nels; ++i) {
                                 if (std::isfinite(tmp[i])) {
                                     const std::lock_guard<std::mutex> lk(m);
                                     if (std::isnan(out[i])) {
                                         out[i] = 0.f;
                                     }
                                     out[i] += tmp[i];
                                 }
                             }
                         });

        this->Moments.emplace(1, std::move(out));
        break;
    }
    case 2: {
        this->GenerateMomentMap(1, inPtr, dimensions);

        std::vector<float> out(nels, std::numeric_limits<float>::quiet_NaN());

        const double initSpectralValue = this->Astro->getInitialSpectralValue();
        const double spectralDelta = std::abs(this->Astro->getIncrements()[2]);

        std::mutex m;
        vtkSMPTools::For(
                0, dimensions[2],
                [this, &m, inPtr, &out, dimensions, nels, initSpectralValue,
                 spectralDelta](vtkIdType start, vtkIdType end) {
                    std::vector<float> tmp(nels, std::numeric_limits<float>::quiet_NaN());
                    const auto &moment0 = this->Moments.at(0);
                    const auto &moment1 = this->Moments.at(1);

                    for (int k = start; k < end; ++k) {
                        const double spectralValue = initSpectralValue + spectralDelta * k;

                        for (int j = 0; j < dimensions[1]; ++j) {
                            for (int i = 0; i < dimensions[0]; ++i) {
                                const vtkIdType outIdx =
                                        1ll * i * this->PointerInc[0] + j * this->PointerInc[1];
                                const vtkIdType inIdx = outIdx + k * this->PointerInc[2];
                                const float pixelValue = inPtr[inIdx];

                                if (std::isfinite(pixelValue) && std::isfinite(moment0[outIdx])
                                    && moment0[outIdx] != 0.f) {
                                    if (std::isnan(tmp[outIdx])) {
                                        tmp[outIdx] = 0.f;
                                    }
                                    tmp[outIdx] += pixelValue * (spectralValue - moment1[outIdx])
                                            * (spectralValue - moment1[outIdx]) * spectralDelta
                                            / moment0[outIdx];
                                }
                            }
                        }
                    }

                    for (vtkIdType i = 0; i < nels; ++i) {
                        if (std::isfinite(tmp[i])) {
                            const std::lock_guard<std::mutex> lk(m);
                            if (std::isnan(out[i])) {
                                out[i] = 0.f;
                            }
                            out[i] += tmp[i];
                        }
                    }
                });

        this->Moments.emplace(2, std::move(out));
        break;
    }
    case 6: {
        std::vector<float> out(nels, std::numeric_limits<float>::quiet_NaN());

        std::mutex m;
        vtkSMPTools::For(0, dimensions[2],
                         [this, &m, inPtr, &out, dimensions, nels](vtkIdType start, vtkIdType end) {
                             std::vector<float> tmp(nels, std::numeric_limits<float>::quiet_NaN());
                             for (int k = start; k < end; ++k) {
                                 for (int j = 0; j < dimensions[1]; ++j) {
                                     for (int i = 0; i < dimensions[0]; ++i) {
                                         const vtkIdType outIdx = 1ll * i * this->PointerInc[0]
                                                 + j * this->PointerInc[1];
                                         const vtkIdType inIdx = outIdx + k * this->PointerInc[2];
                                         const float pixelValue = inPtr[inIdx];
                                         /// TODO: implement
                                     }
                                 }
                             }
                         });

        this->Moments.emplace(6, std::move(out));
        break;
    }
    case 8: {
        std::vector<float> out(nels, std::numeric_limits<float>::quiet_NaN());

        std::mutex m;
        vtkSMPTools::For(0, dimensions[2],
                         [this, &m, inPtr, &out, dimensions, nels](vtkIdType start, vtkIdType end) {
                             std::vector<float> tmp(nels, std::numeric_limits<float>::quiet_NaN());
                             for (int k = start; k < end; ++k) {
                                 for (int j = 0; j < dimensions[1]; ++j) {
                                     for (int i = 0; i < dimensions[0]; ++i) {
                                         const vtkIdType outIdx = 1ll * i * this->PointerInc[0]
                                                 + j * this->PointerInc[1];
                                         const vtkIdType inIdx = outIdx + k * this->PointerInc[2];
                                         const float pixelValue = inPtr[inIdx];

                                         tmp[outIdx] = std::fmaxf(tmp[outIdx], pixelValue);
                                     }
                                 }
                             }

                             for (vtkIdType i = 0; i < nels; ++i) {
                                 const std::lock_guard<std::mutex> lk(m);
                                 out[i] = std::fmaxf(out[i], tmp[i]);
                             }
                         });

        this->Moments.emplace(8, std::move(out));
        break;
    }
    case 10: {
        std::vector<float> out(nels, std::numeric_limits<float>::quiet_NaN());

        std::mutex m;
        vtkSMPTools::For(0, dimensions[2],
                         [this, &m, inPtr, &out, dimensions, nels](vtkIdType start, vtkIdType end) {
                             std::vector<float> tmp(nels, std::numeric_limits<float>::quiet_NaN());
                             for (int k = start; k < end; ++k) {
                                 for (int j = 0; j < dimensions[1]; ++j) {
                                     for (int i = 0; i < dimensions[0]; ++i) {
                                         const vtkIdType outIdx = 1ll * i * this->PointerInc[0]
                                                 + j * this->PointerInc[1];
                                         const vtkIdType inIdx = outIdx + k * this->PointerInc[2];
                                         const float pixelValue = inPtr[inIdx];

                                         tmp[outIdx] = std::fminf(tmp[outIdx], pixelValue);
                                     }
                                 }
                             }

                             for (vtkIdType i = 0; i < nels; ++i) {
                                 const std::lock_guard<std::mutex> lk(m);
                                 out[i] = std::fminf(out[i], tmp[i]);
                             }
                         });

        this->Moments.emplace(10, std::move(out));
        break;
    }
    }
}

//----------------------------------------------------------------------------
int vtkMomentMapFilter::RequestInformation(vtkInformation *vtkNotUsed(request),
                                           vtkInformationVector **inputVector,
                                           vtkInformationVector *outputVector)
{
    vtkInformation *outInfo = outputVector->GetInformationObject(0);
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);

    int wholeExtent[6];
    inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), wholeExtent);
    wholeExtent[4] = wholeExtent[5] = 0;
    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), wholeExtent, 6);

    return 1;
}

//----------------------------------------------------------------------------
void vtkMomentMapFilter::SimpleExecute(vtkImageData *input, vtkImageData *output)
{
    float *inPtr = static_cast<float *>(input->GetScalarPointer());
    const int *dimensions = input->GetDimensions();
    input->GetIncrements(this->PointerInc);
    const vtkIdType nels = 1ll * dimensions[0] * dimensions[1];

    float *outPtr = static_cast<float *>(output->GetScalarPointer());
    this->MaxId = output->GetNumberOfPoints();

    this->GenerateMomentMap(this->MomentOrder, inPtr, dimensions);
    std::copy_n(this->Moments.at(this->MomentOrder).cbegin(), nels, outPtr);
    this->CurrentMoment = outPtr;
}
