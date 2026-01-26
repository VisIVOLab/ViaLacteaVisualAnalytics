#include "vtkFITSReader.h"

#include <fitsio.h>

#include <vtkDataArray.h>
#include <vtkErrorCode.h>
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkSMPTools.h>

#include <cmath>
#include <limits>
#include <mutex>

vtkStandardNewMacro(vtkFITSReader);

//----------------------------------------------------------------------------
vtkFITSReader::vtkFITSReader()
{
    this->ScalarPointer = nullptr;
    this->MaxId = 0ll;
    this->PointerIncrement[0] = this->PointerIncrement[1] = this->PointerIncrement[2] = 0ll;
    this->Dimensions[0] = this->Dimensions[1] = this->Dimensions[2] = 0;

    this->Min = 0.f;
    this->Max = 0.f;
    this->Mean = 0.;
    this->RMS = 0.;

    this->vtkImageReader2::SetDataScalarTypeToFloat();
    this->vtkImageReader2::SetNumberOfScalarComponents(1);
}

//----------------------------------------------------------------------------
vtkFITSReader::~vtkFITSReader() = default;

//----------------------------------------------------------------------------
void vtkFITSReader::PrintSelf(ostream &os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);
    os << indent << "Stats:\n";
    os << indent << indent << "Min: " << this->Min << "\n";
    os << indent << indent << "Max: " << this->Max << "\n";
    os << indent << indent << "Mean: " << this->Mean << "\n";
    os << indent << indent << "RMS: " << this->RMS << "\n";
    os << indent << "Internals:\n";
    os << indent << indent << "ScalarPointer: " << this->ScalarPointer << "\n";
    os << indent << indent << "MaxId: " << this->MaxId << "\n";
    os << indent << indent << "PointerIncrement:\n";
    os << indent << indent << indent << "X: " << this->PointerIncrement[0] << "\n";
    os << indent << indent << indent << "Y: " << this->PointerIncrement[1] << "\n";
    os << indent << indent << indent << "Z: " << this->PointerIncrement[2] << "\n";
    os << indent << indent << "Dimensions:\n";
    os << indent << indent << indent << "X: " << this->Dimensions[0] << "\n";
    os << indent << indent << indent << "Y: " << this->Dimensions[1] << "\n";
    os << indent << indent << indent << "Z: " << this->Dimensions[2] << "\n";
}

//----------------------------------------------------------------------------
int vtkFITSReader::CanReadFile(const char *fname)
{
    fitsfile *fptr;
    int status = 0;

    if (fits_open_image(&fptr, fname, READONLY, &status)) {
        return 0;
    }

    fits_close_file(fptr, &status);
    return 1;
}

//----------------------------------------------------------------------------
const char *vtkFITSReader::GetFileExtensions()
{
    return ".fits";
}

//----------------------------------------------------------------------------
const char *vtkFITSReader::GetDescriptiveName()
{
    return "Flexible Image Transport System";
}

//----------------------------------------------------------------------------
float vtkFITSReader::GetValue(int x, int y, int z) const
{
    if (!this->ScalarPointer || x < 0 || x >= this->Dimensions[0] || y < 0
        || y >= this->Dimensions[1] || z < 0 || z >= this->Dimensions[2]) {
        return std::numeric_limits<float>::quiet_NaN();
    }

    const vtkIdType idx = 1ll * x * this->PointerIncrement[0] + y * this->PointerIncrement[1]
            + z * this->PointerIncrement[2];
    return this->ScalarPointer[idx];
}

