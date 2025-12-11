#include "scene_manager.h"

#include <QBuffer>
#include <QImage>

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageShiftScale.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkUnsignedCharArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkExtractVOI.h>
#include <vtkMarchingCubes.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkProperty.h>
#include <vtkActor.h>
#include <vtkContourFilter.h>
#include <vtkCamera.h>
#include <vtkSmartPointer.h>

#include "../src/vtkfitsreader2.h"

SceneManager::SceneManager()
    : m_window(nullptr),
      m_renderer(nullptr),
      m_imageActor(nullptr),
      m_sliceImage(nullptr),
      m_volumeImage(nullptr),
      m_shift(nullptr),
      m_sliceExtract(nullptr),
      m_currentSlice(0),
      m_hasWindowLevel(false),
      m_windowWL(0.0),
      m_level(0.0) {
}

SceneManager::~SceneManager() {
    resetScene();
    if (m_renderer) { m_renderer->Delete(); m_renderer = nullptr; }
    if (m_window) { m_window->Delete(); m_window = nullptr; }
}

bool SceneManager::ensureScene() {
    if (!m_window) {
        m_window = vtkRenderWindow::New();
        m_window->SetOffScreenRendering(1);
    }
    if (!m_renderer) {
        m_renderer = vtkRenderer::New();
        m_window->AddRenderer(m_renderer);
        m_renderer->SetBackground(0.05, 0.05, 0.05);
    }
    return true;
}

void SceneManager::resetScene() {
    if (m_renderer) {
        m_renderer->RemoveAllViewProps();
    }
    if (m_imageActor) { m_imageActor->Delete(); m_imageActor = nullptr; }
    if (m_sliceImage) { m_sliceImage->Delete(); m_sliceImage = nullptr; }
    if (m_volumeImage) { m_volumeImage->Delete(); m_volumeImage = nullptr; }
    if (m_shift) { m_shift->Delete(); m_shift = nullptr; }
    if (m_sliceExtract) { m_sliceExtract->Delete(); m_sliceExtract = nullptr; }
    m_loadedSource.clear();
    m_numSlices = 0;
    m_range[0] = 0.0; m_range[1] = 1.0;
    m_currentSlice = 0;
    m_hasWindowLevel = false;
}

QJsonObject SceneManager::loadFits(const QString &path) {
    ensureScene();
    resetScene();

    vtkSmartPointer<vtkFitsReader2> reader = vtkSmartPointer<vtkFitsReader2>::New();
    reader->SetFileName(path.toStdString().c_str());
    reader->Update();

    vtkImageData *volume = reader->GetOutput();
    if (!volume) {
        return {{"_error", QStringLiteral("FITS load failed")}};
    }
    volume->Register(nullptr); // keep reference

    int dims[3];
    volume->GetDimensions(dims);
    if (dims[0] < 1 || dims[1] < 1 || dims[2] < 1) {
        return {{"_error", QStringLiteral("FITS dimensions invalid")}};
    }
    m_numSlices = dims[2];

    volume->GetScalarRange(m_range);
    m_volumeImage = volume;

    // Extract a mid-slice for 2D view
    const int k = m_numSlices > 0 ? m_numSlices / 2 : 0;
    vtkExtractVOI *extract = vtkExtractVOI::New();
    extract->SetInputData(volume);
    extract->SetVOI(0, dims[0] - 1, 0, dims[1] - 1, k, k);
    extract->Update();

    vtkImageData *slice = extract->GetOutput();
    slice->Register(nullptr);
    extract->Register(nullptr);
    m_sliceImage = slice;
    m_sliceExtract = extract;
    m_currentSlice = k;

    // Build shift/scale for slice with current window/level or full range
    double window = m_range[1] - m_range[0];
    double level = m_range[0] + 0.5 * window;
    if (m_hasWindowLevel && m_windowWL > 0.0) {
        window = m_windowWL;
        level = m_level;
    }
    if (m_shift) { m_shift->Delete(); m_shift = nullptr; }
    m_shift = vtkImageShiftScale::New();
    m_shift->SetInputData(slice);
    m_shift->SetShift(-(level - window * 0.5));
    m_shift->SetScale(window > 0 ? 255.0 / window : 1.0);
    m_shift->SetOutputScalarTypeToUnsignedChar();
    m_shift->ClampOverflowOn();
    m_shift->Update();

    vtkImageData *scaled = m_shift->GetOutput();
    scaled->Register(nullptr);
    m_shift->Register(nullptr);

    if (m_imageActor) { m_imageActor->Delete(); m_imageActor = nullptr; }
    m_imageActor = vtkImageActor::New();
    m_imageActor->SetInputData(scaled);
    int sliceExt[6];
    scaled->GetExtent(sliceExt);
    m_imageActor->SetDisplayExtent(sliceExt);

    m_renderer->RemoveAllViewProps();
    m_renderer->AddActor(m_imageActor);
    m_renderer->ResetCamera();
    m_loadedSource = path;

    return {{"status", QStringLiteral("ok")},
            {"dimensions", QJsonArray{dims[0], dims[1], m_numSlices}},
            {"slice", k},
            {"range", QJsonArray{m_range[0], m_range[1]}}};
}

