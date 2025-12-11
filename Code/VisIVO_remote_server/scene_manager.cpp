#include "scene_manager.h"

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageShiftScale.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkUnsignedCharArray.h>
#include <vtkCamera.h>

#include "../src/vtkfitsreader2.h"

SceneManager::SceneManager()
    : m_window(nullptr),
      m_renderer(nullptr),
      m_imageActor(nullptr),
      m_sliceImage(nullptr),
      m_shift(nullptr) {
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
    }
    return true;
}

void SceneManager::resetScene() {
    if (m_renderer) {
        m_renderer->RemoveAllViewProps();
    }
    if (m_imageActor) { m_imageActor->Delete(); m_imageActor = nullptr; }
    if (m_sliceImage) { m_sliceImage->Delete(); m_sliceImage = nullptr; }
    if (m_shift) { m_shift->Delete(); m_shift = nullptr; }
    m_loadedSource.clear();
    m_numSlices = 0;
    m_range[0] = 0.0; m_range[1] = 1.0;
}

QJsonObject SceneManager::loadFits(const QString &path) {
    ensureScene();
    resetScene();

    vtkSmartPointer<vtkFitsReader2> reader = vtkSmartPointer<vtkFitsReader2>::New();
    reader->SetFileName(path.toStdString().c_str());
    reader->SliceModeOn();

    // Query number of slices
    reader->SetSlice(0);
    reader->UpdateInformation();
    m_numSlices = reader->GetNumberOfSlices();
    const int k = m_numSlices > 0 ? m_numSlices / 2 : 0;
    reader->SetSlice(k);
    reader->Update();

    vtkImageData *slice = reader->GetOutput();
    if (!slice) {
        return {{"_error", QStringLiteral("FITS load failed")}};
    }

    int dims[3];
    slice->GetDimensions(dims);
    if (dims[0] < 1 || dims[1] < 1) {
        return {{"_error", QStringLiteral("FITS dimensions invalid")}};
    }

    slice->GetScalarRange(m_range);
    const double delta = (m_range[1] - m_range[0]);

    vtkImageShiftScale *shift = vtkImageShiftScale::New();
    shift->SetInputData(slice);
    shift->SetShift(-m_range[0]);
    shift->SetScale(delta > 0 ? 255.0 / delta : 1.0);
    shift->SetOutputScalarTypeToUnsignedChar();
    shift->ClampOverflowOn();
    shift->Update();

    vtkImageData *scaled = shift->GetOutput();
    scaled->Register(nullptr);
    shift->Register(nullptr);

    m_sliceImage = scaled;
    m_shift = shift;

    m_imageActor = vtkImageActor::New();
    m_imageActor->SetInputData(scaled);
    int sliceExt[6];
    scaled->GetExtent(sliceExt);
    m_imageActor->SetDisplayExtent(sliceExt);

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

QJsonObject SceneManager::renderPng(int width, int height) {
    ensureScene();
    if (!m_imageActor) {
        return {{"_error", QStringLiteral("No data loaded")}};
    }
    if (width > 0 && height > 0) {
        m_window->SetSize(width, height);
    }
    m_window->Render();

    vtkSmartPointer<vtkWindowToImageFilter> w2i = vtkSmartPointer<vtkWindowToImageFilter>::New();
    w2i->SetInput(m_window);
    w2i->Update();

    vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
    writer->SetWriteToMemory(true);
    writer->SetInputConnection(w2i->GetOutputPort());
    writer->Write();
    vtkUnsignedCharArray *data = writer->GetResult();
    if (!data) {
        return {{"_error", QStringLiteral("Render failed")}};
    }
    QByteArray png(reinterpret_cast<const char *>(data->GetPointer(0)),
                   static_cast<int>(data->GetSize()));
    const QString b64 = QString::fromLatin1(png.toBase64());
    return {{"status", QStringLiteral("ok")}, {"image", b64}};
}
