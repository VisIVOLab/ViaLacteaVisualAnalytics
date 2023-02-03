#include "vtkfitsreader2.h"

#include <fitsio.h>

#include <vtkAbstractArray.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <algorithm>
#include <cmath>

vtkStandardNewMacro(vtkFitsReader2);

vtkFitsReader2::vtkFitsReader2()
{
    this->FileName = NULL;
    this->ScaleFactor = 1;
    this->SliceMode = false;
    this->Slice = 0;
}

vtkFitsReader2::~vtkFitsReader2()
{
    this->SetFileName(0);
}

void vtkFitsReader2::PrintSelf(std::ostream &os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);
    os << indent << "Using CFITSIO " << CFITSIO_MAJOR << "." << CFITSIO_MINOR << "."
       << CFITSIO_MICRO;
    os << indent << "ScaleFactor: " << this->ScaleFactor;
    os << indent << "SliceMode: " << this->SliceMode;
    os << indent << indent << "Slice: " << this->Slice;
    os << indent << indent << "NumberOfSlices: " << this->NumberOfSlices;
}

float *vtkFitsReader2::GetPixelValue(int x, int y, int z)
{
    if (outData) {
        return static_cast<float *>(outData->GetScalarPointer(x, y, z));
    }

    return nullptr;
}

int vtkFitsReader2::RequestInformation(vtkInformation *vtkNotUsed(request),
                                       vtkInformationVector **vtkNotUsed(inputVector),
                                       vtkInformationVector *outputVector)
{
    fitsfile *fptr;
    int ReadStatus = 0;
    if (fits_open_data(&fptr, this->FileName, READONLY, &ReadStatus)) {
        std::cerr << "vtkFitsReader2::RequestInformation [CFITSIO] Error fits_open_data";
        fits_report_error(stderr, ReadStatus);
        return 0;
    }

    int naxis = 0;
    if (fits_get_img_dim(fptr, &naxis, &ReadStatus)) {
        std::cerr << "vtkFitsReader2::RequestInformation [CFITSIO] Error fits_get_img_dim";
        fits_report_error(stderr, ReadStatus);
        return 0;
    }

    long *naxes = new long[naxis];
    if (fits_get_img_size(fptr, naxis, naxes, &ReadStatus)) {
        std::cerr << "vtkFitsReader2::RequestInformation [CFITSIO] Error fits_get_img_size";
        fits_report_error(stderr, ReadStatus);
        delete[] naxes;
        return 0;
    }

    if (fits_close_file(fptr, &ReadStatus)) {
        std::cerr << "vtkFitsReader2::RequestInformation [CFITSIO] Error fits_close_file";
        fits_report_error(stderr, ReadStatus);
        // We should have axes information, so we do not abort (i.e. no return failure here)
    }

    int dataExtent[6] = { 0, 0, 0, 0, 0, 0 };
    this->NumberOfSlices = naxes[2];
    dataExtent[1] = (naxes[0] - 1) / this->ScaleFactor;
    dataExtent[3] = (naxes[1] - 1) / this->ScaleFactor;
    if (this->SliceMode) {
        dataExtent[4] = this->Slice;
        dataExtent[5] = this->Slice;
    } else {
        dataExtent[5] = (naxes[2] - 1) / this->ScaleFactor;
    }

    vtkInformation *outInfo = outputVector->GetInformationObject(0);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), dataExtent, 6);

    delete[] naxes;
    return 1;
}

int vtkFitsReader2::RequestData(vtkInformation *vtkNotUsed(request),
                                vtkInformationVector **vtkNotUsed(inputVector),
                                vtkInformationVector *outputVector)
{
    int dataExtent[6];
    vtkInformation *outInfo = outputVector->GetInformationObject(0);
    outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), dataExtent);

    long dimX = dataExtent[1] - dataExtent[0] + 1;
    long dimY = dataExtent[3] - dataExtent[2] + 1;
    long dimZ = dataExtent[5] - dataExtent[4] + 1;

    outData = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
    outData->SetExtent(dataExtent);
    outData->SetSpacing(ScaleFactor, ScaleFactor, ScaleFactor);
    outData->SetOrigin(0.0, 0.0, 0.0);

    fitsfile *fptr;
    int ReadStatus = 0;
    if (fits_open_data(&fptr, this->FileName, READONLY, &ReadStatus)) {
        std::cerr << "vtkFitsReader2::RequestData [CFITSIO] Error fits_open_data";
        fits_report_error(stderr, ReadStatus);
        return 0;
    }

    // DO NOT assume datacubes have exactly 3 axes, they may have more
    int naxis = 0;
    if (fits_get_img_dim(fptr, &naxis, &ReadStatus)) {
        std::cerr << "vtkFitsReader2::RequestData [CFITSIO] Error fits_get_img_dim";
        fits_report_error(stderr, ReadStatus);
        return 0;
    }

    // First Pixel coords
    long *fP = new long[naxis];
    std::fill_n(fP, naxis, 1);
    fP[0] = dataExtent[0] + 1;
    fP[1] = dataExtent[2] + 1;
    fP[2] = dataExtent[4] + 1;

    // Last Pixel coords
    long *lP = new long[naxis];
    std::fill_n(lP, naxis, 1);
    lP[0] = ScaleFactor * dataExtent[1] + 1;
    lP[1] = ScaleFactor * dataExtent[3] + 1;
    lP[2] = ScaleFactor * dataExtent[5] + 1;

    // Increments
    long *inc = new long[naxis];
    std::fill_n(inc, naxis, 1);
    inc[0] = ScaleFactor;
    inc[1] = ScaleFactor;
    inc[2] = ScaleFactor;

    float nulval = 1e-30;
    int anynul = 0;
    long nels = dimX * dimY * dimZ;
    float *ptr = new float[nels];
    if (fits_read_subset(fptr, TFLOAT, fP, lP, inc, &nulval, ptr, &anynul, &ReadStatus)) {
        std::cerr << "vtkFitsReader2::RequestData [CFITSIO] Error fits_read_subset";
        fits_report_error(stderr, ReadStatus);
        delete[] ptr;
        return 0;
    }

    if (fits_img_stats_float(ptr, dimX * dimY, dimZ, 1, nulval, 0, &this->ValueRange[0],
                             &this->ValueRange[1], &this->Mean, &this->RMS, 0, 0, 0, 0,
                             &ReadStatus)) {
        std::cerr << "vtkFitsReader2::RequestData [CFITSIO] Error fits_img_stats_float";
        fits_report_error(stderr, ReadStatus);
        return 0;
    }

    if (fits_close_file(fptr, &ReadStatus)) {
        std::cerr << "vtkFitsReader2::RequestData [CFITSIO] Error fits_close_file";
        fits_report_error(stderr, ReadStatus);
        // We should have read the data, so we do not abort (i.e. no return failure here)
    }

    vtkNew<vtkFloatArray> scalars;
    scalars->SetName("FITS");
    scalars->SetNumberOfComponents(1);
    scalars->SetVoidArray(ptr, nels, 0, vtkAbstractArray::VTK_DATA_ARRAY_DELETE);
    outData->GetPointData()->SetScalars(scalars);
    outData->GetBounds(this->Bounds);

    return 1;
}
