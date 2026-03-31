#ifndef vtkWindowImage_h
#define vtkWindowImage_h

#include "ImageLayerLoadTask.h"

#include <vtkNew.h>

#include <QCloseEvent>
#include <QElapsedTimer>
#include <QFutureWatcher>
#include <QMainWindow>
#include <QPointer>
#include <QTimer>

#include <array>
#include <memory>

class ImageLayerController;
class ImageLayerImportService;
class LayerListModel;
class LUTCustomizerDialog;
class ProfileWidget;
class AstroUtils;
class vtkCoordinate;
class vtkImageStack;
class vtkLegendScaleActorWCS;
class vtkScalarBarActor;

QT_BEGIN_NAMESPACE
namespace Ui {
class vtkWindowImage;
}
QT_END_NAMESPACE

class vtkWindowImage : public QMainWindow
{
    Q_OBJECT

public:
    explicit vtkWindowImage(const QString &filepath, QWidget *parent = nullptr);
    vtkWindowImage(const QString &filepath, const QString &backendUrl, const QString &datasetId,
                   QWidget *parent = nullptr);
    ~vtkWindowImage();
    void closeEvent(QCloseEvent *event) override;

private slots:
    void showLUTCustomizer();
    void updateLUTCustomizer();

    void addLocalFile();

    void vtkRender();
    void changeLegendWCS();
    void changeCurrentColorMap();
    void changeCurrentColorScale();
    void changeCurrentLayerOpacity();
    void showCurrentLayerSettings();

    // Interactors
    void setInteractorStyleImage();
    void setInteractorStyleProfile();

private:
    Ui::vtkWindowImage *ui;
    const QString filepath;
    const bool isRemoteMode;
    const QString remoteBackendUrl;
    const QString remoteDatasetId;
    std::unique_ptr<AstroUtils> astro;
    QPointer<LUTCustomizerDialog> lutCustomizer;
    QPointer<ProfileWidget> profileWidget;
    QFutureWatcher<ImageLayerLoadResult> layerLoadWatcher;
    QFutureWatcher<ImageLayerLoadResult> remoteImageWatcher;
    QTimer statusMessageClearTimer;
    QElapsedTimer statusMessageElapsed;
    int statusMessageMinDurationMs{ 0 };
    bool persistentStatusActive{ false };
    std::unique_ptr<ImageLayerImportService> importService;

    // Renderer
    vtkNew<vtkLegendScaleActorWCS> legendWCS;
    vtkNew<vtkScalarBarActor> colorbar;
    vtkNew<vtkCoordinate> coordinate;
    void setupRenderer();
    void mouseCallback();

    // Stack
    vtkNew<vtkImageStack> stack;
    LayerListModel *layers;
    std::unique_ptr<ImageLayerController> layerController;
    void applyLoadedLayer(const ImageLayerLoadResult &result);
    void applyRemoteMasterLayer(const ImageLayerLoadResult &result);
    bool isBusy() const;
    void showPersistentStatusMessage(const QString &text, int minDurationMs = 400);
    void clearPersistentStatusMessage();
    void setLayerImportEnabled(bool enabled);
    int currentLayerIndex() const;
    void addLayerImage(const std::string &filepath);
};

#endif
