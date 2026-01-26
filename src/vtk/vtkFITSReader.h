#ifndef vtkFITSReader_h
#define vtkFITSReader_h

#include <vtkImageReader2.h>

class vtkFITSReader : public vtkImageReader2
{
public:
    static vtkFITSReader *New();
    vtkTypeMacro(vtkFITSReader, vtkImageReader2);
    void PrintSelf(ostream &os, vtkIndent indent) override;

    int CanReadFile(const char *fname) override;
    const char *GetFileExtensions() override;
    const char *GetDescriptiveName() override;

    vtkGetMacro(Min, float);
    vtkGetMacro(Max, float);
    vtkGetMacro(Mean, double);
    vtkGetMacro(RMS, double);

    float GetValue(int x, int y, int z = 0) const;

protected:
    vtkFITSReader();
    ~vtkFITSReader() override;

    void ExecuteInformation() override;
    void ExecuteDataWithInformation(vtkDataObject *output, vtkInformation *outInfo) override;

    float Min;
    float Max;
    double Mean;
    double RMS;

private:
    vtkFITSReader(const vtkFITSReader &) = delete;
    void operator=(const vtkFITSReader &) = delete;

    float *ScalarPointer;
    vtkIdType MaxId;
    vtkIdType PointerIncrement[3];
    int Dimensions[3];
};

#endif
