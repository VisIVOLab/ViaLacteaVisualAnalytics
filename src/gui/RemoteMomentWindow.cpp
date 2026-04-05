#include "RemoteMomentWindow.h"

#include "ColorMaps.h"

#include <QVTKOpenGLNativeWidget.h>
#include <vtkCamera.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkInteractorStyleImage.h>
#include <vtkLookupTable.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkTrivialProducer.h>

#include <QCloseEvent>
#include <QElapsedTimer>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QtConcurrentRun>

using namespace Qt::StringLiterals;

namespace {
vtkSmartPointer<vtkImageData> createPlaceholderImageData()
{
    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
    image->SetExtent(0, 0, 0, 0, 0, 0);
    image->AllocateScalars(VTK_FLOAT, 1);
    image->SetScalarComponentFromFloat(0, 0, 0, 0, 0.f);
    return image;
}
}

RemoteMomentWindow::RemoteMomentWindow(const QString &backendUrl, const QString &datasetId,
                                       const QString &datasetPath, const QString &sessionId,
                                       QWidget *parent)
    : QMainWindow(parent), backendUrl(backendUrl), datasetId(datasetId), datasetPath(datasetPath),
      sessionId(sessionId)
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowTitle(u"%1 [remote moment]"_s.arg(datasetPath));

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    this->vtkWidget = new QVTKOpenGLNativeWidget(central);
    layout->addWidget(this->vtkWidget);
    this->setCentralWidget(central);

    this->renderWindow->AddRenderer(this->renderer);
    this->renderer->SetBackground(0.21, 0.23, 0.25);
    this->renderer->GetActiveCamera()->ParallelProjectionOn();
    this->vtkWidget->setRenderWindow(this->renderWindow);
    this->vtkWidget->setEnableTouchEventProcessing(false);

    vtkNew<vtkInteractorStyleImage> style;
    this->renderWindow->GetInteractor()->SetInteractorStyle(style);

    this->lookupTable->SetNanColor(1., 1., 1., 1.);
    ColorMaps::SetColorMap(this->lookupTable);

    auto *initialImage = vtkImageData::SafeDownCast(this->imageSource->GetOutputDataObject(0));
    if (initialImage) {
        this->imageColors->SetInputData(initialImage);
    } else {
        this->imageColors->SetInputData(createPlaceholderImageData());
    }
    this->imageColors->SetLookupTable(this->lookupTable);
    vtkNew<vtkImageSliceMapper> mapper;
    mapper->SetInputConnection(this->imageColors->GetOutputPort());
    vtkNew<vtkImageSlice> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetInterpolationTypeToNearest();
    this->renderer->AddViewProp(actor);

    this->colorbar->SetLookupTable(this->lookupTable);
    this->colorbar->SetMaximumWidthInPixels(100);
    this->colorbar->SetPosition(0.9, 0.1);
    this->renderer->AddViewProp(this->colorbar);

    auto *momentMenu = this->menuBar()->addMenu(u"Moment"_s);
    for (const int order : { 0, 1, 2, 8, 10 }) {
        auto *action = momentMenu->addAction(u"Moment %1"_s.arg(order));
        QObject::connect(action, &QAction::triggered, this, [this, order]() { this->requestMoment(order); });
    }

    this->statusMessageClearTimer.setSingleShot(true);
    QObject::connect(&this->statusMessageClearTimer, &QTimer::timeout, this, [this]() {
        this->persistentStatusActive = false;
        this->statusBar()->clearMessage();
    });
    QObject::connect(&this->watcher, &QFutureWatcher<MomentMapComputeResult>::finished, this, [this]() {
        const int requestId = this->watcher.property("requestId").toInt();
        if (requestId != this->currentRequestId) {
            return;
        }

        const auto result = this->watcher.result();
        if (!result.valid || !result.imageData) {
            this->persistentStatusActive = false;
            this->statusMessageClearTimer.stop();
            this->statusBar()->showMessage(result.errorMessage.isEmpty()
                                                   ? u"Could not compute remote moment."_s
                                                   : result.errorMessage);
            return;
        }

        this->imageSource->SetOutput(result.imageData);
        auto *img = vtkImageData::SafeDownCast(this->imageSource->GetOutputDataObject(0));
        if (img) {
            this->imageColors->SetInputData(img);
        } else {
            qWarning() << "[vtk] Expected vtkImageData but got null or wrong type";
            return;
        }
        this->lookupTable->SetTableRange(result.imageRange[0], result.imageRange[1]);
        this->renderer->ResetCamera();
        this->renderWindow->Render();
        this->clearPersistentStatusMessage();
    });

    this->requestMoment(0);
}

RemoteMomentWindow::~RemoteMomentWindow() = default;

void RemoteMomentWindow::closeEvent(QCloseEvent *event)
{
    if (this->watcher.isRunning()) {
        this->showPersistentStatusMessage(u"Computing moment..."_s);
        event->ignore();
        return;
    }

    QMainWindow::closeEvent(event);
}

void RemoteMomentWindow::requestMoment(int order)
{
    if (this->watcher.isRunning()) {
        return;
    }

    const int requestId = ++this->currentRequestId;
    this->watcher.setProperty("requestId", requestId);
    this->showPersistentStatusMessage(u"Computing moment..."_s);
    this->watcher.setFuture(QtConcurrent::run(
            &computeMomentMap,
            MomentMapComputeRequest { {}, this->datasetId, this->backendUrl, this->sessionId, order }));
}

void RemoteMomentWindow::showPersistentStatusMessage(const QString &text, int minDurationMs)
{
    this->statusMessageClearTimer.stop();
    this->persistentStatusActive = true;
    this->statusMessageMinDurationMs = minDurationMs;
    this->statusMessageElapsed.restart();
    this->statusBar()->showMessage(text);
}

void RemoteMomentWindow::clearPersistentStatusMessage()
{
    if (!this->persistentStatusActive) {
        this->statusBar()->clearMessage();
        return;
    }

    const int remaining = this->statusMessageMinDurationMs - this->statusMessageElapsed.elapsed();
    if (remaining <= 0) {
        this->persistentStatusActive = false;
        this->statusBar()->clearMessage();
        return;
    }

    this->statusMessageClearTimer.start(remaining);
}
