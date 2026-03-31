#ifndef ImageLayerSet_h
#define ImageLayerSet_h

#include <memory>
#include <string>
#include <vector>

struct ImageLayerLoadResult;

class ImageLayer;
class vtkImageData;
class vtkImageSlice;
class vtkLookupTable;

class ImageLayerSet
{
public:
    explicit ImageLayerSet(const std::string &masterFilepath);
    explicit ImageLayerSet(const ImageLayerLoadResult &masterResult);
    ~ImageLayerSet();

    int size() const;
    int masterIndex() const;

    vtkImageSlice *masterLayerActor() const;
    vtkImageSlice *layerActor(int index) const;

    float pixelValue(int index, int x, int y) const;
    vtkImageData *imageData(int index) const;
    vtkLookupTable *lookupTable(int index) const;

    double layerOpacity(int index) const;
    void setLayerOpacity(int index, double opacity);

    bool usingLogScale(int index) const;
    void setLogScale(int index, bool flag);

    std::string colorMapName(int index) const;
    void setColorMap(int index, const std::string &name);

    std::string filepath(int index) const;
    bool isVisible(int index) const;
    void setVisible(int index, bool visible);

    vtkImageSlice *addLayer(const std::string &filepath);
    vtkImageSlice *addLayer(const ImageLayerLoadResult &result);
    vtkImageSlice *replaceMasterLayer(const ImageLayerLoadResult &result);
    bool moveLayer(int sourceIndex, int destinationRow);

private:
    std::string masterFilepath;
    int masterIdx;
    std::vector<std::unique_ptr<ImageLayer>> layers;

    bool isValidIndex(int index) const;
    void registerLayerToMaster(ImageLayer &layer);
    void refreshLayerNumbers();
};

#endif
