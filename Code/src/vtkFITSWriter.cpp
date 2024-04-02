#include "vtkFITSWriter.h"

#include <fitsio.h>

#include <vtkErrorCode.h>
#include <vtkExecutive.h>
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>

vtkStandardNewMacro(vtkFITSWriter);

//----------------------------------------------------------------------------
vtkFITSWriter::vtkFITSWriter() {
  this->SetFileDimensionality(2);
  this->WriteToMemory = false;
}

//----------------------------------------------------------------------------
vtkFITSWriter::~vtkFITSWriter() = default;

//----------------------------------------------------------------------------
void vtkFITSWriter::Write() {
  this->SetErrorCode(vtkErrorCode::NoError);

  if (this->GetInput() == nullptr) {
    vtkErrorMacro(<< "No input provided. Cannot write to file.");
    this->SetErrorCode(vtkErrorCode::UserError);
    return;
  }

  if (!this->FileName) {
    vtkErrorMacro(<< "Specify a FileName");
    this->SetErrorCode(vtkErrorCode::NoFileNameError);
    return;
  }

  fitsfile *fptr;
  int status = 0;
  fits_create_file(&fptr, this->FileName, &status);
  const int *dimensions = this->GetInput()->GetDimensions();

  const int naxis = 2;
  long naxes[2] = {dimensions[0], dimensions[1]};
  fits_create_img(fptr, FLOAT_IMG, naxis, naxes, &status);

  long fpixel[2] = {1l, 1l};
  void *ptr = this->GetInput()->GetScalarPointer();
  fits_write_subset(fptr, TFLOAT, fpixel, naxes, ptr, &status);

  if (status) {
    vtkErrorMacro(<< "[CFITSIO] Error occurred");
    fits_report_error(stderr, status);
    this->SetErrorCode(vtkErrorCode::UnknownError);
  }

  fits_close_file(fptr, &status);
}

//----------------------------------------------------------------------------
void vtkFITSWriter::PrintSelf(ostream &os, vtkIndent indent) {
  this->Superclass::PrintSelf(os, indent);
}
