#ifndef RemoteMomentWindow_h
#define RemoteMomentWindow_h

#include "MomentMapComputeTask.h"

#include <vtkNew.h>

#include <QElapsedTimer>
#include <QFutureWatcher>
#include <QMainWindow>
#include <QPointer>
#include <QTimer>

class QVTKOpenGLNativeWidget;
class vtkGenericOpenGLRenderWindow;
class vtkLookupTable;
class vtkRenderer;
class vtkScalarBarActor;
class vtkTrivialProducer;

class RemoteMomentWindow : public QMainWindow
{
    Q_OBJECT

public:
    RemoteMomentWindow(const QString &backendUrl, const QString &datasetId, const QString &datasetPath,
                       QWidget *parent = nullptr);
    ~RemoteMomentWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void requestMoment(int order);
    void showPersistentStatusMessage(const QString &text, int minDurationMs = 400);
    void clearPersistentStatusMessage();

    QString backendUrl;
    QString datasetId;
    QString datasetPath;
    QPointer<QVTKOpenGLNativeWidget> vtkWidget;
    QFutureWatcher<MomentMapComputeResult> watcher;
    QTimer statusMessageClearTimer;
    QElapsedTimer statusMessageElapsed;
    int statusMessageMinDurationMs{ 0 };
    bool persistentStatusActive{ false };
    int currentRequestId{ 0 };

    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkNew<vtkRenderer> renderer;
    vtkNew<vtkTrivialProducer> imageSource;
    vtkNew<vtkLookupTable> lookupTable;
    vtkNew<vtkScalarBarActor> colorbar;
};

#endif
