#include "ImageLayer.h"

#include "ColorMaps.h"
#include "ImageLayerLoadTask.h"
#include "vtkFITSReader.h"

#include <QDebug>

#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkLookupTable.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkTrivialProducer.h>
#include <vtkTransform.h>

#include <algorithm>
#include <cmath>
#include <vector>

namespace {
constexpr qint64 previewPixelThreshold = 25000000;
constexpr int previewMaxLongestSide = 1536;

qint64 imagePixelCount(const int *dims)
{
    if (!dims || dims[0] <= 0 || dims[1] <= 0) {
        return 0;
    }
    return static_cast<qint64>(dims[0]) * static_cast<qint64>(dims[1]);
}

vtkSmartPointer<vtkImageData> buildPreviewImage(vtkImageData *sourceImage)
{
    if (!sourceImage) {
        return nullptr;
    }

    const int *dims = sourceImage->GetDimensions();
    if (!dims || dims[0] <= 0 || dims[1] <= 0) {
        return nullptr;
    }

    const int longestSide = std::max(dims[0], dims[1]);
    if (longestSide <= previewMaxLongestSide) {
        return nullptr;
    }

    const double scale =
            static_cast<double>(previewMaxLongestSide) / static_cast<double>(longestSide);
    const int previewWidth = std::max(1, static_cast<int>(std::lround(dims[0] * scale)));
    const int previewHeight = std::max(1, static_cast<int>(std::lround(dims[1] * scale)));

    auto preview = vtkSmartPointer<vtkImageData>::New();
    preview->SetDimensions(previewWidth, previewHeight, 1);
    preview->SetExtent(0, previewWidth - 1, 0, previewHeight - 1, 0, 0);
    preview->AllocateScalars(sourceImage->GetScalarType(),
                             sourceImage->GetNumberOfScalarComponents());

    const double *origin = sourceImage->GetOrigin();
    const double *spacing = sourceImage->GetSpacing();
    const double xFactor = static_cast<double>(dims[0]) / static_cast<double>(previewWidth);
    const double yFactor = static_cast<double>(dims[1]) / static_cast<double>(previewHeight);
    preview->SetOrigin(origin[0], origin[1], origin[2]);
    preview->SetSpacing(spacing[0] * xFactor, spacing[1] * yFactor, spacing[2]);

    auto *sourceScalars = sourceImage->GetPointData() ? sourceImage->GetPointData()->GetScalars()
                                                      : nullptr;
    auto *previewScalars =
            preview->GetPointData() ? preview->GetPointData()->GetScalars() : nullptr;
    if (!sourceScalars || !previewScalars) {
        return nullptr;
    }

    const int components = sourceImage->GetNumberOfScalarComponents();
    std::vector<double> tuple(components, 0.);
    for (int y = 0; y < previewHeight; ++y) {
        const int sourceY =
                std::min(dims[1] - 1, static_cast<int>(std::floor((y + 0.5) * yFactor)));
        for (int x = 0; x < previewWidth; ++x) {
            const int sourceX =
                    std::min(dims[0] - 1, static_cast<int>(std::floor((x + 0.5) * xFactor)));
            sourceScalars->GetTuple(sourceY * dims[0] + sourceX, tuple.data());
            previewScalars->SetTuple(y * previewWidth + x, tuple.data());
        }
    }

    preview->Modified();
    return preview;
}
} // namespace

ImageLayer::ImageLayer(const std::string &filepath) : readerBacked(true), filepath(filepath)
{
    this->reader->SetFileName(this->filepath.c_str());
    this->reader->Update();
    this->imageData = this->reader->GetOutput();
    this->scalarRange[0] = this->reader->GetMin();
    this->scalarRange[1] = this->reader->GetMax();

    this->lut->SetTableRange(this->scalarRange[0], this->scalarRange[1]);
    this->lut->SetNanColor(1., 1., 1., 1.);
    ColorMaps::SetColorMap(this->lut);
    this->initializeDisplayPipeline();
    this->rebuildPreviewImageIfNeeded("local");
    this->updateDisplaySource();
}

