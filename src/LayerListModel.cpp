#include "LayerListModel.h"
#include "ImageLayerSet.h"

#include <QDataStream>
#include <QIODevice>
#include <QMimeData>

LayerListModel::LayerListModel(const std::string &filepath, QObject *parent)
    : QAbstractListModel(parent), layerSet(std::make_unique<ImageLayerSet>(filepath)),
      layerMimeType("application/x-image-layer")
{
}

vtkImageSlice *LayerListModel::getMasterLayerActor() const
{
    return this->layerSet->masterLayerActor();
}

int LayerListModel::getMasterIndex() const
{
    return this->layerSet->masterIndex();
}

LayerListModel::~LayerListModel() = default;

vtkImageSlice *LayerListModel::addLayer(const std::string &filepath)
{
    const int lastIdx = this->layerSet->size();
    auto actor = this->layerSet->addLayer(filepath);
    emit this->dataChanged(this->index(lastIdx), this->index(lastIdx));
    return actor;
}

float LayerListModel::getPixelValue(int index, int x, int y) const
{
    return this->layerSet->pixelValue(index, x, y);
}

vtkImageData *LayerListModel::getImageData(int index) const
{
    return this->layerSet->imageData(index);
}

vtkLookupTable *LayerListModel::getLookupTable(int index) const
{
    return this->layerSet->lookupTable(index);
}

double LayerListModel::getLayerOpacity(int index) const
{
    return this->layerSet->layerOpacity(index);
}

void LayerListModel::setLayerOpacity(int index, double opacity)
{
    this->layerSet->setLayerOpacity(index, opacity);
    emit this->dataChanged(this->index(index), this->index(index));
}

bool LayerListModel::usingLogScale(int index) const
{
    return this->layerSet->usingLogScale(index);
}

void LayerListModel::setLogScale(int index, bool flag)
{
    this->layerSet->setLogScale(index, flag);
    emit this->dataChanged(this->index(index), this->index(index));
}

std::string LayerListModel::getColorMapName(int index) const
{
    return this->layerSet->colorMapName(index);
}

void LayerListModel::setColorMap(int index, const std::string &name)
{
    this->layerSet->setColorMap(index, name);
    emit this->dataChanged(this->index(index), this->index(index));
}

int LayerListModel::rowCount(const QModelIndex &parent) const
{
    return this->layerSet->size();
}

QVariant LayerListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole: {
        return QString::fromStdString(this->layerSet->filepath(index.row()));
    }
    case Qt::CheckStateRole: {
        return this->layerSet->isVisible(index.row()) ? Qt::Checked : Qt::Unchecked;
    }
    default: {
        return QVariant();
    }
    }
}

bool LayerListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!this->checkIndex(index)) {
        return false;
    }

    if (role == Qt::CheckStateRole) {
        this->layerSet->setVisible(index.row(), value.toBool());
        emit this->dataChanged(index, index, { role });
        return true;
    }

    return false;
}

Qt::ItemFlags LayerListModel::flags(const QModelIndex &index) const
{
    auto defaults = QAbstractListModel::flags(index);
    if (index.isValid()) {
        return defaults | Qt::ItemIsUserCheckable | Qt::ItemIsDragEnabled;
    }

    return defaults | Qt::ItemIsDropEnabled;
}

Qt::DropActions LayerListModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QStringList LayerListModel::mimeTypes() const
{
    return { this->layerMimeType };
}

QMimeData *LayerListModel::mimeData(const QModelIndexList &indexes) const
{
    auto mimeData = new QMimeData;
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    stream << indexes.front().row();
    mimeData->setData(this->layerMimeType, encodedData);
    return mimeData;
}

bool LayerListModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row,
                                     int column, const QModelIndex &parent)
{
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);

    return data->hasFormat(this->layerMimeType);
}

bool LayerListModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
                                  const QModelIndex &parent)
{
    if (!this->canDropMimeData(data, action, row, column, parent)) {
        return false;
    }

    if (action == Qt::IgnoreAction) {
        return true;
    }

    QByteArray encodedData = data->data(this->layerMimeType);
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    int idxSrc;
    stream >> idxSrc;

    if (!this->layerSet->moveLayer(idxSrc, row)) {
        return false;
    }

    emit this->dataChanged(this->index(0), this->index(this->rowCount() - 1));

    return true;
}
