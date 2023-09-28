#include "vtkdrawlineinteractorstyleimage.h"

#include <vtkNamedColors.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include "vtkRenderWindow.h"
#include "vtkRendererCollection.h"
#include <vtkRenderWindowInteractor.h>

vtkStandardNewMacro(vtkDrawLineInteractorStyleImage);

vtkDrawLineInteractorStyleImage::vtkDrawLineInteractorStyleImage()
{
    isDrawingPVSliceLine = false;
    startPVSliceLine = endPVSliceLine = QPointF(0, 0);
}

vtkDrawLineInteractorStyleImage::~vtkDrawLineInteractorStyleImage()
{
}

void vtkDrawLineInteractorStyleImage::setCoordLocCallback(const std::function<void (const std::string &)> &newCoordLocCallback)
{
    coordLocCallback = newCoordLocCallback;
}

void vtkDrawLineInteractorStyleImage::setLineDrawnCallback(const std::function<void ()> &newLineDrawnCallback)
{
    lineDrawnCallback = newLineDrawnCallback;
}

void vtkDrawLineInteractorStyleImage::PrintSelf(ostream &os, vtkIndent indent)
{
    Superclass::PrintSelf(os, indent);
}

void vtkDrawLineInteractorStyleImage::OnMouseMove()
{
    Superclass::OnMouseMove();

    std::stringstream ss;

    if (isDrawingPVSliceLine) {
        endPVSliceLine = QPointF(this->GetInteractor()->GetEventPosition()[0], this->GetInteractor()->GetEventPosition()[1]);
        // Send information back to pqWindowCube to send to the server to calculate the actual pvslice.
        LinePointCallback(startPVSliceLine, endPVSliceLine);
    }
    this->coordLocCallback(ss.str());
}

void vtkDrawLineInteractorStyleImage::OnLeftButtonDown()
{
    Superclass::OnLeftButtonDown();

    startPVSliceLine = QPointF(this->GetInteractor()->GetEventPosition()[0], this->GetInteractor()->GetEventPosition()[1]);
    endPVSliceLine = startPVSliceLine;
    isDrawingPVSliceLine = true;
}

void vtkDrawLineInteractorStyleImage::OnLeftButtonUp()
{
    Superclass::OnLeftButtonUp();

    isDrawingPVSliceLine = false;
//    this->lineDrawnCallback();
}

void vtkDrawLineInteractorStyleImage::setLineValsCallback(const std::function<void (QPointF, QPointF)> &callback)
{
    LinePointCallback = callback;
}
