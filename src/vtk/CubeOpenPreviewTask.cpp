#include "CubeOpenPreviewTask.h"

#include "vtkFITSReader.h"
#include "vtkMomentMapFilter.h"

#include <fitsio.h>

#include <QDebug>
#include <QElapsedTimer>

#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkTrivialProducer.h>

#include <cmath>
#include <limits>

using namespace Qt::StringLiterals;

namespace {
constexpr int maxPreviewSpatialAxis = 96;

int computeSpatialStride(long size)
{
    return std::max(1L, static_cast<long>(std::ceil(
                                 static_cast<double>(size) / static_cast<double>(maxPreviewSpatialAxis))));
}

void computeStats(vtkImageData *imageData, std::array<double, 2> &range, double &mean, double &rms)
{
    range = { std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest() };
    double sum = 0.;
    double sum2 = 0.;
    vtkIdType count = 0;

    const auto *scalars = static_cast<float *>(imageData->GetScalarPointer());
    const vtkIdType nels = imageData->GetNumberOfPoints();
    for (vtkIdType i = 0; i < nels; ++i) {
        const float value = scalars[i];
        if (!std::isfinite(value)) {
            continue;
        }

        range[0] = std::min(range[0], static_cast<double>(value));
        range[1] = std::max(range[1], static_cast<double>(value));
        sum += value;
        sum2 += value * value;
        ++count;
    }

    if (count == 0) {
        range = { 0., 0. };
        mean = 0.;
        rms = 0.;
        return;
    }

    mean = sum / static_cast<double>(count);
    rms = std::sqrt((sum2 / static_cast<double>(count)) - mean * mean);
}
}

CubeOpenStageResult loadCubeOpenPreview(const QString &filepath)
{
    QElapsedTimer totalTimer;
    totalTimer.start();
    fitsfile *fptr = nullptr;
    int status = 0;
    if (fits_open_image(&fptr, filepath.toUtf8().constData(), READONLY, &status)) {
        return { false, u"Could not open FITS file."_s };
    }

    int naxis = 0;
    if (fits_get_img_dim(fptr, &naxis, &status)) {
        fits_close_file(fptr, &status);
        return { false, u"Could not read FITS dimensions."_s };
    }

    constexpr int maxaxis = 4;
    long naxes[maxaxis] = { 1l, 1l, 1l, 1l };
    if (fits_get_img_size(fptr, naxis, naxes, &status)) {
        fits_close_file(fptr, &status);
        return { false, u"Could not read FITS size."_s };
    }

    const long strideX = computeSpatialStride(naxes[0]);
    const long strideY = computeSpatialStride(naxes[1]);
    const long strideZ = 1l;

    const int dims[3] = { static_cast<int>(((naxes[0] - 1) / strideX) + 1),
                          static_cast<int>(((naxes[1] - 1) / strideY) + 1),
                          static_cast<int>(std::max(1l, naxes[2])) };

    vtkNew<vtkImageData> cubeImage;
    cubeImage->SetExtent(0, dims[0] - 1, 0, dims[1] - 1, 0, dims[2] - 1);
    cubeImage->AllocateScalars(VTK_FLOAT, 1);

    long fpixel[4] = { 1l, 1l, 1l, 1l };
    long lpixel[4] = { naxes[0], naxes[1], std::max(1l, naxes[2]), 1l };
    long inc[4] = { strideX, strideY, strideZ, 1l };

    auto *cubePtr = static_cast<float *>(cubeImage->GetScalarPointer());
    QElapsedTimer readTimer;
    readTimer.start();
    if (fits_read_subset(fptr, TFLOAT, fpixel, lpixel, inc, nullptr, cubePtr, nullptr, &status)) {
        fits_close_file(fptr, &status);
        return { false, u"Could not read FITS preview subset."_s };
    }
    qDebug().noquote() << QStringLiteral("[perf][cube] preview FITS subset read: %1 ms").arg(
            readTimer.elapsed());

    fits_close_file(fptr, &status);

    std::array<double, 2> cubeRange{ 0., 0. };
    double cubeMean = 0.;
    double cubeRms = 0.;
    QElapsedTimer statsTimer;
    statsTimer.start();
    computeStats(cubeImage, cubeRange, cubeMean, cubeRms);
    qDebug().noquote()
            << QStringLiteral("[perf][cube] preview stats: %1 ms").arg(statsTimer.elapsed());

    vtkNew<vtkTrivialProducer> previewSource;
    previewSource->SetOutput(cubeImage);

    vtkNew<vtkMomentMapFilter> moment;
    moment->SetInputConnection(previewSource->GetOutputPort());
    moment->Init(filepath.toStdString());
    moment->SetMomentOrder(0);
    QElapsedTimer momentTimer;
    momentTimer.start();
    moment->Update();
    qDebug().noquote() << QStringLiteral("[perf][cube] preview moment generation: %1 ms").arg(
            momentTimer.elapsed());

    vtkNew<vtkImageData> momentImage;
    momentImage->DeepCopy(moment->GetOutput());
    const double *momentRange = momentImage->GetScalarRange();
    qDebug().noquote()
            << QStringLiteral("[perf][cube] preview load: %1 ms").arg(totalTimer.elapsed());

    return { true,
             { },
             cubeImage,
             momentImage,
             { cubeRange[0], cubeRange[1] },
             { momentRange[0], momentRange[1] },
             { 0, dims[0] - 1, 0, dims[1] - 1, 0, dims[2] - 1 },
             cubeMean,
             cubeRms };
}

CubeOpenStageResult loadCubeOpenFull(const QString &filepath)
{
    QElapsedTimer totalTimer;
    totalTimer.start();
    vtkNew<vtkFITSReader> reader;
    reader->SetFileName(filepath.toUtf8());
    QElapsedTimer readTimer;
    readTimer.start();
    reader->Update();
    qDebug().noquote()
            << QStringLiteral("[perf][cube] full FITS read: %1 ms").arg(readTimer.elapsed());

    vtkNew<vtkImageData> cubeImage;
    QElapsedTimer deepCopyTimer;
    deepCopyTimer.start();
    cubeImage->DeepCopy(reader->GetOutput());
    qDebug().noquote()
            << QStringLiteral("[perf][cube] full DeepCopy: %1 ms").arg(deepCopyTimer.elapsed());
    const int *extent = cubeImage->GetExtent();
    qDebug().noquote()
            << QStringLiteral("[perf][cube] full load: %1 ms").arg(totalTimer.elapsed());

    return { true,
             { },
             cubeImage,
             nullptr,
             { reader->GetMin(), reader->GetMax() },
             { 0., 0. },
             { extent[0], extent[1], extent[2], extent[3], extent[4], extent[5] },
             reader->GetMean(),
             reader->GetRMS() };
}