QJsonObject SceneManager::setCamera(const QJsonObject &params) {
    ensureScene();
    vtkCamera *cam = m_renderer ? m_renderer->GetActiveCamera() : nullptr;
    if (!cam) return {{"_error", QStringLiteral("No renderer")}};
    const QJsonArray pos = params.value(QStringLiteral("position")).toArray();
    const QJsonArray foc = params.value(QStringLiteral("focal")).toArray();
    const QJsonArray up  = params.value(QStringLiteral("up")).toArray();
    if (pos.size() == 3) cam->SetPosition(pos[0].toDouble(), pos[1].toDouble(), pos[2].toDouble());
    if (foc.size() == 3) cam->SetFocalPoint(foc[0].toDouble(), foc[1].toDouble(), foc[2].toDouble());
    if (up.size() == 3)  cam->SetViewUp(up[0].toDouble(), up[1].toDouble(), up[2].toDouble());
    m_window->Render();
    return {{"status", QStringLiteral("ok")}};
}

QJsonObject SceneManager::setSlice(int slice) {
    ensureScene();
    if (!m_volumeImage) return {{"_error", QStringLiteral("No volume loaded")}};
    if (slice < 0 || slice >= m_numSlices) {
        return {{"_error", QStringLiteral("Slice out of range")}};
    }
    vtkExtractVOI *extract = m_sliceExtract;
    if (!extract) {
        extract = vtkExtractVOI::New();
        extract->SetInputData(m_volumeImage);
        m_sliceExtract = extract;
    }
    int dims[3];
    m_volumeImage->GetDimensions(dims);
    extract->SetVOI(0, dims[0] - 1, 0, dims[1] - 1, slice, slice);
    extract->Update();
    vtkImageData *s = extract->GetOutput();
    s->Register(nullptr);
    if (m_sliceImage) { m_sliceImage->Delete(); }
    m_sliceImage = s;
    m_currentSlice = slice;

    double window = m_range[1] - m_range[0];
    double level = m_range[0] + 0.5 * window;
    if (m_hasWindowLevel && m_windowWL > 0.0) {
        window = m_windowWL;
        level = m_level;
    }
    if (m_shift) { m_shift->Delete(); m_shift = nullptr; }
    m_shift = vtkImageShiftScale::New();
    m_shift->SetInputData(s);
    m_shift->SetShift(-(level - window * 0.5));
    m_shift->SetScale(window > 0 ? 255.0 / window : 1.0);
    m_shift->SetOutputScalarTypeToUnsignedChar();
    m_shift->ClampOverflowOn();
    m_shift->Update();

    vtkImageData *scaled = m_shift->GetOutput();
    scaled->Register(nullptr);
    m_shift->Register(nullptr);

    if (m_imageActor) { m_imageActor->Delete(); m_imageActor = nullptr; }
    m_imageActor = vtkImageActor::New();
    m_imageActor->SetInputData(scaled);
    int sliceExt[6];
    scaled->GetExtent(sliceExt);
    m_imageActor->SetDisplayExtent(sliceExt);

    m_renderer->RemoveAllViewProps();
    m_renderer->AddActor(m_imageActor);
    m_renderer->ResetCamera();
    return {{"status", QStringLiteral("ok")}, {"slice", slice}};
}

