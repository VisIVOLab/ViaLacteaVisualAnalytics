#ifndef VTKDRAWLINEINTERACTORSTYLEUSER_H
#define VTKDRAWLINEINTERACTORSTYLEUSER_H

#include <QPointF>

#include <vtkCoordinate.h>
#include <vtkInteractorStyleUser.h>
#include <vtkLeaderActor2D.h>
#include <vtkSmartPointer.h>

#include <functional>
#include <set>

class vtkDrawLineInteractorStyleUser : public vtkInteractorStyleUser
{
public:
    static vtkDrawLineInteractorStyleUser *New();
    vtkTypeMacro(vtkDrawLineInteractorStyleUser, vtkInteractorStyleUser);
    void PrintSelf(std::ostream &os, vtkIndent indent) override;

    void OnMouseMove() override;
    void OnLeftButtonDown() override;
    void OnLeftButtonUp() override;
    void OnKeyPress() override;

    void setLineValsCallback(const std::function<void(float, float, float, float)>& callback);
    void setLineAbortCallback(const std::function<void ()> &newLineAbortCallback);

protected:
    vtkDrawLineInteractorStyleUser();
    ~vtkDrawLineInteractorStyleUser();
private:
    std::function<void(float, float, float, float)> LinePointCallback;
    std::function<void()> lineAbortCallback;

    bool isDrawingPVSliceLine;
    QPointF startPVSliceLine;
    QPointF endPVSliceLine;

    std::set<vtkRenderer *> renderers;

    double Start[3];
    double End[3];
    vtkNew<vtkCoordinate> Coordinate;
    vtkNew<vtkLeaderActor2D> Actor;
};

#endif // VTKDRAWLINEINTERACTORSTYLEUSER_H
