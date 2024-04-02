#ifndef VTKFITSWRITER_H
#define VTKFITSWRITER_H

#include <vtkImageWriter.h>

class vtkFITSWriter : public vtkImageWriter {
public:
  static vtkFITSWriter *New();
  vtkTypeMacro(vtkFITSWriter, vtkImageWriter);
  void PrintSelf(ostream &os, vtkIndent indent) override;

  void Write() override;

protected:
  vtkFITSWriter();
  ~vtkFITSWriter() override;

private:
  vtkFITSWriter(const vtkFITSWriter &) = delete;
  void operator=(const vtkFITSWriter &) = delete;
};

#endif // VTKFITSWRITER_H
