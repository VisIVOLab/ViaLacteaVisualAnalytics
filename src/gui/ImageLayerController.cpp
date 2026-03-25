#include "ImageLayerController.h"

#include "LayerListModel.h"

#include <vtkImageStack.h>
#include <vtkLookupTable.h>
#include <vtkScalarBarActor.h>

ImageLayerController::ImageLayerController(LayerListModel &layers, vtkImageStack *stack,
                                           vtkScalarBarActor *colorbar)
    : layers(layers), stack(stack), colorbar(colorbar)
{
}

void ImageLayerController::setCurrentColorMap(int index, const std::string &name)
{
    if (!this->isValidIndex(index)) {
        return;
    }

    this->layers.setColorMap(index, name);
}

void ImageLayerController::setCurrentLogScale(int index, bool enabled)
{
    if (!this->isValidIndex(index)) {
        return;
    }

    this->layers.setLogScale(index, enabled);
}

void ImageLayerController::setCurrentOpacity(int index, double opacity)
{
    if (!this->isValidIndex(index)) {
        return;
    }

    this->layers.setLayerOpacity(index, opacity);
}

ImageLayerController::LayerViewState ImageLayerController::layerViewState(int index) const
{
    if (!this->isValidIndex(index)) {
        return { { }, false, 0, false };
    }

    return { this->layers.getColorMapName(index),
             this->layers.usingLogScale(index),
             static_cast<int>(this->layers.getLayerOpacity(index) * 100),
             true };
}

void ImageLayerController::activateLayer(int index)
{
    if (!this->isValidIndex(index)) {
        return;
    }

    this->stack->SetActiveLayer(index);
    this->colorbar->SetLookupTable(this->layers.getLookupTable(index));
}

bool ImageLayerController::isValidIndex(int index) const
{
    return index >= 0 && index < this->layers.rowCount();
}
