#ifndef vtkMomentMapFilter_h
#define vtkMomentMapFilter_h

#include <vtkSimpleImageToImageFilter.h>

#include <memory>
#include <unordered_map>

class AstroUtils;

class vtkMomentMapFilter : public vtkSimpleImageToImageFilter
{
public:
    static vtkMomentMapFilter *New();
    vtkTypeMacro(vtkMomentMapFilter, vtkSimpleImageToImageFilter);
    void PrintSelf(ostream &os, vtkIndent indent) override;

    vtkGetMacro(MomentOrder, int);
    vtkSetMacro(MomentOrder, int);

    void Init(const std::string &filepath);
    float GetValue(int x, int y) const;

protected:
    vtkMomentMapFilter();
    ~vtkMomentMapFilter() override;

    int MomentOrder;

    int RequestInformation(vtkInformation *request, vtkInformationVector **inputVector,
                           vtkInformationVector *output) override;
    void SimpleExecute(vtkImageData *input, vtkImageData *output) override;

private:
    vtkMomentMapFilter(const vtkMomentMapFilter &) = delete;
    void operator=(const vtkMomentMapFilter &) = delete;

    void GenerateMomentMap(int order, const float *inPtr, const int *dimensions);

    std::unique_ptr<AstroUtils> Astro;
    std::unordered_map<int, std::vector<float>> Moments;
    const float *CurrentMoment;
    vtkIdType MaxId;
    vtkIdType PointerInc[3];
};

#endif
