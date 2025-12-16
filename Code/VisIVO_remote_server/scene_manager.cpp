#include "scene_manager.h"

#include <QBuffer>
#include <QImage>
#include <QDebug>

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageShiftScale.h>
#include <vtkImageMapToColors.h>
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
#include <vtkImageShrink3D.h>
#include <vtkCamera.h>
#include <vtkSmartPointer.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include "../src/luteditor.h"
#include "video_encoder.h"

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
    m_mc = nullptr;
    m_mcNormals = nullptr;
    m_volumeMapper = nullptr;
    m_volumeActor = nullptr;
    m_lastIso = std::numeric_limits<double>::quiet_NaN();
    m_lastLod = 1;
    m_cameraInitialized = false;
    m_loadedSource.clear();
    m_numSlices = 0;
    m_range[0] = 0.0; m_range[1] = 1.0;
    m_currentSlice = 0;
    m_hasWindowLevel = false;
    m_lutScale = "Log";
    m_lutType = "Gray";
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
    // Support anche immagini 2D (NAXIS=2): in quel caso dims[2] può essere 1
    if (dims[0] < 1 || dims[1] < 1) {
        return {{"_error", QStringLiteral("FITS dimensions invalid")}};
    }
    if (dims[2] < 1) dims[2] = 1;
    m_numSlices = dims[2];

    volume->GetScalarRange(m_range);
    // Come in vtkwindow_new case 0: clamp min a 0 se negativo prima di impostare la LUT
    if (m_range[0] < 0.0) {
        m_range[0] = 0.0;
    }
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

    vtkImageData *colored = mapWithLut(scaled);

    if (m_imageActor) { m_imageActor->Delete(); m_imageActor = nullptr; }
    m_imageActor = vtkImageActor::New();
    m_imageActor->SetInputData(colored);
    int sliceExt[6];
    colored->GetExtent(sliceExt);
    m_imageActor->SetDisplayExtent(sliceExt);

    m_renderer->RemoveAllViewProps();
    m_renderer->AddActor(m_imageActor);
    m_renderer->ResetCamera();
    m_loadedSource = path;

    qInfo() << "[SceneManager] Loaded" << path << "dims" << dims[0] << dims[1] << m_numSlices
            << "range" << m_range[0] << m_range[1];

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

QJsonObject SceneManager::setRange(double min, double max) {
    ensureScene();
    if (max <= min) {
        return {{"_error", QStringLiteral("Invalid range")}};
    }
    m_range[0] = min;
    m_range[1] = max;
    if (m_lutScale.compare("Log", Qt::CaseInsensitive) == 0 && m_range[0] <= 0.0) {
        m_range[0] = 1e-6;
    }
    return {{"status", QStringLiteral("ok")}};
}

QJsonObject SceneManager::rotateCamera(double yawDeg, double pitchDeg) {
    ensureScene();
    vtkCamera *cam = m_renderer ? m_renderer->GetActiveCamera() : nullptr;
    if (!cam) return {{"_error", QStringLiteral("No renderer")}};
    cam->Azimuth(yawDeg);
    cam->Elevation(pitchDeg);
    cam->OrthogonalizeViewUp();
    m_renderer->ResetCameraClippingRange();
    m_window->Render();
    return {{"status", QStringLiteral("ok")}};
}

QJsonObject SceneManager::panCamera(double dx, double dy) {
    ensureScene();
    vtkCamera *cam = m_renderer ? m_renderer->GetActiveCamera() : nullptr;
    if (!cam) return {{"_error", QStringLiteral("No renderer")}};

    double fp[3], pos[3], up[3], vpn[3], right[3];
    cam->GetFocalPoint(fp);
    cam->GetPosition(pos);
    cam->GetViewUp(up);
    cam->GetViewPlaneNormal(vpn);
    vtkMath::Cross(vpn, up, right);
    vtkMath::Normalize(right);
    vtkMath::Normalize(up);

    const double dist = vtkMath::Distance2BetweenPoints(fp, pos);
    const double scale = std::sqrt(dist) * 0.5; // fattore empirico

    for (int i = 0; i < 3; ++i) {
        const double shift = (-right[i] * dx + up[i] * dy) * scale;
        fp[i] += shift;
        pos[i] += shift;
    }
    cam->SetFocalPoint(fp);
    cam->SetPosition(pos);
    m_renderer->ResetCameraClippingRange();
    m_window->Render();
    return {{"status", QStringLiteral("ok")}};
}

