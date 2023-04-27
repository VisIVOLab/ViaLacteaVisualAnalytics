#ifndef VTKINTERACTORSTYLEPROFILE_H
#define VTKINTERACTORSTYLEPROFILE_H

#include "vtkinteractorstyleimagecustom.h"

#include <vtkSmartPointer.h>

#include <functional>
#include <set>

class vtkLineSource;

class vtkInteractorStyleProfile : public vtkInteractorStyleImageCustom
{
public:
    static vtkInteractorStyleProfile *New();
    vtkTypeMacro(vtkInteractorStyleProfile, vtkInteractorStyleImageCustom);
    void PrintSelf(std::ostream &os, vtkIndent indent) override;

    void OnMouseMove() override;
    void OnLeftButtonDown() override;

    void SetProfileCallback(const std::function<void(double, double)> &ProfileCb);

protected:
    vtkInteractorStyleProfile();
    ~vtkInteractorStyleProfile() override;

private:
    vtkInteractorStyleProfile(const vtkInteractorStyleProfile &) = delete;
    void operator=(const vtkInteractorStyleProfile &) = delete;

    std::function<void(double, double)> ProfileCb;

    std::set<vtkRenderer *> renderers;

    vtkSmartPointer<vtkLineSource> lineX;
    vtkSmartPointer<vtkActor> actorX;

    vtkSmartPointer<vtkLineSource> lineY;
    vtkSmartPointer<vtkActor> actorY;
};

#endif // VTKINTERACTORSTYLEIMAGECUSTOM_H
