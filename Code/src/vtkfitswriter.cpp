#include "vtkfitswriter.h"

#include <vtkErrorCode.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>

#include <fitsio.h>

vtkStandardNewMacro(vtkFitsWriter);

void vtkFitsWriter::Write()
{
    this->SetErrorCode(vtkErrorCode::NoError);

    // Error checking
    if (this->GetInput() == nullptr) {
        vtkErrorMacro(<< "Write:Please specify an input!");
        return;
    }
    if (!this->WriteToMemory && !this->FileName && !this->FilePattern) {
        vtkErrorMacro(<< "Write:Please specify either a FileName or a file prefix "
                         "and a pattern");
        this->SetErrorCode(vtkErrorCode::NoFileNameError);
        return;
    }

    auto scalars = vtkFloatArray::FastDownCast(this->GetInput()->GetPointData()->GetScalars());
    if (!scalars) {
        vtkErrorMacro(<< "Write:Got null scalars array from input");
        return;
    }

    fitsfile *fptr;
    int status = 0;
    char errtext[FLEN_ERRMSG] = { 0 };
    if (fits_open_file(&fptr, this->FileName, READWRITE, &status)) {
        fits_get_errstatus(status, errtext);
        throw std::runtime_error(errtext);
    }

    long naxes[2];
    if (fits_get_img_size(fptr, 2, naxes, &status)) {
        fits_get_errstatus(status, errtext);
        throw std::runtime_error(errtext);
    }

    float *ptr = static_cast<float *>(scalars->GetVoidPointer(0));
    if (fits_write_img(fptr, TFLOAT, 1, naxes[0] * naxes[1], ptr, &status)) {
        fits_get_errstatus(status, errtext);
        throw std::runtime_error(errtext);
    }

    fits_close_file(fptr, &status);
}

void vtkFitsWriter::PrintSelf(std::ostream &os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);
}