QJsonObject SceneManager::setWindowLevel(double window, double level) {
    ensureScene();
    if (!m_sliceImage) return {{"_error", QStringLiteral("No slice loaded")}};
    if (window <= 0.0) return {{"_error", QStringLiteral("Window must be >0")}};
    m_hasWindowLevel = true;
    m_windowWL = window;
    m_level = level;
    // Re-apply current slice to rebuild shift/actor
    return setSlice(m_currentSlice);
}

QJsonObject SceneManager::renderPng(int width, int height) {
    QByteArray rgba, depth;
    QJsonObject rawRes = renderRaw(width, height, rgba, depth);
    if (rawRes.contains(QStringLiteral("_error"))) {
        return rawRes;
    }
    // Encode RGBA to PNG via Qt
    QImage img(reinterpret_cast<const uchar *>(rgba.constData()),
               width, height, QImage::Format_RGBA8888);
    QByteArray png;
    QBuffer buf(&png);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    const QString b64 = QString::fromLatin1(png.toBase64());
    return {{"status", QStringLiteral("ok")}, {"image", b64}};
}

QJsonObject SceneManager::renderVolumePng(int width, int height, const QJsonObject &volumeParams) {
    QByteArray rgba, depth;
    QJsonObject res = renderRawVolume(width, height, volumeParams, rgba, depth);
    if (res.contains(QStringLiteral("_error"))) return res;
    QImage img(reinterpret_cast<const uchar *>(rgba.constData()),
               width, height, QImage::Format_RGBA8888);
    QByteArray png;
    QBuffer buf(&png);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    const QString b64 = QString::fromLatin1(png.toBase64());
    return {{"status", QStringLiteral("ok")}, {"image", b64}};
}

QJsonObject SceneManager::renderContourPng(int width, int height, const QJsonObject &params) {
    QByteArray rgba, depth;
    QJsonObject res = renderRawContour(width, height, params, rgba, depth);
    if (res.contains(QStringLiteral("_error"))) return res;
    QImage img(reinterpret_cast<const uchar *>(rgba.constData()),
               width, height, QImage::Format_RGBA8888);
    QByteArray png;
    QBuffer buf(&png);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    const QString b64 = QString::fromLatin1(png.toBase64());
    return {{"status", QStringLiteral("ok")}, {"image", b64}};
}

