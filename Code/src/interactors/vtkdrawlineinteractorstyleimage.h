#ifndef VTKDRAWLINEINTERACTORSTYLEIMAGE_H
#define VTKDRAWLINEINTERACTORSTYLEIMAGE_H

#include <QPointF>

#include "vtkActor.h"
#include "vtkCoordinate.h"
#include "vtkInteractorStyleImage.h"
#include "vtkLineSource.h"

#include <functional>
#include <set>

class vtkDrawLineInteractorStyleImage : public vtkInteractorStyleImage
{
public:
    static vtkDrawLineInteractorStyleImage *New();
    vtkTypeMacro(vtkDrawLineInteractorStyleImage, vtkInteractorStyleImage);
    void PrintSelf(std::ostream &os, vtkIndent indent) override;

    void OnMouseMove() override;
    void OnLeftButtonDown() override;
    void OnLeftButtonUp() override;

    void setLineValsCallback(const std::function<void(QPointF startPoint, QPointF endPoint)>& callback);
    void setLineDrawnCallback(const std::function<void ()> &newLineDrawnCallback);

    void setCoordLocCallback(const std::function<void (const std::string &)> &newCoordLocCallback);

protected:
    vtkDrawLineInteractorStyleImage();
    ~vtkDrawLineInteractorStyleImage();
private:
    std::function<void(QPointF startPoint, QPointF endPoint)> LinePointCallback;
    std::function<void()> lineDrawnCallback;
    std::function<void(const std::string &)> coordLocCallback;

    bool isDrawingPVSliceLine;
    QPointF startPVSliceLine;
    QPointF endPVSliceLine;

    std::set<vtkRenderer *> renderers;

    vtkNew<vtkCoordinate> coordinate;
    vtkNew<vtkLineSource> line;
    vtkNew<vtkActor> lineActor;
};

#endif // VTKDRAWLINEINTERACTORSTYLEIMAGE_H