//----------------------------------------------------------------------------
void vtkFITSReader::ExecuteInformation()
{
    this->ComputeInternalFileName(0);

    fitsfile *fptr;
    int status = 0;
    if (fits_open_image(&fptr, this->InternalFileName, READONLY, &status)) {
        vtkErrorMacro(<< "[CFITSIO] Error fits_open_image");
        fits_report_error(stderr, status);
        this->SetErrorCode(vtkErrorCode::CannotOpenFileError);
        return;
    }

    int naxis = 0;
    if (fits_get_img_dim(fptr, &naxis, &status)) {
        vtkErrorMacro(<< "[CFITSIO] Error fits_get_img_dim");
        fits_report_error(stderr, status);
        fits_close_file(fptr, &status);
        this->SetErrorCode(vtkErrorCode::UnknownError);
        return;
    }

    constexpr int maxaxis = 4;
    if (naxis < 2 || naxis > maxaxis) {
        fits_close_file(fptr, &status);
        vtkErrorMacro(<< "Unsupported NAXIS value.");
        this->SetErrorCode(vtkErrorCode::UserError);
        return;
    }

    long naxes[maxaxis] = { 1l, 1l, 1l, 1l };
    if (fits_get_img_size(fptr, naxis, naxes, &status)) {
        vtkErrorMacro(<< "[CFITSIO] Error fits_get_img_size");
        fits_report_error(stderr, status);
        fits_close_file(fptr, &status);
        this->SetErrorCode(vtkErrorCode::UnknownError);
        return;
    }

    fits_close_file(fptr, &status);

    this->DataExtent[0] = this->DataExtent[2] = this->DataExtent[4] = 0;
    this->DataExtent[1] = naxes[0] - 1;
    this->DataExtent[3] = naxes[1] - 1;
    this->DataExtent[5] = naxes[2] - 1;

    this->SetFileDimensionality(naxis);
}

//----------------------------------------------------------------------------
void vtkFITSReader::ExecuteDataWithInformation(vtkDataObject *output, vtkInformation *outInfo)
{
    vtkImageData *data = this->AllocateOutputData(output, outInfo);
    data->GetPointData()->GetScalars()->SetName("FITSImage");
    this->MaxId = data->GetPointData()->GetScalars()->GetMaxId();
    data->GetDimensions(this->Dimensions);

    this->ComputeDataIncrements();
    data->GetIncrements(this->PointerIncrement);

    fitsfile *fptr;
    int status = 0;
    if (fits_open_image(&fptr, this->InternalFileName, READONLY, &status)) {
        vtkErrorMacro(<< "[CFITSIO] Error fits_open_image");
        fits_report_error(stderr, status);
        this->SetErrorCode(vtkErrorCode::CannotOpenFileError);
        return;
    }

    long fpixel[4] = { 1l, 1l, 1l, 1l };
    long lpixel[4] = { this->Dimensions[0], this->Dimensions[1], this->Dimensions[2], 1l };
    long inc[4] = { 1l, 1l, 1l, 1l };

    this->ScalarPointer = static_cast<float *>(data->GetScalarPointer());
    if (fits_read_subset(fptr, TFLOAT, fpixel, lpixel, inc, nullptr, this->ScalarPointer, nullptr,
                         &status)) {
        vtkErrorMacro(<< "[CFITSIO] Error fits_read_subset");
        fits_report_error(stderr, status);
        fits_close_file(fptr, &status);
        this->SetErrorCode(vtkErrorCode::UnknownError);
        return;
    }

    fits_close_file(fptr, &status);

    vtkIdType ngood = 0ll;
    double sum = 0.;
    double sum2 = 0.;
    this->Min = std::numeric_limits<float>::max();
    this->Max = std::numeric_limits<float>::min();
    std::mutex m;
    vtkSMPTools::For(0ll, this->MaxId,
                     [this, &m, &ngood, &sum, &sum2](vtkIdType start, vtkIdType end) {
                         vtkIdType tmpGood = 0ll;
                         double tmpSum = 0.;
                         double tmpSum2 = 0.;
                         float tmpMin = std::numeric_limits<float>::max();
                         float tmpMax = std::numeric_limits<float>::min();

                         for (vtkIdType i = start; i < end; ++i) {
                             if (std::isfinite(this->ScalarPointer[i])) {
                                 ++tmpGood;
                                 tmpSum += this->ScalarPointer[i];
                                 tmpSum2 += this->ScalarPointer[i] * this->ScalarPointer[i];
                                 tmpMin = std::fminf(tmpMin, this->ScalarPointer[i]);
                                 tmpMax = std::fmaxf(tmpMax, this->ScalarPointer[i]);
                             }
                         }

                         const std::lock_guard<std::mutex> lk(m);
                         ngood += tmpGood;
                         sum += tmpSum;
                         sum2 += tmpSum2;
                         this->Min = std::fminf(this->Min, tmpMin);
                         this->Max = std::fmaxf(this->Max, tmpMax);
                     });

    if (ngood > 0ll) {
        this->Mean = sum / ngood;
        this->RMS = std::sqrt((sum2 / ngood) - (this->Mean * this->Mean));
    }
}
