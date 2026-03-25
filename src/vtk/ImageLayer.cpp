#include "ImageLayer.h"

#include "ColorMaps.h"
#include "ImageLayerLoadTask.h"
#include "vtkFITSReader.h"

#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkLookupTable.h>
#include <vtkTrivialProducer.h>
#include <vtkTransform.h>

ImageLayer::ImageLayer(const std::string &filepath) : readerBacked(true), filepath(filepath)
{
    this->reader->SetFileName(this->filepath.c_str());
    this->reader->Update();
    this->imageData = this->reader->GetOutput();
    this->source->SetOutput(this->imageData);
    this->scalarRange[0] = this->reader->GetMin();
    this->scalarRange[1] = this->reader->GetMax();

    this->lut->SetTableRange(this->scalarRange[0], this->scalarRange[1]);
    this->lut->SetNanColor(1., 1., 1., 1.);
    ColorMaps::SetColorMap(this->lut);

    vtkNew<vtkImageMapToColors> colors;
    colors->SetInputConnection(this->source->GetOutputPort());
    colors->SetLookupTable(this->lut);

    vtkNew<vtkImageSliceMapper> mapper;
    mapper->SetInputConnection(colors->GetOutputPort());
    mapper->BorderOn();

    this->actor->SetMapper(mapper);
    this->actor->GetProperty()->SetInterpolationTypeToNearest();
}

ImageLayer::ImageLayer(const ImageLayerLoadResult &result)
    : readerBacked(false), filepath(result.filepath), imageData(result.imageData)
{
    this->imageData->SetSpacing(result.spacing[0], result.spacing[1], result.spacing[2]);
    this->imageData->SetOrigin(result.origin[0], result.origin[1], result.origin[2]);
    this->source->SetOutput(this->imageData);
    this->scalarRange[0] = result.scalarRange[0];
    this->scalarRange[1] = result.scalarRange[1];

    this->lut->SetTableRange(this->scalarRange[0], this->scalarRange[1]);
    this->lut->SetNanColor(1., 1., 1., 1.);
    ColorMaps::SetColorMap(this->lut);

    vtkNew<vtkImageMapToColors> colors;
    colors->SetInputConnection(this->source->GetOutputPort());
    colors->SetLookupTable(this->lut);

    vtkNew<vtkImageSliceMapper> mapper;
    mapper->SetInputConnection(colors->GetOutputPort());
    mapper->BorderOn();

    this->actor->SetMapper(mapper);
    this->actor->GetProperty()->SetInterpolationTypeToNearest();

    vtkNew<vtkTransform> transform;
    transform->Translate(result.origin[0], result.origin[1], result.origin[2]);
    transform->RotateWXYZ(result.rotationDegrees, 0., 0., 1.);
    transform->Translate(-result.origin[0], -result.origin[1], -result.origin[2]);
    this->actor->SetUserTransform(transform);
}

ImageLayer::~ImageLayer() = default;

std::string ImageLayer::getFilepath() const
{
    return this->filepath;
}

float ImageLayer::getPixelValue(int x, int y) const
{
    if (this->readerBacked) {
        return this->reader->GetValue(x, y);
    }

    if (!this->imageData) {
        return std::numeric_limits<float>::quiet_NaN();
    }

    const int *dims = this->imageData->GetDimensions();
    if (x < 0 || y < 0 || x >= dims[0] || y >= dims[1]) {
        return std::numeric_limits<float>::quiet_NaN();
    }

    return this->imageData->GetScalarComponentAsFloat(x, y, 0, 0);
}

vtkImageSlice *ImageLayer::getActor() const
{
    return this->actor;
}

vtkImageData *ImageLayer::getImageData() const
{
    return this->imageData;
}

vtkLookupTable *ImageLayer::getLookupTable() const
{
    return this->lut;
}

bool ImageLayer::isVisible() const
{
    return this->actor->GetVisibility();
}

void ImageLayer::setVisibility(bool visible)
{
    this->actor->SetVisibility(visible);
}

double ImageLayer::getOpacity() const
{
    return this->actor->GetProperty()->GetOpacity();
}

void ImageLayer::setOpacity(double opacity)
{
    this->actor->GetProperty()->SetOpacity(opacity);
}

bool ImageLayer::usingLogScale() const
{
    return this->lut->UsingLogScale();
}

void ImageLayer::setLogScale(bool flag)
{
    double range[] = { this->scalarRange[0], this->scalarRange[1] };
    if (flag) {
        if (range[0] <= 0. && range[1] > 0.) {
            range[0] = range[1] * 1e-4;
            range[0] = (range[0] < 1.) ? range[0] : 1.;
        }
        this->lut->SetTableRange(range[0], range[1]);
        this->lut->SetScaleToLog10();
    } else {
        this->lut->SetScaleToLinear();
        this->lut->SetTableRange(range[0], range[1]);
    }
}

std::string ImageLayer::getColorMapName() const
{
    return this->lut->GetObjectName();
}

void ImageLayer::setColorMap(const std::string &name)
{
    ColorMaps::SetColorMap(this->lut, name);
}

void ImageLayer::setOrigin(const double *origin)
{
    if (this->readerBacked) {
        this->reader->SetDataOrigin(origin);
        this->reader->Update();
        this->imageData = this->reader->GetOutput();
        this->source->SetOutput(this->imageData);
    } else if (this->imageData) {
        this->imageData->SetOrigin(origin);
    }
}

void ImageLayer::setSpacing(const double *spacing)
{
    if (this->readerBacked) {
        this->reader->SetDataSpacing(spacing);
        this->reader->Update();
        this->imageData = this->reader->GetOutput();
        this->source->SetOutput(this->imageData);
    } else if (this->imageData) {
        this->imageData->SetSpacing(spacing);
    }
}

void ImageLayer::setRotation(double angle)
{
    double bounds[6];
    if (this->imageData) {
        this->imageData->GetBounds(bounds);
    }

    vtkNew<vtkTransform> transform;
    transform->Translate(bounds[0], bounds[2], bounds[4]);
    transform->RotateWXYZ(angle, 0., 0., 1.);
    transform->Translate(-bounds[0], -bounds[2], -bounds[4]);
    this->actor->SetUserTransform(transform);
}
