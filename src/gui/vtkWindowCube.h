#ifndef vtkWindowCube_H
#define vtkWindowCube_H

#include "AstroUtils.h"
#include "CubeOpenPreviewTask.h"
#include "MomentMapComputeTask.h"

#include <vtkSmartPointer.h>
#include <vtkNew.h>

#include <QFutureWatcher>
#include <QCloseEvent>
#include <QElapsedTimer>
#include <QHash>
#include <QMainWindow>
#include <QPointer>
#include <QSet>
#include <QTimer>

#include <array>
#include <memory>

class CubeViewController;
class QLabel;
class LUTCustomizerDialog;
class ProfileWidget;
class vtkActor;
class vtkColorTransferFunction;
class vtkCoordinate;
class vtkExtractVOI;
class vtkFITSReader;
class vtkFlyingEdges2D;
class vtkFlyingEdges3D;
class vtkGenericOpenGLRenderWindow;
class vtkImageData;
class vtkImageMapToColors;
class vtkImageReslice;
class vtkLegendScaleActorWCS;
class vtkLookupTable;
class vtkMomentMapFilter;
class vtkOrientationMarkerWidget;
class vtkPlaneSource;
class vtkPiecewiseFunction;
class vtkPolyData;
class vtkTrivialProducer;
class vtkVolume;

QT_BEGIN_NAMESPACE
namespace Ui {
class vtkWindowCube;
}
QT_END_NAMESPACE

struct AsyncIsosurfaceResult
{
    vtkSmartPointer<vtkPolyData> mesh;
    QString errorMessage;
    int requestId{ 0 };
};

struct RemoteCubePreviewResult
{
    bool valid{ false };
    QString errorMessage;
    vtkSmartPointer<vtkImageData> cubeImageData;
    std::array<double, 2> cubeRange{ 0., 0. };
    std::array<int, 6> dataExtent{ 0, -1, 0, -1, 0, -1 };
};

struct RemoteCubeSliceResult
{
    bool valid{ false };
    QString errorMessage;
    vtkSmartPointer<vtkImageData> imageData;
    std::array<double, 2> imageRange{ 0., 0. };
    int index{ 0 };
};

struct RemoteCubeSubvolumeResult
{
    bool valid{ false };
    QString errorMessage;
    vtkSmartPointer<vtkImageData> cubeImageData;
    std::array<double, 2> cubeRange{ 0., 0. };
    std::array<int, 6> dataExtent{ 0, -1, 0, -1, 0, -1 };
    double cubeMean{ 0. };
    double cubeRms{ 0. };
};

class vtkWindowCube : public QMainWindow
{
    Q_OBJECT

public:
    vtkWindowCube(const QString &filepath, QWidget *parent = nullptr);
    vtkWindowCube(const QString &filepath, const QString &backendUrl, const QString &datasetId,
                  int remoteWidth, int remoteHeight, int remoteDepth,
                  const std::array<double, 3> &remoteSpacing,
                  const std::array<double, 3> &remoteOrigin, QWidget *parent = nullptr);
    ~vtkWindowCube() override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void resetCameraFront();
    void resetCameraBack();
    void resetCameraTop();
    void resetCameraRight();
    void resetCameraBottom();
    void resetCameraLeft();

    void renderImage();
    void changeImageRenderer();
    void syncSlicesLUT();

    void changeCubeRender();
    void changeCubeColor();

    void thresholdSliderChanged(int action);
    void thresholdLineChanged();

    void sliceSliderChanged(int action);
    void sliceSpinChanged(int value);

    void changeLegendWCS();

    void showLUTCustomizer();
    void updateLUTCustomizer();

    void setInteractorStyleImage();
    void setInteractorStyleProfile();

private:
    enum class RemoteCubeDisplayState
    {
        Preview,
        LoadingFullResolution,
        FullResolution,
    };

    struct MomentMapApplyResult
    {
        vtkSmartPointer<vtkImageData> imageData;
        std::array<double, 2> imageRange;
    };

    Ui::vtkWindowCube *ui;
    const QString filepath;
    const bool isRemoteMode;
    const QString remoteBackendUrl;
    const QString remoteDatasetId;
    const int remoteDatasetWidth;
    const int remoteDatasetHeight;
    const int remoteDatasetDepth;
    const std::array<double, 3> remoteDatasetSpacing;
    const std::array<double, 3> remoteDatasetOrigin;
    std::unique_ptr<AstroUtils> astro;

    QPointer<LUTCustomizerDialog> lutCustomizer;
    QPointer<ProfileWidget> profileWidget;
    QPointer<QLabel> cubeOpenStateLabel;
    QFutureWatcher<CubeOpenStageResult> cubeOpenWatcher;
    QFutureWatcher<RemoteCubePreviewResult> remotePreviewWatcher;
    QFutureWatcher<RemoteCubeSubvolumeResult> remoteHighResCubeWatcher;
    QFutureWatcher<MomentMapComputeResult> momentComputeWatcher;
    QFutureWatcher<AsyncIsosurfaceResult> isosurfaceWatcher;
    int currentRemoteSliceRequestId{ 0 };
    int currentRequestedRemoteSliceIndex{ 0 };
    int activeRemoteSliceRequests{ 0 };
    int activeRemoteIsosurfaceRequests{ 0 };
    QTimer isosurfaceDebounceTimer;
    int currentMomentRequestId{ 0 };
    int currentIsosurfaceRequestId{ 0 };
    int currentFullCubeGeneration{ 0 };
    int lastIsosurfacePrewarmGeneration{ 0 };
    QTimer statusMessageClearTimer;
    QElapsedTimer statusMessageElapsed;
    int statusMessageMinDurationMs{ 0 };
    bool persistentStatusActive{ false };
    bool usingHighResCube{ false };
    RemoteCubeDisplayState remoteCubeDisplayState{ RemoteCubeDisplayState::Preview };

