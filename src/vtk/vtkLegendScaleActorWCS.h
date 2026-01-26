#ifndef vtkLegendScaleActorWCS_h
#define vtkLegendScaleActorWCS_h

#include <vtkLegendScaleActor.h>

#include <memory>

class AstroUtils;

class vtkLegendScaleActorWCS : public vtkLegendScaleActor
{
public:
    static vtkLegendScaleActorWCS *New();
    vtkTypeMacro(vtkLegendScaleActorWCS, vtkLegendScaleActor);
    void PrintSelf(ostream &os, vtkIndent indent) override;

    vtkGetMacro(WCS, int);
    void SetWCS(int wcs);

    void Init(const std::string &filepath);

    void BuildRepresentation(vtkViewport *viewport) override;

protected:
    vtkLegendScaleActorWCS();
    ~vtkLegendScaleActorWCS() override;

    int WCS;

private:
    vtkLegendScaleActorWCS(const vtkLegendScaleActorWCS &) = delete;
    void operator=(const vtkLegendScaleActorWCS &) = delete;

    std::unique_ptr<AstroUtils> Astro;
};

#endif
