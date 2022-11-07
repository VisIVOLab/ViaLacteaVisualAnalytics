#ifndef VTKFITSREADER2_H
#define VTKFITSREADER2_H

#include <vtkImageReader2.h>
#include <vtkSetGet.h>

class vtkFitsReader2 : public vtkImageReader2
{
public:
    static vtkFitsReader2 *New();
    vtkTypeMacro(vtkFitsReader2, vtkImageReader2);
    void PrintSelf(std::ostream &os, vtkIndent indent) override;

    vtkGetVector6Macro(Bounds, double);

    vtkGetMacro(ScaleFactor, int);
    vtkSetMacro(ScaleFactor, int);

    vtkGetMacro(SliceMode, bool);
    vtkSetMacro(SliceMode, bool);
    vtkBooleanMacro(SliceMode, bool);

    vtkSetMacro(Slice, int);
    vtkGetMacro(Slice, int);

    vtkGetMacro(NumberOfSlices, int);

    vtkGetMacro(Mean, double);
    vtkGetMacro(RMS, double);
    vtkGetVector2Macro(ValueRange, float);

    float *GetPixelValue(int x, int y, int z);

protected:
    vtkFitsReader2();
    ~vtkFitsReader2() override;

    int RequestInformation(vtkInformation *request, vtkInformationVector **inputVector,
                           vtkInformationVector *outputVector) override;
    int RequestData(vtkInformation *request, vtkInformationVector **inputVector,
                    vtkInformationVector *outputVector) override;

private:
    vtkFitsReader2(const vtkFitsReader2 &) = delete;
    vtkFitsReader2 &operator=(const vtkFitsReader2 &) = delete;

    vtkImageData *outData;

    double Bounds[6];

    int ScaleFactor;

    bool SliceMode;
    int Slice;
    int NumberOfSlices;

    double Mean;
    double RMS;
    float ValueRange[2];
};

#endif // VTKFITSREADER2_H