QJsonObject SceneManager::renderRawVolume(int width, int height, const QJsonObject &volumeParams, QByteArray &rgba, QByteArray &depth) {
    ensureScene();
    if (!m_volumeImage) {
        return {{"_error", QStringLiteral("No volume loaded")}};
    }
    if (width > 0 && height > 0) {
        m_window->SetSize(width, height);
    }

    // Iso-surface pipeline (marching cubes) per evidenziare strutture
    const double delta = m_range[1] - m_range[0];
    const double defaultIso = m_range[0] + 0.5 * delta;
    double iso = volumeParams.value(QStringLiteral("iso")).toDouble(defaultIso);
    // Clamp iso dentro il range valido
    if (iso <= m_range[0] || iso >= m_range[1]) {
        iso = defaultIso;
    }

    auto mc = vtkSmartPointer<vtkMarchingCubes>::New();
    mc->SetInputData(m_volumeImage);
    mc->ComputeNormalsOn();
    mc->SetValue(0, iso);

    auto normals = vtkSmartPointer<vtkPolyDataNormals>::New();
    normals->SetInputConnection(mc->GetOutputPort());
    normals->SplittingOff();

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(normals->GetOutputPort());
    mapper->ScalarVisibilityOff();

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // Colore/opacity di default, con override da params
    double color[3] = {0.8, 0.8, 1.0};
    double opacity = 1.0;
    const QJsonArray colorArr = volumeParams.value(QStringLiteral("color")).toArray();
    if (colorArr.size() >= 3) {
        color[0] = colorArr[0].toDouble(color[0]);
        color[1] = colorArr[1].toDouble(color[1]);
        color[2] = colorArr[2].toDouble(color[2]);
    }
    if (volumeParams.contains(QStringLiteral("opacity"))) {
        opacity = volumeParams.value(QStringLiteral("opacity")).toDouble(opacity);
    }
    actor->GetProperty()->SetColor(color);
    actor->GetProperty()->SetOpacity(opacity);

    m_renderer->RemoveAllViewProps();
    m_renderer->AddActor(actor);
    m_renderer->ResetCamera();
    m_renderer->ResetCameraClippingRange();
    m_window->Render();

    auto w2i = vtkSmartPointer<vtkWindowToImageFilter>::New();
    w2i->SetInput(m_window);
    w2i->SetInputBufferTypeToRGBA();
    w2i->ReadFrontBufferOff();
    w2i->Update();

    vtkUnsignedCharArray *rgbaData = vtkUnsignedCharArray::SafeDownCast(w2i->GetOutput()->GetPointData()->GetScalars());
    if (!rgbaData) {
        return {{"_error", QStringLiteral("Render failed")}};
    }
    rgba = QByteArray(reinterpret_cast<const char *>(rgbaData->GetPointer(0)),
                      static_cast<int>(rgbaData->GetSize()));

    auto w2iDepth = vtkSmartPointer<vtkWindowToImageFilter>::New();
    w2iDepth->SetInput(m_window);
    w2iDepth->SetInputBufferTypeToZBuffer();
    w2iDepth->ReadFrontBufferOff();
    w2iDepth->Update();
    auto depthArray = vtkFloatArray::SafeDownCast(w2iDepth->GetOutput()->GetPointData()->GetScalars());
    if (depthArray) {
        depth = QByteArray(reinterpret_cast<const char *>(depthArray->GetPointer(0)),
                           static_cast<int>(depthArray->GetSize()) * static_cast<int>(sizeof(float)));
    } else {
        depth.clear();
    }
    return {{"status", QStringLiteral("ok")}};
}

