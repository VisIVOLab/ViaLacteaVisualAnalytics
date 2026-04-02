#include "ImageLayerSet.h"

#include "AstroUtils.h"
#include "ImageLayer.h"
#include "ImageLayerLoadTask.h"

#include "wcs.h"

#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>

#include <cmath>
#include <limits>
#include <utility>

ImageLayerSet::ImageLayerSet(const std::string &masterFilepath)
    : masterFilepath(masterFilepath), masterIdx(0)
{
    this->layers.push_back(std::make_unique<ImageLayer>(this->masterFilepath));
    this->refreshLayerNumbers();
}

ImageLayerSet::ImageLayerSet(const ImageLayerLoadResult &masterResult)
    : masterFilepath(masterResult.filepath), masterIdx(0)
{
    this->layers.push_back(std::make_unique<ImageLayer>(masterResult));
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
    if (!this->isValidIndex(index)) {
        return nullptr;
    }

    return this->layers[index]->getActor();
}

float ImageLayerSet::pixelValue(int index, int x, int y) const
{
    if (!this->isValidIndex(index)) {
        return std::numeric_limits<float>::quiet_NaN();
    }

    return this->layers[index]->getPixelValue(x, y);
}

vtkImageData *ImageLayerSet::imageData(int index) const
{
    if (!this->isValidIndex(index)) {
        return nullptr;
    }

    return this->layers[index]->getImageData();
}

vtkLookupTable *ImageLayerSet::lookupTable(int index) const
{
    if (!this->isValidIndex(index)) {
        return nullptr;
    }

    return this->layers[index]->getLookupTable();
}

double ImageLayerSet::layerOpacity(int index) const
{
    if (!this->isValidIndex(index)) {
        return 0.;
    }

    return this->layers[index]->getOpacity();
}

void ImageLayerSet::setLayerOpacity(int index, double opacity)
{
    if (!this->isValidIndex(index)) {
        return;
    }

    this->layers[index]->setOpacity(opacity);
}

bool ImageLayerSet::usingLogScale(int index) const
{
    if (!this->isValidIndex(index)) {
        return false;
    }

    return this->layers[index]->usingLogScale();
}

void ImageLayerSet::setLogScale(int index, bool flag)
{
    if (!this->isValidIndex(index)) {
        return;
    }

    this->layers[index]->setLogScale(flag);
}

std::string ImageLayerSet::colorMapName(int index) const
{
    if (!this->isValidIndex(index)) {
        return { };
    }

    return this->layers[index]->getColorMapName();
}

void ImageLayerSet::setColorMap(int index, const std::string &name)
{
    if (!this->isValidIndex(index)) {
        return;
    }

    this->layers[index]->setColorMap(name);
}

std::string ImageLayerSet::filepath(int index) const
{
    if (!this->isValidIndex(index)) {
        return { };
    }

    return this->layers[index]->getFilepath();
}

bool ImageLayerSet::isVisible(int index) const
{
    if (!this->isValidIndex(index)) {
        return false;
    }

    return this->layers[index]->isVisible();
}

void ImageLayerSet::setVisible(int index, bool visible)
{
    if (!this->isValidIndex(index)) {
        return;
    }

    this->layers[index]->setVisibility(visible);
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

vtkImageSlice *ImageLayerSet::addLayer(const ImageLayerLoadResult &result)
{
    this->layers.push_back(std::make_unique<ImageLayer>(result));
    auto &layer = *this->layers.back();

    layer.getActor()->GetProperty()->SetOpacity(0.5);

    this->refreshLayerNumbers();
    return layer.getActor();
}

vtkImageSlice *ImageLayerSet::replaceMasterLayer(const ImageLayerLoadResult &result)
{
    if (!this->isValidIndex(this->masterIdx)) {
        return nullptr;
    }

    this->masterFilepath = result.filepath;
    this->layers[this->masterIdx] = std::make_unique<ImageLayer>(result);
    this->refreshLayerNumbers();
    return this->layers[this->masterIdx]->getActor();
}

vtkImageSlice *ImageLayerSet::updateMasterLayer(const ImageLayerLoadResult &result)
{
    if (!this->isValidIndex(this->masterIdx)) {
        return nullptr;
    }

    this->masterFilepath = result.filepath;
    this->layers[this->masterIdx]->applyLoadResult(result);
    this->refreshLayerNumbers();
    return this->layers[this->masterIdx]->getActor();
}

bool ImageLayerSet::moveLayer(int sourceIndex, int destinationRow)
{
    const int n = this->size();
    if (!this->isValidIndex(sourceIndex) || destinationRow < 0 || destinationRow > n) {
        return false;
    }

    int insertIndex = destinationRow;
    if (sourceIndex < destinationRow) {
        insertIndex -= 1;
    }

    if (insertIndex == sourceIndex || insertIndex < 0 || insertIndex >= n) {
        return false;
    }

    auto moved = std::move(this->layers[sourceIndex]);
    this->layers.erase(this->layers.begin() + sourceIndex);
    this->layers.insert(this->layers.begin() + insertIndex, std::move(moved));

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
