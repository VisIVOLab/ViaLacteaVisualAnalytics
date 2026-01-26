#include "LayerListModel.h"
#include "AstroUtils.h"

#include "wcs.h"

#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkMath.h>

#include <QDataStream>
#include <QIODevice>
#include <QMimeData>

#include <algorithm>
#include <memory>

LayerListModel::LayerListModel(const std::string &filepath, QObject *parent)
    : QAbstractListModel(parent),
      masterFilepath(filepath),
      masterIdx(0),
      layerMimeType("application/x-image-layer")
{
    this->layers.push_back(std::make_unique<ImageLayer>(this->masterFilepath));
    this->layers.front()->getActor()->GetProperty()->SetLayerNumber(this->masterIdx);
}

vtkImageSlice *LayerListModel::getMasterLayerActor() const
{
    return this->layers[this->masterIdx]->getActor();
}

int LayerListModel::getMasterIndex() const
{
    return this->masterIdx;
}

LayerListModel::~LayerListModel() = default;

vtkImageSlice *LayerListModel::addLayer(const std::string &filepath)
{
    this->layers.push_back(std::make_unique<ImageLayer>(filepath));
    const int lastIdx = this->layers.size() - 1;
    auto &newLayer = this->layers.back();
    AstroUtils astroMaster(this->layers[this->masterIdx]->getFilepath());
    AstroUtils astroNewLayer(newLayer->getFilepath());

    // Scaling
    const double scalingFactor = astroNewLayer.getSecPix() / astroMaster.getSecPix();
    const double spacing[3] = { scalingFactor, scalingFactor, 1. };
    newLayer->setSpacing(spacing);

    // Origin
    double posNewLayer[2];
    double pix[2] = { 0., 0. };
    astroNewLayer.xy2sky(pix, posNewLayer, WCS_J2000);
    astroMaster.sky2xy(posNewLayer, pix, WCS_J2000);
    const double origin[3] = { pix[0], pix[1], 0. };
    newLayer->setOrigin(origin);

    // Rotation
    pix[0] = 0.;
    pix[1] = 100.;
    astroNewLayer.xy2sky(pix, posNewLayer, WCS_J2000);
    astroMaster.sky2xy(posNewLayer, pix, WCS_J2000);
    const double m = std::abs((pix[1] - origin[1]) / (pix[0] - origin[0]));
    const double angle = 90. - std::atan(m) * 180. / vtkMath::Pi();
    newLayer->setRotation(angle);

    // Properties
    auto actor = newLayer->getActor();
    actor->GetProperty()->SetLayerNumber(lastIdx);
    actor->GetProperty()->SetOpacity(0.5);

    emit this->dataChanged(this->index(lastIdx), this->index(lastIdx));
    return actor;
}

float LayerListModel::getPixelValue(int index, int x, int y) const
{
    if (index < this->layers.size()) {
        return this->layers[index]->getPixelValue(x, y);
    }
    return std::numeric_limits<float>::quiet_NaN();
}

vtkImageData *LayerListModel::getImageData(int index) const
{
    if (index < this->layers.size()) {
        return this->layers[index]->getImageData();
    }
    return nullptr;
}

vtkLookupTable *LayerListModel::getLookupTable(int index) const
{
    if (index < this->layers.size()) {
        return this->layers[index]->getLookupTable();
    }
    return nullptr;
}

double LayerListModel::getLayerOpacity(int index) const
{
    if (index < this->layers.size()) {
        return this->layers[index]->getOpacity();
    }
    return 0.;
}

void LayerListModel::setLayerOpacity(int index, double opacity)
{
    if (index < this->layers.size()) {
        this->layers[index]->setOpacity(opacity);
        emit this->dataChanged(this->index(index), this->index(index));
    }
}

bool LayerListModel::usingLogScale(int index) const
{
    if (index < this->layers.size()) {
        return this->layers[index]->usingLogScale();
    }
    return false;
}

void LayerListModel::setLogScale(int index, bool flag)
{
    if (index < this->layers.size()) {
        this->layers[index]->setLogScale(flag);
        emit this->dataChanged(this->index(index), this->index(index));
    }
}

std::string LayerListModel::getColorMapName(int index) const
{
    if (index < this->layers.size()) {
        return this->layers[index]->getColorMapName();
    }
    return { };
}

void LayerListModel::setColorMap(int index, const std::string &name)
{
    if (index < this->layers.size()) {
        this->layers[index]->setColorMap(name);
        emit this->dataChanged(this->index(index), this->index(index));
    }
}

int LayerListModel::rowCount(const QModelIndex &parent) const
{
    return this->layers.size();
}

QVariant LayerListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole: {
        return QString::fromStdString(this->layers[index.row()]->getFilepath());
    }
    case Qt::CheckStateRole: {
        return this->layers[index.row()]->isVisible() ? Qt::Checked : Qt::Unchecked;
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
        this->layers[index.row()]->setVisibility(value.toBool());
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

    if (idxSrc > row) {
        for (int i = idxSrc - 1; i >= row; --i) {
            std::iter_swap(this->layers.begin() + i, this->layers.begin() + i + 1);
        }
    } else {
        for (int i = idxSrc + 1; i < row; ++i) {
            std::iter_swap(this->layers.begin() + i, this->layers.begin() + i - 1);
        }
    }

    for (int i = 0; i < this->layers.size(); ++i) {
        this->layers[i]->getActor()->GetProperty()->SetLayerNumber(i);
        if (this->layers[i]->getFilepath() == this->masterFilepath) {
            this->masterIdx = i;
        }
    }

    emit this->dataChanged(this->index(0), this->index(layers.size() - 1));

    return true;
}
