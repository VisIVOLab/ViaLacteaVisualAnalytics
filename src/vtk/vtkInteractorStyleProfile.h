#ifndef vtkInteractorStyleProfile_h
#define vtkInteractorStyleProfile_h

#include <vtkInteractorStyleUser.h>

#include <vtkNew.h>

#include <functional>

class vtkActor;
class vtkCoordinate;
class vtkLineSource;

class vtkInteractorStyleProfile : public vtkInteractorStyleUser
{
public:
    static vtkInteractorStyleProfile *New();
    vtkTypeMacro(vtkInteractorStyleProfile, vtkInteractorStyleUser);
    void PrintSelf(ostream &os, vtkIndent indent) override;

    vtkSetMacro(LiveMode, bool);
    vtkGetMacro(LiveMode, bool);
    vtkBooleanMacro(LiveMode, bool);

    vtkSetVector2Macro(Lengths, int);
    vtkGetVector2Macro(Lengths, int);

    void SetCallback(const std::function<void(double, double, bool)> &cb);

    void OnMouseMove() override;
    void OnLeftButtonDown() override;

protected:
    vtkInteractorStyleProfile();
    ~vtkInteractorStyleProfile() override;

    bool Draw;
    bool LiveMode;
    int Lengths[2];
    double WorldCoords[2];

    std::function<void(double, double, bool)> Callback;

    vtkNew<vtkCoordinate> Coordinate;
    vtkNew<vtkLineSource> LineX;
    vtkNew<vtkActor> ActorX;
    vtkNew<vtkLineSource> LineY;
    vtkNew<vtkActor> ActorY;

private:
    vtkInteractorStyleProfile(const vtkInteractorStyleProfile &) = delete;
    void operator=(const vtkInteractorStyleProfile &) = delete;
};

#endif
