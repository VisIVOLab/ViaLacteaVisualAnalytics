#ifndef vtkFITSWriter_h
#define vtkFITSWriter_h

#include <vtkImageWriter.h>

class vtkFitsWriter : public vtkImageWriter
{
public:
    static vtkFitsWriter *New();
    vtkTypeMacro(vtkFitsWriter, vtkImageWriter);
    void PrintSelf(std::ostream &os, vtkIndent indent) override;

    void Write() override;

protected:
    vtkFitsWriter() = default;
    ~vtkFitsWriter() override = default;

private:
    vtkFitsWriter(const vtkFitsWriter &) = delete;
    vtkFitsWriter &operator=(const vtkFitsWriter &) = delete;
};

#endif
