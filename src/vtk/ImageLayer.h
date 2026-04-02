#ifndef ImageLayer_h
#define ImageLayer_h

#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <string>

struct ImageLayerLoadResult;

class vtkFITSReader;
class vtkImageData;
class vtkImageSlice;
class vtkLookupTable;
class vtkTrivialProducer;
class vtkImageMapToColors;

/**
 * Layer represented in vtkWindowImage
 */
class ImageLayer
{
public:
    /**
   * Layer represented in vtkWindowImage
   * @param filepath FITS absolute filepath
   */
    ImageLayer(const std::string &filepath);
    ImageLayer(const ImageLayerLoadResult &result);
    ~ImageLayer();

    /**
   * @return FITS absolute filepath associated to this layer
   */
    std::string getFilepath() const;

    /**
   * @param x pixel coordinate
   * @param y pixel coordinate
   * @return pixel value
   */
    float getPixelValue(int x, int y) const;

    /**
   * @return vtkImageSlice associated to this layer
   */
    vtkImageSlice *getActor() const;

    /**
   * @return vtkImageData associated to this layer
   */
    vtkImageData *getImageData() const;

    /**
   * @return vtkLookupTable associated to this layer
   */
    vtkLookupTable *getLookupTable() const;

    /**
   * @return true if the layer is visible
   */
    bool isVisible() const;

    /**
   * Set layer's visibility
   * @param visible flag
   */
    void setVisibility(bool visible);

    /**
   * @return layer's opacity [0, 1]
   */
    double getOpacity() const;

    /**
   * Set layer's ppacity
   * @param opacity value in [0, 1]
   */
    void setOpacity(double opacity);

    /**
   * @return true if vtkLookupTable is using Log scaling
   */
    bool usingLogScale() const;

    /**
   * Set vtkLookupTable log scale flag
   * @param flag
   */
    void setLogScale(bool flag);

    /**
   * @return Color map used by vtkLookupTable
   */
    std::string getColorMapName() const;

    /**
   * Set this layer's color map
   * @param name Color map name
   */
    void setColorMap(const std::string &name);

    /**
   * Update this layer's origin
   * @param origin
   */
    void setOrigin(const double *origin);

    /**
   * Update this layer's spacing
   * @param spacing
   */
    void setSpacing(const double *spacing);

    /**
   * Set this layer's rotation angle
   * @param angle Degree angle
   */
    void setRotation(double angle);

private:
    void initializeDisplayPipeline();
    void updateDisplaySource();
    void rebuildPreviewImageIfNeeded(const char *context);

    bool readerBacked;
    bool previewModeActive{ false };
    double scalarRange[2];
    std::string filepath;
    vtkSmartPointer<vtkImageData> imageData;
    vtkSmartPointer<vtkImageData> displayImageData;
    vtkNew<vtkFITSReader> reader;
    vtkNew<vtkTrivialProducer> source;
    vtkNew<vtkLookupTable> lut;
    vtkNew<vtkImageSlice> actor;
    vtkSmartPointer<vtkImageMapToColors> colors;
};

#endif