ImageLayer::ImageLayer(const ImageLayerLoadResult &result)
    : readerBacked(false), filepath(result.filepath), imageData(result.imageData)
{
    this->imageData->SetSpacing(result.spacing[0], result.spacing[1], result.spacing[2]);
    this->imageData->SetOrigin(result.origin[0], result.origin[1], result.origin[2]);
    this->scalarRange[0] = result.scalarRange[0];
    this->scalarRange[1] = result.scalarRange[1];

    this->lut->SetTableRange(this->scalarRange[0], this->scalarRange[1]);
    this->lut->SetNanColor(1., 1., 1., 1.);
    ColorMaps::SetColorMap(this->lut);
    this->initializeDisplayPipeline();
    this->rebuildPreviewImageIfNeeded("remote");
    this->updateDisplaySource();

    vtkNew<vtkTransform> transform;
    transform->Translate(result.origin[0], result.origin[1], result.origin[2]);
    transform->RotateWXYZ(result.rotationDegrees, 0., 0., 1.);
    transform->Translate(-result.origin[0], -result.origin[1], -result.origin[2]);
    this->actor->SetUserTransform(transform);
}

ImageLayer::~ImageLayer() = default;

void ImageLayer::initializeDisplayPipeline()
{
    this->colors = vtkSmartPointer<vtkImageMapToColors>::New();
    this->colors->SetLookupTable(this->lut);

    vtkNew<vtkImageSliceMapper> mapper;
    mapper->SetInputConnection(this->colors->GetOutputPort());
    mapper->BorderOn();

    this->actor->SetMapper(mapper);
    this->actor->GetProperty()->SetInterpolationTypeToNearest();
}

void ImageLayer::updateDisplaySource()
{
    vtkImageData *display = this->displayImageData ? this->displayImageData.GetPointer()
                                                   : this->imageData.GetPointer();
    this->source->SetOutput(display);
    if (this->colors) {
        this->colors->SetInputData(display);
        this->colors->Update();
    }
}

void ImageLayer::rebuildPreviewImageIfNeeded(const char *context)
{
    this->displayImageData = nullptr;
    this->previewModeActive = false;

    if (!this->imageData) {
        return;
    }

    const int *dims = this->imageData->GetDimensions();
    const qint64 pixels = imagePixelCount(dims);
    if (pixels < previewPixelThreshold) {
        qDebug().noquote()
                << QStringLiteral("[image-preview] context=%1 original=%2x%3 preview=off")
                           .arg(QString::fromUtf8(context ? context : "unknown"))
                           .arg(dims ? dims[0] : 0)
                           .arg(dims ? dims[1] : 0);
        return;
    }

    this->displayImageData = buildPreviewImage(this->imageData);
    this->previewModeActive = (this->displayImageData != nullptr);

    const int *previewDims =
            this->displayImageData ? this->displayImageData->GetDimensions() : nullptr;
    qDebug().noquote()
            << QStringLiteral("[image-preview] context=%1 original=%2x%3 pixels=%4 preview=%5 "
                              "preview_dims=%6x%7")
                       .arg(QString::fromUtf8(context ? context : "unknown"))
                       .arg(dims ? dims[0] : 0)
                       .arg(dims ? dims[1] : 0)
                       .arg(pixels)
                       .arg(this->previewModeActive ? QStringLiteral("on")
                                                   : QStringLiteral("off"))
                       .arg(previewDims ? previewDims[0] : 0)
                       .arg(previewDims ? previewDims[1] : 0);
}

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
    } else if (this->imageData) {
        this->imageData->SetOrigin(origin);
    }
    this->rebuildPreviewImageIfNeeded("setOrigin");
    this->updateDisplaySource();
}

void ImageLayer::setSpacing(const double *spacing)
{
    if (this->readerBacked) {
        this->reader->SetDataSpacing(spacing);
        this->reader->Update();
        this->imageData = this->reader->GetOutput();
    } else if (this->imageData) {
        this->imageData->SetSpacing(spacing);
    }
    this->rebuildPreviewImageIfNeeded("setSpacing");
    this->updateDisplaySource();
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