QJsonObject SceneManager::zoomCamera(double factor) {
    ensureScene();
    vtkCamera *cam = m_renderer ? m_renderer->GetActiveCamera() : nullptr;
    if (!cam) return {{"_error", QStringLiteral("No renderer")}};
    if (factor <= 0.0) factor = 1.0;
    cam->Dolly(factor);
    m_renderer->ResetCameraClippingRange();
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

    vtkImageData *colored = mapWithLut(scaled);

    if (m_imageActor) { m_imageActor->Delete(); m_imageActor = nullptr; }
    m_imageActor = vtkImageActor::New();
    m_imageActor->SetInputData(colored);
    int sliceExt[6];
    colored->GetExtent(sliceExt);
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

QJsonObject SceneManager::setLut(const QJsonObject &params) {
    m_lutType = params.value(QStringLiteral("type")).toString(m_lutType);
    m_lutScale = params.value(QStringLiteral("scale")).toString(m_lutScale);
    if (m_sliceImage) {
        return setSlice(m_currentSlice);
    }
    return {{"status", QStringLiteral("ok")}};
}

static QString encodeImage(const QByteArray &rgba, int width, int height, const QString &qualityTag) {
    QImage img(reinterpret_cast<const uchar *>(rgba.constData()),
               width, height, QImage::Format_RGBA8888);
    QByteArray out;
    QBuffer buf(&out);
    buf.open(QIODevice::WriteOnly);
    const char *fmt = "PNG";
    int quality = -1;
    if (qualityTag.compare(QStringLiteral("interactive"), Qt::CaseInsensitive) == 0) {
        fmt = "JPEG";
        quality = 40; // bitrate-friendly durante pan/zoom
    } else if (qualityTag.compare(QStringLiteral("final"), Qt::CaseInsensitive) == 0) {
        fmt = "JPEG";
        quality = 95; // alta qualità a riposo
    }
    img.save(&buf, fmt, quality);
    return QString::fromLatin1(out.toBase64());
}

QJsonObject SceneManager::renderPng(int width, int height, const QString &quality) {
    QByteArray rgba, depth;
    QJsonObject rawRes = renderRaw(width, height, rgba, depth);
    if (rawRes.contains(QStringLiteral("_error"))) {
        return rawRes;
    }
    const QString b64 = encodeImage(rgba, width, height, quality);
    return {{"status", QStringLiteral("ok")}, {"image", b64}};
}

QJsonObject SceneManager::renderVolumePng(int width, int height, const QJsonObject &volumeParams, const QString &quality) {
    QByteArray rgba, depth;
    QJsonObject res = renderRawVolume(width, height, volumeParams, rgba, depth);
    if (res.contains(QStringLiteral("_error"))) return res;
    const QString b64 = encodeImage(rgba, width, height, quality);
    return {{"status", QStringLiteral("ok")}, {"image", b64}};
}

QJsonObject SceneManager::renderContourPng(int width, int height, const QJsonObject &params, const QString &quality) {
    QByteArray rgba, depth;
    QJsonObject res = renderRawContour(width, height, params, rgba, depth);
    if (res.contains(QStringLiteral("_error"))) return res;
    const QString b64 = encodeImage(rgba, width, height, quality);
    return {{"status", QStringLiteral("ok")}, {"image", b64}};
}

QJsonObject SceneManager::renderH264(int width, int height, const QString &quality) {
    QByteArray rgba, depth;
    QJsonObject rawRes = renderRaw(width, height, rgba, depth);
    if (rawRes.contains(QStringLiteral("_error"))) {
        return rawRes;
    }
    QString err;
    QByteArray nal = VideoEncoder::encodeH264(rgba, width, height, quality, err);
    if (nal.isEmpty()) {
        return {{"_error", err.isEmpty() ? QStringLiteral("encode failed") : err}};
    }
    const QString b64 = QString::fromLatin1(nal.toBase64());
    return {{"status", QStringLiteral("ok")},
            {"codec", QStringLiteral("h264")},
            {"data", b64},
            {"width", width},
            {"height", height}};
}

QJsonObject SceneManager::renderH264Volume(int width, int height, const QJsonObject &volumeParams, const QString &quality) {
    QByteArray rgba, depth;
    QJsonObject rawRes = renderRawVolume(width, height, volumeParams, rgba, depth);
    if (rawRes.contains(QStringLiteral("_error"))) return rawRes;
    QString err;
    QByteArray nal = VideoEncoder::encodeH264(rgba, width, height, quality, err);
    if (nal.isEmpty()) {
        return {{"_error", err.isEmpty() ? QStringLiteral("encode failed") : err}};
    }
    const QString b64 = QString::fromLatin1(nal.toBase64());
    return {{"status", QStringLiteral("ok")},
            {"codec", QStringLiteral("h264")},
            {"data", b64},
            {"width", width},
            {"height", height}};
}

QJsonObject SceneManager::renderH264Contour(int width, int height, const QJsonObject &params, const QString &quality) {
    QByteArray rgba, depth;
    QJsonObject rawRes = renderRawContour(width, height, params, rgba, depth);
    if (rawRes.contains(QStringLiteral("_error"))) return rawRes;
    QString err;
    QByteArray nal = VideoEncoder::encodeH264(rgba, width, height, quality, err);
    if (nal.isEmpty()) {
        return {{"_error", err.isEmpty() ? QStringLiteral("encode failed") : err}};
    }
    const QString b64 = QString::fromLatin1(nal.toBase64());
    return {{"status", QStringLiteral("ok")},
            {"codec", QStringLiteral("h264")},
            {"data", b64},
            {"width", width},
            {"height", height}};
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
    int lod = volumeParams.value(QStringLiteral("lod")).toInt(1);
    lod = std::max(1, lod);
    // Clamp iso dentro il range valido
    if (iso <= m_range[0] || iso >= m_range[1]) {
        iso = defaultIso;
    }

    // Costruisci la pipeline solo la prima volta o se l'iso cambia
    bool rebuildIso = !m_mc || std::isnan(m_lastIso) || std::abs(m_lastIso - iso) > 1e-6 || lod != m_lastLod;
    if (rebuildIso) {
        vtkSmartPointer<vtkImageData> inputVol = m_volumeImage;
        vtkSmartPointer<vtkImageShrink3D> shrink;
        if (lod > 1) {
            shrink = vtkSmartPointer<vtkImageShrink3D>::New();
            shrink->SetInputData(m_volumeImage);
            shrink->SetShrinkFactors(lod, lod, lod);
            shrink->AveragingOn();
            shrink->Update();
            inputVol = shrink->GetOutput();
        }
        m_mc = vtkSmartPointer<vtkMarchingCubes>::New();
        m_mc->SetInputData(inputVol);
        m_mc->ComputeNormalsOn();
        m_mcNormals = vtkSmartPointer<vtkPolyDataNormals>::New();
        m_mcNormals->SetInputConnection(m_mc->GetOutputPort());
        m_mcNormals->SplittingOff();
        m_volumeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        m_volumeMapper->SetInputConnection(m_mcNormals->GetOutputPort());
        m_volumeMapper->ScalarVisibilityOff();
        m_volumeActor = vtkSmartPointer<vtkActor>::New();
        m_volumeActor->SetMapper(m_volumeMapper);
        m_renderer->RemoveAllViewProps();
        m_renderer->AddActor(m_volumeActor);
        m_lastIso = iso;
        m_lastLod = lod;
        m_cameraInitialized = false; // reset camera on first build
    }
    if (m_mc) {
        if (rebuildIso) {
            m_mc->SetValue(0, iso);
            m_mc->Modified();
            m_mc->Update();
            m_mcNormals->Update();
        }
    }

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
    if (m_volumeActor) {
        m_volumeActor->GetProperty()->SetColor(color);
        m_volumeActor->GetProperty()->SetOpacity(opacity);
    }

    if (!m_cameraInitialized) {
        m_renderer->ResetCamera();
        m_cameraInitialized = true;
    }
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

vtkImageData *SceneManager::mapWithLut(vtkImageData *src) {
    if (!src) return nullptr;
    auto lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->SetRange(m_range);
    if (m_lutScale.compare("Log", Qt::CaseInsensitive) == 0) {
        lut->SetScaleToLog10();
    } else {
        lut->SetScaleToLinear();
    }
    lut->SetNumberOfTableValues(256);
    SelectLookTable(QString::fromStdString(m_lutType.toStdString()), lut);
    lut->Build();
    auto mapper = vtkSmartPointer<vtkImageMapToColors>::New();
    mapper->SetInputData(src);
    mapper->SetLookupTable(lut);
    mapper->SetOutputFormatToRGBA();
    mapper->Update();
    vtkImageData *out = mapper->GetOutput();
    out->Register(nullptr);
    return out;
}
