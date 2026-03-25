#include "ImageLayerSet.h"

#include "AstroUtils.h"
#include "ImageLayer.h"

#include "wcs.h"

#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>

#include <algorithm>
#include <cmath>
#include <limits>

ImageLayerSet::ImageLayerSet(const std::string &masterFilepath)
    : masterFilepath(masterFilepath), masterIdx(0)
{
    this->layers.push_back(std::make_unique<ImageLayer>(this->masterFilepath));
    this->refreshLayerNumbers();
}

ImageLayerSet::~ImageLayerSet() = default;

int ImageLayerSet::size() const
{
    return static_cast<int>(this->layers.size());
}

int ImageLayerSet::masterIndex() const
{
    return this->masterIdx;
}

vtkImageSlice *ImageLayerSet::masterLayerActor() const
{
    return this->layers[this->masterIdx]->getActor();
}

vtkImageSlice *ImageLayerSet::layerActor(int index) const
{
    if (this->isValidIndex(index)) {
        return this->layers[index]->getActor();
    }

    return nullptr;
}

float ImageLayerSet::pixelValue(int index, int x, int y) const
{
    if (this->isValidIndex(index)) {
        return this->layers[index]->getPixelValue(x, y);
    }

    return std::numeric_limits<float>::quiet_NaN();
}

vtkImageData *ImageLayerSet::imageData(int index) const
{
    if (this->isValidIndex(index)) {
        return this->layers[index]->getImageData();
    }

    return nullptr;
}

vtkLookupTable *ImageLayerSet::lookupTable(int index) const
{
    if (this->isValidIndex(index)) {
        return this->layers[index]->getLookupTable();
    }

    return nullptr;
}

double ImageLayerSet::layerOpacity(int index) const
{
    if (this->isValidIndex(index)) {
        return this->layers[index]->getOpacity();
    }

    return 0.;
}

void ImageLayerSet::setLayerOpacity(int index, double opacity)
{
    if (this->isValidIndex(index)) {
        this->layers[index]->setOpacity(opacity);
    }
}

bool ImageLayerSet::usingLogScale(int index) const
{
    if (this->isValidIndex(index)) {
        return this->layers[index]->usingLogScale();
    }

    return false;
}

void ImageLayerSet::setLogScale(int index, bool flag)
{
    if (this->isValidIndex(index)) {
        this->layers[index]->setLogScale(flag);
    }
}

std::string ImageLayerSet::colorMapName(int index) const
{
    if (this->isValidIndex(index)) {
        return this->layers[index]->getColorMapName();
    }

    return { };
}

void ImageLayerSet::setColorMap(int index, const std::string &name)
{
    if (this->isValidIndex(index)) {
        this->layers[index]->setColorMap(name);
    }
}

std::string ImageLayerSet::filepath(int index) const
{
    if (this->isValidIndex(index)) {
        return this->layers[index]->getFilepath();
    }

    return { };
}

bool ImageLayerSet::isVisible(int index) const
{
    if (this->isValidIndex(index)) {
        return this->layers[index]->isVisible();
    }

    return false;
}

void ImageLayerSet::setVisible(int index, bool visible)
{
    if (this->isValidIndex(index)) {
        this->layers[index]->setVisibility(visible);
    }
}

vtkImageSlice *ImageLayerSet::addLayer(const std::string &filepath)
{
    this->layers.push_back(std::make_unique<ImageLayer>(filepath));
    auto &layer = *this->layers.back();

    this->registerLayerToMaster(layer);
    layer.getActor()->GetProperty()->SetOpacity(0.5);

    this->refreshLayerNumbers();
    return layer.getActor();
}

bool ImageLayerSet::moveLayer(int sourceIndex, int destinationRow)
{
    if (!this->isValidIndex(sourceIndex) || destinationRow < 0 || destinationRow > this->size()) {
        return false;
    }

    if (sourceIndex > destinationRow) {
        for (int i = sourceIndex - 1; i >= destinationRow; --i) {
            std::iter_swap(this->layers.begin() + i, this->layers.begin() + i + 1);
        }
    } else {
        for (int i = sourceIndex + 1; i < destinationRow; ++i) {
            std::iter_swap(this->layers.begin() + i, this->layers.begin() + i - 1);
        }
    }

    this->refreshLayerNumbers();
    return true;
}

bool ImageLayerSet::isValidIndex(int index) const
{
    return index >= 0 && index < this->size();
}

void ImageLayerSet::registerLayerToMaster(ImageLayer &layer)
{
    AstroUtils astroMaster(this->layers[this->masterIdx]->getFilepath());
    AstroUtils astroNewLayer(layer.getFilepath());

    const double scalingFactor = astroNewLayer.getSecPix() / astroMaster.getSecPix();
    const double spacing[3] = { scalingFactor, scalingFactor, 1. };
    layer.setSpacing(spacing);

    double posNewLayer[2];
    double pix[2] = { 0., 0. };
    astroNewLayer.xy2sky(pix, posNewLayer, WCS_J2000);
    astroMaster.sky2xy(posNewLayer, pix, WCS_J2000);
    const double origin[3] = { pix[0], pix[1], 0. };
    layer.setOrigin(origin);

    pix[0] = 0.;
    pix[1] = 100.;
    astroNewLayer.xy2sky(pix, posNewLayer, WCS_J2000);
    astroMaster.sky2xy(posNewLayer, pix, WCS_J2000);
    const double m = std::abs((pix[1] - origin[1]) / (pix[0] - origin[0]));
    const double angle = 90. - std::atan(m) * 180. / vtkMath::Pi();
    layer.setRotation(angle);
}

void ImageLayerSet::refreshLayerNumbers()
{
    for (int i = 0; i < this->size(); ++i) {
        this->layers[i]->getActor()->GetProperty()->SetLayerNumber(i);
        if (this->layers[i]->getFilepath() == this->masterFilepath) {
            this->masterIdx = i;
        }
    }
}