QJsonObject SceneManager::renderRawContour(int width, int height, const QJsonObject &params, QByteArray &rgba, QByteArray &depth) {
    ensureScene();
    if (!m_sliceImage) return {{"_error", QStringLiteral("No slice loaded")}};
    if (m_renderer) {
        m_renderer->RemoveAllViewProps();
        if (m_imageActor) m_renderer->AddActor(m_imageActor);
    }
    if (width > 0 && height > 0) {
        m_window->SetSize(width, height);
    }

    std::vector<double> levels;
    const QJsonArray lv = params.value(QStringLiteral("levels")).toArray();
    if (!lv.isEmpty()) {
        for (const auto &v : lv) levels.push_back(v.toDouble());
    } else {
        const int n = 5;
        const double step = (m_range[1] - m_range[0]) / (n + 1);
        for (int i = 1; i <= n; ++i) levels.push_back(m_range[0] + i * step);
    }

    auto contour = vtkSmartPointer<vtkContourFilter>::New();
    contour->SetInputData(m_sliceImage);
    contour->GenerateValues(static_cast<int>(levels.size()),
                            levels.front(), levels.back());
    for (size_t i = 0; i < levels.size(); ++i) {
        contour->SetValue(static_cast<int>(i), levels[i]);
    }

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(contour->GetOutputPort());
    mapper->ScalarVisibilityOff();

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    double color[3] = {0.9, 0.2, 0.2};
    const QJsonArray c = params.value(QStringLiteral("color")).toArray();
    if (c.size() >= 3) {
        color[0] = c[0].toDouble(color[0]);
        color[1] = c[1].toDouble(color[1]);
        color[2] = c[2].toDouble(color[2]);
    }
    actor->GetProperty()->SetColor(color);
    actor->GetProperty()->SetLineWidth(1.5);
    actor->GetProperty()->SetOpacity(1.0);

    m_renderer->AddActor(actor);
    m_renderer->ResetCameraClippingRange();
    m_window->Render();

    auto w2i = vtkSmartPointer<vtkWindowToImageFilter>::New();
    w2i->SetInput(m_window);
    w2i->SetInputBufferTypeToRGBA();
    w2i->ReadFrontBufferOff();
    w2i->Update();

    vtkUnsignedCharArray *rgbaData = vtkUnsignedCharArray::SafeDownCast(w2i->GetOutput()->GetPointData()->GetScalars());
    if (!rgbaData) {
        m_renderer->RemoveActor(actor);
        return {{"_error", QStringLiteral("Render failed")}};
    }
    rgba = QByteArray(reinterpret_cast<const char *>(rgbaData->GetPointer(0)),
                      static_cast<int>(rgbaData->GetSize()));

    auto w2iDepth = vtkSmartPointer<vtkWindowToImageFilter>::New();
    w2iDepth->SetInput(m_window);
    w2iDepth->SetInputBufferTypeToZBuffer();
    w2iDepth->ReadFrontBufferOff();
    w2iDepth->Update();
    auto depthArray = vtkFloatArray::SafeDownCast(w2iDepth->GetOutput()->GetPointData()->GetScalars());
    if (depthArray) {
        depth = QByteArray(reinterpret_cast<const char *>(depthArray->GetPointer(0)),
                           static_cast<int>(depthArray->GetSize()) * static_cast<int>(sizeof(float)));
    } else {
        depth.clear();
    }

    // rimuovi actor per non accumulare
    m_renderer->RemoveActor(actor);
    return {{"status", QStringLiteral("ok")}};
}

QJsonObject SceneManager::renderRaw(int width, int height, QByteArray &rgba, QByteArray &depth) {
    ensureScene();
    if (!m_imageActor) {
        return {{"_error", QStringLiteral("No data loaded")}};
    }
    if (m_renderer) {
        m_renderer->RemoveAllViewProps();
        m_renderer->AddActor(m_imageActor);
    }
    if (width > 0 && height > 0) {
        m_window->SetSize(width, height);
    }
    m_window->Render();

    auto w2i = vtkSmartPointer<vtkWindowToImageFilter>::New();
    w2i->SetInput(m_window);
    w2i->SetInputBufferTypeToRGBA();
    w2i->ReadFrontBufferOff();
    w2i->Update();

    vtkUnsignedCharArray *rgbaData = vtkUnsignedCharArray::SafeDownCast(w2i->GetOutput()->GetPointData()->GetScalars());
    if (!rgbaData) {
        return {{"_error", QStringLiteral("Render failed")}};
    }
    rgba = QByteArray(reinterpret_cast<const char *>(rgbaData->GetPointer(0)),
                      static_cast<int>(rgbaData->GetSize()));

    auto w2iDepth = vtkSmartPointer<vtkWindowToImageFilter>::New();
    w2iDepth->SetInput(m_window);
    w2iDepth->SetInputBufferTypeToZBuffer();
    w2iDepth->ReadFrontBufferOff();
    w2iDepth->Update();
    auto depthArray = vtkFloatArray::SafeDownCast(w2iDepth->GetOutput()->GetPointData()->GetScalars());
    if (depthArray) {
        depth = QByteArray(reinterpret_cast<const char *>(depthArray->GetPointer(0)),
                           static_cast<int>(depthArray->GetSize()) * static_cast<int>(sizeof(float)));
    } else {
        depth.clear();
    }
    return {{"status", QStringLiteral("ok")}};
}
