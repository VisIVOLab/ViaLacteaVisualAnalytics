#ifndef LayerListModel_h
#define LayerListModel_h

#include <QAbstractListModel>

#include <memory>
#include <string>

struct ImageLayerLoadResult;

class ImageLayerSet;
class vtkImageData;
class vtkImageSlice;
class vtkLookupTable;

/**
 * Layer List Model used in vtkWindowImage
 */
class LayerListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    /**
   * Layer List Model used in vtkWindowImage
   * @param filepath FITS absolute path for the Master Layer
   */
    explicit LayerListModel(const std::string &filepath, QObject *parent = nullptr);
    ~LayerListModel() override;

    /**
   * @return Master layer's vtkImageSlice
   */
    vtkImageSlice *getMasterLayerActor() const;

    /**
   * @return index of the Master Layer
   */
    int getMasterIndex() const;

    /**
   * Add a new Layer to the model
   * @param filepath FITS absolute filepath
   * @return the vtkImageSlice of the added layer
   */
    vtkImageSlice *addLayer(const std::string &filepath);
    vtkImageSlice *addLayer(const ImageLayerLoadResult &result);

    /**
   * @param index inside the model
   * @param x pixel coordinate
   * @param y pixel coordinate
   * @return pixel valye
   */
    float getPixelValue(int index, int x, int y) const;

    /**
   * @param index
   * @return vtkImageData associated to the layer
   */
    vtkImageData *getImageData(int index) const;

    /**
   * @param index
   * @return vtkLookupTable associated to the layer
   */
    vtkLookupTable *getLookupTable(int index) const;

    /**
   * @param index
   * @return layer's opacity [0, 1]
   */
    double getLayerOpacity(int index) const;

    /**
   * Set the Layer Opacity
   * @param index
   * @param opacity value in [0, 1]
   */
    void setLayerOpacity(int index, double opacity);

    /**
   * @param index
   * @return true if vtkLookupTable is using Log scaling
   */
    bool usingLogScale(int index) const;

    /**
   * Set vtkLookupTable log scale flag
   * @param index
   * @param flag
   */
    void setLogScale(int index, bool flag);

    /**
   * @param index
   * @return Color map used by vtkLookupTable
   */
    std::string getColorMapName(int index) const;

    /**
   * Set layer's color map
   * @param index
   * @param name Color map name
   */
    void setColorMap(int index, const std::string &name);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Drag and Drop
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
                         const QModelIndex &parent);
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
                      const QModelIndex &parent) override;

private:
    std::unique_ptr<ImageLayerSet> layerSet;
    QString layerMimeType;
};

#endif
