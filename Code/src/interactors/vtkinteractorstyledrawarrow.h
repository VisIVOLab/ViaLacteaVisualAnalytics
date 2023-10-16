#ifndef VTKINTERACTORSTYLEDRAWARROW_H
#define VTKINTERACTORSTYLEDRAWARROW_H

#include <vtkInteractorStyleUser.h>
#include <vtkSmartPointer.h>

#include <functional>
#include <set>

class vtkCoordinate;
class vtkLeaderActor2D;

class vtkInteractorStyleDrawArrow : public vtkInteractorStyleUser
{
public:
    static vtkInteractorStyleDrawArrow *New();
    vtkTypeMacro(vtkInteractorStyleDrawArrow, vtkInteractorStyleUser);
    void PrintSelf(std::ostream &os, vtkIndent indent) override;

    void OnLeftButtonDown() override;
    void OnMouseMove() override;
    void OnLeftButtonUp() override;
    void OnKeyPress() override;

    void SetAbortCallback(const std::function<void(void)> &cb);
    void SetCallback(const std::function<void(float, float, float, float)> &cb);

protected:
    vtkInteractorStyleDrawArrow();
    ~vtkInteractorStyleDrawArrow() override;

private:
    vtkInteractorStyleDrawArrow(const vtkInteractorStyleDrawArrow &) = delete;
    void operator=(const vtkInteractorStyleDrawArrow &) = delete;

    std::function<void()> AbortCallback;
    std::function<void(float, float, float, float)> Callback;

    std::set<vtkRenderer *> Renderers;

    bool Drawing;
    double Start[3];
    double End[3];
    vtkNew<vtkCoordinate> Coordinate;
    vtkNew<vtkLeaderActor2D> Actor;
};

#endif // VTKINTERACTORSTYLEDRAWARROW_H
