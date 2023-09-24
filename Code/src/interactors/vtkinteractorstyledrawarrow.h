#ifndef VTKINTERACTORSTYLEDRAWARROW_H
#define VTKINTERACTORSTYLEDRAWARROW_H

#include <vtkInteractorStyleUser.h>
#include <vtkSmartPointer.h>

#include <functional>
#include <set>

class vtkCoordinate;

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
    void SetCallback(const std::function<void(int, int, int, int)> &cb);

protected:
    vtkInteractorStyleDrawArrow();
    ~vtkInteractorStyleDrawArrow() override;

private:
    vtkInteractorStyleDrawArrow(const vtkInteractorStyleDrawArrow &) = delete;
    void operator=(const vtkInteractorStyleDrawArrow &) = delete;

    std::function<void()> AbortCallback;
    std::function<void(int, int, int, int)> Callback;

    std::set<vtkRenderer *> Renderers;

    bool Drawing;
    double Start[3];
    double End[3];
    vtkNew<vtkCoordinate> Coordinate;
    vtkNew<vtkActor> Actor;
};

#endif // VTKINTERACTORSTYLEDRAWARROW_H
