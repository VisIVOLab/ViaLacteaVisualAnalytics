#ifndef VTKDRAWLINEINTERACTORSTYLEIMAGE_H
#define VTKDRAWLINEINTERACTORSTYLEIMAGE_H

#include <QPointF>

#include "vtkInteractorStyleImage.h"

#include <functional>

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

protected:
    vtkDrawLineInteractorStyleImage();
private:
    std::function<void(QPointF startPoint, QPointF endPoint)> LinePointCallback;

    bool isDrawingPVSliceLine;
    QPointF startPVSliceLine;
    QPointF endPVSliceLine;
};

#endif // VTKDRAWLINEINTERACTORSTYLEIMAGE_H
