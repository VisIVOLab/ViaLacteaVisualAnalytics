#include "ImageLayer.h"

#include "ColorMaps.h"
#include "vtkFITSReader.h"

#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkLookupTable.h>
#include <vtkTransform.h>

ImageLayer::ImageLayer(const std::string &filepath) : filepath(filepath)
{
    this->reader->SetFileName(this->filepath.c_str());
    this->reader->Update();

    this->lut->SetTableRange(this->reader->GetMin(), this->reader->GetMax());
    this->lut->SetNanColor(1., 1., 1., 1.);
    ColorMaps::SetColorMap(this->lut);

    vtkNew<vtkImageMapToColors> colors;
    colors->SetInputConnection(this->reader->GetOutputPort());
    colors->SetLookupTable(this->lut);

    vtkNew<vtkImageSliceMapper> mapper;
    mapper->SetInputConnection(colors->GetOutputPort());
    mapper->BorderOn();

    this->actor->SetMapper(mapper);
    this->actor->GetProperty()->SetInterpolationTypeToNearest();
}

ImageLayer::~ImageLayer() = default;

std::string ImageLayer::getFilepath() const
{
    return this->filepath;
}

float ImageLayer::getPixelValue(int x, int y) const
{
    return this->reader->GetValue(x, y);
}

vtkImageSlice *ImageLayer::getActor() const
{
    return this->actor;
}

vtkImageData *ImageLayer::getImageData() const
{
    return this->reader->GetOutput();
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
    double range[] = { this->reader->GetMin(), this->reader->GetMax() };
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
    this->reader->SetDataOrigin(origin);
    this->reader->Update();
}

void ImageLayer::setSpacing(const double *spacing)
{
    this->reader->SetDataSpacing(spacing);
    this->reader->Update();
}

void ImageLayer::setRotation(double angle)
{
    double bounds[6];
    this->reader->Update();
    this->reader->GetOutput()->GetBounds(bounds);

    vtkNew<vtkTransform> transform;
    transform->Translate(bounds[0], bounds[2], bounds[4]);
    transform->RotateWXYZ(angle, 0., 0., 1.);
    transform->Translate(-bounds[0], -bounds[2], -bounds[4]);
    this->actor->SetUserTransform(transform);
}
