#ifndef ImageLayerController_h
#define ImageLayerController_h

#include <string>

class LayerListModel;
class vtkImageStack;
class vtkScalarBarActor;

class ImageLayerController
{
public:
    struct LayerViewState
    {
        std::string colorMapName;
        bool usingLogScale;
        int opacityPercent;
        bool valid;
    };

    ImageLayerController(LayerListModel &layers, vtkImageStack *stack, vtkScalarBarActor *colorbar);

    void setCurrentColorMap(int index, const std::string &name);
    void setCurrentLogScale(int index, bool enabled);
    void setCurrentOpacity(int index, double opacity);

    LayerViewState layerViewState(int index) const;
    void activateLayer(int index);

private:
    LayerListModel &layers;
    vtkImageStack *stack;
    vtkScalarBarActor *colorbar;

    bool isValidIndex(int index) const;
};

#endif
