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
                                       const QString &backendToken,
                                       QWidget *parent)
    : QMainWindow(parent), backendUrl(backendUrl), datasetId(datasetId), datasetPath(datasetPath),
      sessionId(sessionId), backendToken(backendToken)
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
    const QList<QPair<int, QString>> momentOrders = {
        { 0, u"Moment 0 (Integrated intensity)"_s },
        { 1, u"Moment 1 (Mean velocity)"_s },
        { 2, u"Moment 2 (Variance – NOT dispersion)"_s },
        { 8, u"Moment 8 (Peak value)"_s },
        { 10, u"Moment 10 (Minimum value)"_s },
    };
    for (const auto &[order, label] : momentOrders) {
        auto *action = momentMenu->addAction(label);
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

        // Update title and colorbar to show the moment order and physical units.
        const int momentOrder = this->watcher.property("momentOrder").toInt();
        const QString unitSuffix = result.momentUnit.isEmpty()
                ? QString()
                : u" [%1]"_s.arg(result.momentUnit);
        this->setWindowTitle(u"%1  M%2%3"_s.arg(this->datasetPath).arg(momentOrder).arg(unitSuffix));
        const QByteArray colorbarTitle =
                (result.momentUnit.isEmpty() ? u"M%1"_s.arg(momentOrder)
                                             : u"M%1 (%2)"_s.arg(momentOrder).arg(result.momentUnit))
                        .toUtf8();
        this->colorbar->SetTitle(colorbarTitle.constData());

        this->renderer->ResetCamera();
        this->renderWindow->Render();
        this->clearPersistentStatusMessage();

        // Show a non-blocking WCS warning in the status bar when the server had to sanitize
        // or degrade the WCS (e.g. dropped PC matrix, fell back to CDELT-only).
        if (result.wcsStatus != u"ok"_s) {
            const QString warnText = result.wcsWarningMessage.isEmpty()
                    ? u"WCS: %1"_s.arg(result.wcsStatus)
                    : u"WCS (%1): %2"_s.arg(result.wcsStatus, result.wcsWarningMessage);
            this->statusBar()->showMessage(warnText, 8000);
        }
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
    this->watcher.setProperty("momentOrder", order);
    this->showPersistentStatusMessage(u"Computing moment..."_s);
    this->watcher.setFuture(QtConcurrent::run(
            &computeMomentMap,
            MomentMapComputeRequest { {}, this->datasetId, this->backendUrl, this->sessionId,
                                      this->backendToken, order }));
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