    vtkNew<vtkFITSReader> reader;
    float lowerBound;
    float upperBound;

    // Render Windows
    vtkNew<vtkGenericOpenGLRenderWindow> sliceWin;
    vtkNew<vtkGenericOpenGLRenderWindow> momentWin;
    vtkNew<vtkOrientationMarkerWidget> axesWidget;
    vtkNew<vtkCoordinate> coordinate;
    double initialCameraPosition[3];
    double initialCameraFocalPoint[3];
    std::unique_ptr<CubeViewController> viewController;
    void setupCubeRenderer();
    void setupSliceRenderer();
    void setupMomentRenderer();
    void resetCubeCamera();
    void setCameraAzimuth(double az);
    void setCameraElevation(double el);
    void mouseCallback();
    bool viewingIsosurface() const;
    bool viewingSlice() const;

    // Cube
    vtkNew<vtkTrivialProducer> cubeDisplaySource;
    vtkNew<vtkFlyingEdges3D> isosurfaceFilter;
    vtkNew<vtkActor> isosurface;
    vtkSmartPointer<vtkActor> currentIsosurfaceActor;
    vtkNew<vtkVolume> volume;
    vtkNew<vtkColorTransferFunction> volumeColorTransferFunction;
    vtkNew<vtkPiecewiseFunction> volumeOpacity;
    vtkNew<vtkPlaneSource> remoteCuttingPlaneSource;
    vtkNew<vtkActor> remoteCuttingPlaneActor;
    bool remoteIsosurfaceReady{ false };
    void applyCubeOpenResult(const CubeOpenStageResult &result);
    void applyIsosurfaceResult(const AsyncIsosurfaceResult &result);
    void startAsyncIsosurface(double isoValue);
    void scheduleIsosurfaceRecompute();
    void scheduleIsosurfacePrewarm();
    void setCubeRenderModeLocally(bool isosurfaceMode);
    void updateCube();
    std::array<int, 6> computeVisibleROI() const;
    bool requestHighResCube();
    void updateRemoteCuttingPlane(int sliceIndex);
    int remoteSliceCount() const;
    int clampRemoteSliceIndex(int sliceIndex) const;
    double remoteSliceCoordinate(int sliceIndex) const;

    // Slice
    vtkNew<vtkImageReslice> slice;
    vtkNew<vtkImageMapToColors> sliceColors;
    vtkNew<vtkLookupTable> lutSlice;
    vtkNew<vtkExtractVOI> sliceOnCube;
    vtkNew<vtkLookupTable> lutSliceOnCube;
    vtkNew<vtkTrivialProducer> remoteSliceDisplaySource;
    vtkNew<vtkLegendScaleActorWCS> legendSlice;
    QHash<QString, RemoteCubeSliceResult> remoteSliceCache;
    QList<QString> remoteSliceCacheLru;
    QSet<QString> remoteSliceFetchesInFlight;
    static constexpr int remoteSliceCacheCapacity = 8;
    void updateSlice();
    void requestRemoteSlice(int sliceIndex);
    void applyRemoteSliceResult(const RemoteCubeSliceResult &result);
    QString remoteSliceCacheKey(int sliceIndex) const;
    void touchRemoteSliceCacheKey(const QString &key);
    void cacheRemoteSliceResult(const RemoteCubeSliceResult &result);
    bool tryApplyCachedRemoteSlice(int sliceIndex);
    void startRemoteSliceFetch(int sliceIndex, bool isPrefetch, int requestId = 0);
    void prefetchNeighborRemoteSlices(int sliceIndex);

    // Contours
    int level;
    vtkNew<vtkFlyingEdges2D> contours;
    vtkNew<vtkActor> contoursActor;
    void updateContours();
    void updateContoursVisibility();

    // Moment
    vtkNew<vtkMomentMapFilter> moment;
    vtkNew<vtkTrivialProducer> momentDisplaySource;
    vtkNew<vtkImageMapToColors> momentColors;
    vtkNew<vtkLookupTable> lutMoment;
    vtkNew<vtkLegendScaleActorWCS> legendMoment;
    void applyMomentMapResult(const MomentMapApplyResult &result);
    bool isBusy() const;
    void setRemoteCubeDisplayState(RemoteCubeDisplayState state);
    void showPersistentStatusMessage(const QString &text, int minDurationMs = 400);
    void clearPersistentStatusMessage();
    void setCubeOpenStateLabel(const QString &text);
    void setCubeOpenActionsEnabled(bool enabled);
    void setMomentActionsEnabled(bool enabled);
    void setMomentOrder(int order);
};
#endif
