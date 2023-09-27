#include "vtkdrawlineinteractorstyleimage.h"
#include "vtkRenderWindowInteractor.h"

vtkDrawLineInteractorStyleImage::vtkDrawLineInteractorStyleImage()
{
    isDrawingPVSliceLine = false;
    startPVSliceLine = endPVSliceLine = QPointF(0, 0);
}

void vtkDrawLineInteractorStyleImage::OnMouseMove()
{
    if (isDrawingPVSliceLine) {
        endPVSliceLine = QPointF(this->GetInteractor()->GetEventPosition()[0], this->GetInteractor()->GetEventPosition()[1]);
        LinePointCallback(startPVSliceLine, endPVSliceLine);
    }

    Superclass::OnMouseMove();
}

void vtkDrawLineInteractorStyleImage::OnLeftButtonDown()
{
    startPVSliceLine = QPointF(this->GetInteractor()->GetEventPosition()[0], this->GetInteractor()->GetEventPosition()[1]);
    endPVSliceLine = startPVSliceLine;
    isDrawingPVSliceLine = true;

    Superclass::OnLeftButtonDown();
}

void vtkDrawLineInteractorStyleImage::OnLeftButtonUp()
{
    isDrawingPVSliceLine = false;

    Superclass::OnLeftButtonUp();
}

void vtkDrawLineInteractorStyleImage::setLineValsCallback(const std::function<void (QPointF, QPointF)> &callback)
{
    LinePointCallback = callback;
}
