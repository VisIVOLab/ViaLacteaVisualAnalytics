#ifndef vtkWindowImage_h
#define vtkWindowImage_h

#include "ImageLayerLoadTask.h"

#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <QCloseEvent>
#include <QElapsedTimer>
#include <QFutureWatcher>
#include <QMainWindow>
#include <QPointer>
#include <QStringList>
#include <QTimer>

#include <array>
#include <limits>
#include <memory>
#include <vector>

class ImageLayerController;
class ImageLayerImportService;
class LayerListModel;
class LUTCustomizerDialog;
class ProfileWidget;
class AstroUtils;
class QAction;
class QCheckBox;
class QLabel;
class vtkActor;
class vtkAxisActor2D;
class vtkCoordinate;
class vtkImageStack;
class vtkLegendScaleActorWCS;
class vtkLookupTable;
class vtkCellArray;
class vtkContourTriangulator;
class vtkLineSource;
class vtkPoints;
class vtkPolyData;
class vtkRegularPolygonSource;
class vtkRenderer;
class vtkScalarBarActor;
class vtkTextActor;

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
                   const std::array<QString, 3> &remoteCtype,
                   const std::array<QString, 3> &remoteCunit,
                   const std::array<double, 3> &remoteCrval,
                   const std::array<double, 3> &remoteCrpix,
                   const std::array<double, 3> &remoteCdelt, const QString &remoteDegenerateAxesSummary,
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
    void setInteractorStyleRegion();
    void toggleProbeFreeze();
    void finishRegionInteraction();
    void setProbeModeActive(bool active);
    void beginImageInteraction();
    void endImageInteraction();

public:
    enum class RegionMode
    {
        None,
        Box,
        Circle,
        Polygon,
        Annulus,
    };

private:
    Ui::vtkWindowImage *ui;
    const QString filepath;
    const bool isRemoteMode;
    const QString remoteBackendUrl;
    const QString remoteDatasetId;
    const std::array<QString, 3> remoteDatasetCtype;
    const std::array<QString, 3> remoteDatasetCunit;
    const std::array<double, 3> remoteDatasetCrval;
    const std::array<double, 3> remoteDatasetCrpix;
    const std::array<double, 3> remoteDatasetCdelt;
    const QString remoteDegenerateAxesSummary;
    bool remoteDisplayingPreview{ false };
    std::array<int, 2> remoteFullImageDims{ 0, 0 };
    bool transitioningToFull{ false };
    bool suspendLutEditorUpdates{ false };
    bool displayStateInitialized{ false };
    bool userAdjustedDisplayState{ false };
    int remoteLoadGeneration{ 0 };
    std::unique_ptr<AstroUtils> astro;
    QPointer<LUTCustomizerDialog> lutCustomizer;
    QPointer<ProfileWidget> profileWidget;
    QPointer<QCheckBox> wcsAxesCheck;
    QPointer<QLabel> hoverReadoutLabel;
    QPointer<QLabel> dataStateLabel;
    QPointer<QLabel> sanityLabel;
    QPointer<QAction> actionWcsSexagesimal;
    QPointer<QAction> actionWcsDecimal;
    QPointer<QAction> actionExtractSpectrum;
    QPointer<QAction> actionBoxRegion;
    QPointer<QAction> actionCircleRegion;
    QPointer<QAction> actionPolygonRegion;
    QPointer<QAction> actionAnnulusRegion;
    QPointer<QAction> actionLoadCatalogueOverlay;
    QPointer<QAction> actionShowCatalogueOverlay;
    QPointer<QAction> actionShowCatalogueLabels;
    QPointer<QAction> actionClearCatalogueOverlay;
    QFutureWatcher<ImageLayerLoadResult> layerLoadWatcher;
    QFutureWatcher<ImageLayerLoadResult> remoteImageWatcher;
    QFutureWatcher<ImageLayerLoadResult> remoteFullImageWatcher;
    QTimer statusMessageClearTimer;
    QElapsedTimer statusMessageElapsed;
    int statusMessageMinDurationMs{ 0 };
    bool persistentStatusActive{ false };
    std::unique_ptr<ImageLayerImportService> importService;

    // Renderer
    vtkNew<vtkLegendScaleActorWCS> legendWCS;
    vtkNew<vtkAxisActor2D> overlayXAxis;
    vtkNew<vtkAxisActor2D> overlayYAxis;
    vtkNew<vtkTextActor> overlayXTitleActor;
    vtkNew<vtkTextActor> overlayYTitleActor;
    std::vector<vtkSmartPointer<vtkTextActor>> overlayXTickActors;
    std::vector<vtkSmartPointer<vtkTextActor>> overlayYTickActors;
    vtkNew<vtkScalarBarActor> colorbar;
    vtkNew<vtkCoordinate> coordinate;
    bool showWcsAxes{ true };
    bool probeModeActive{ false };
    RegionMode regionMode{ RegionMode::None };
    bool useSexagesimalWcsFormat{ false };
    bool wcsFormatExplicitlyChosen{ false };
    bool wcsOverlayInitialized{ false };
    bool probeFrozen{ false };
    bool probeValid{ false };
    bool regionDragging{ false };
    bool regionValid{ false };
    bool catalogueOverlayLoaded{ false };
    bool largeImageModeActive{ false };
    bool suspendHeavyOverlayUpdates{ false };
    bool ignoreNextPolygonRelease{ false };
    std::array<int, 2> probeVoxel{ -1, -1 };
    std::array<int, 2> regionAnchorVoxel{ -1, -1 };
    std::array<int, 2> regionCurrentVoxel{ -1, -1 };
    std::vector<std::array<int, 2>> regionPolygonVertices;
    std::vector<std::array<double, 2>> savedLinearDisplayRanges;
    std::vector<std::array<double, 2>> catalogueOverlayPixels;
    std::vector<std::vector<std::array<double, 2>>> catalogueOverlayPolylines;
    std::vector<int> catalogueOverlayLabelIndices;
    QStringList catalogueOverlayLabels;
    QString catalogueOverlaySummary;
    double regionAnnulusInnerRadius{ 0.0 };
    std::array<double, 4> lastOverlayVisibleBounds{ std::numeric_limits<double>::quiet_NaN(),
                                                    std::numeric_limits<double>::quiet_NaN(),
                                                    std::numeric_limits<double>::quiet_NaN(),
                                                    std::numeric_limits<double>::quiet_NaN() };
    std::array<int, 2> lastOverlayViewportSize{ -1, -1 };
    int lastRenderedWcsFrame{ std::numeric_limits<int>::min() };
    int lastRenderedWcsFormat{ -1 };
    vtkNew<vtkLineSource> probeHorizontalLine;
    vtkNew<vtkLineSource> probeVerticalLine;
    vtkNew<vtkActor> probeHorizontalActor;
    vtkNew<vtkActor> probeVerticalActor;
    vtkNew<vtkLineSource> regionTopLine;
    vtkNew<vtkLineSource> regionBottomLine;
    vtkNew<vtkLineSource> regionLeftLine;
    vtkNew<vtkLineSource> regionRightLine;
    vtkNew<vtkActor> regionTopActor;
    vtkNew<vtkActor> regionBottomActor;
    vtkNew<vtkActor> regionLeftActor;
    vtkNew<vtkActor> regionRightActor;
    vtkNew<vtkRegularPolygonSource> regionCircleSource;
    vtkNew<vtkActor> regionCircleActor;
    vtkNew<vtkRegularPolygonSource> regionAnnulusOuterSource;
    vtkNew<vtkRegularPolygonSource> regionAnnulusInnerSource;
    vtkNew<vtkActor> regionAnnulusOuterActor;
    vtkNew<vtkActor> regionAnnulusInnerActor;
    vtkNew<vtkPoints> regionAnnulusFillPoints;
    vtkNew<vtkCellArray> regionAnnulusFillCells;
    vtkNew<vtkPolyData> regionAnnulusFillData;
    vtkNew<vtkActor> regionAnnulusFillActor;
    vtkNew<vtkPoints> regionPolygonPoints;
    vtkNew<vtkCellArray> regionPolygonCells;
    vtkNew<vtkPolyData> regionPolygonData;
    vtkNew<vtkActor> regionPolygonActor;
    vtkNew<vtkPolyData> regionPolygonFillData;
    vtkNew<vtkContourTriangulator> regionPolygonTriangulator;
    vtkNew<vtkActor> regionPolygonFillActor;
    vtkNew<vtkPoints> catalogueOverlayPoints;
    vtkNew<vtkCellArray> catalogueOverlayCells;
    vtkNew<vtkPolyData> catalogueOverlayData;
    vtkNew<vtkActor> catalogueOverlayActor;
    std::vector<vtkSmartPointer<vtkTextActor>> catalogueOverlayLabelActors;
    void setupRenderer();
    void mouseCallback();
    bool updateProbeFromDisplayPosition(int displayX, int displayY);
    void refreshProbeOverlay();
    void clearProbe();
    void updateProbeProfile();
    void updateWcsOverlay();
    void setWcsOverlayVisible(bool visible);
    void ensureOverlayTickActors(vtkRenderer *renderer);
    void invalidateWcsOverlayCache();
    void applyDefaultWcsFormatForSelectedFrame();
    void applyInitialAutoscale(vtkImageData *imageData, vtkLookupTable *lut, const char *context);
    void requestWcsOverlayRender();
    void refreshWcsOverlayImmediately();
    void updateDataStatePanel();
    void updateSanityPanel();
    QString currentWcsFrameLabel() const;
    void setRegionMode(RegionMode mode, bool active);
    bool updateRegionFromDisplayPosition(int displayX, int displayY);
    void refreshRegionOverlay();
    void clearRegion();
    void analyzeCurrentRegion();
    bool finalizePolygonRegion();
    void updateLargeImageMode(vtkImageData *imageData, const char *context);
    void loadCatalogueOverlay();
    void clearCatalogueOverlay();
    void setCatalogueOverlayVisible(bool visible);
    void rebuildCatalogueOverlay();
    void updateCatalogueOverlayLabels();
    bool catalogueWorldToPixel(double raDeg, double decDeg, std::array<double, 2> &pixel) const;
    QString formatLocalProbeCoordinate(int axis, const std::array<int, 2> &voxel) const;
    QString selectedFrameAxisTitle(int axis) const;

    // Stack
    vtkNew<vtkImageStack> stack;
    LayerListModel *layers;
    std::unique_ptr<ImageLayerController> layerController;
    void applyLoadedLayer(const ImageLayerLoadResult &result);
    void applyRemoteMasterLayer(const ImageLayerLoadResult &result);
    void startRemoteFullResolutionLoad();
    void markDisplayStateAdjusted(const char *reason);
    bool isBusy() const;
    void showPersistentStatusMessage(const QString &text, int minDurationMs = 400);
    void clearPersistentStatusMessage();
    void setLayerImportEnabled(bool enabled);
    int currentLayerIndex() const;
    void addLayerImage(const std::string &filepath);
    bool remoteHasWcsAxis(int axis) const;
    double remoteVoxelToWcs(int axis, double voxelIndex, bool *ok = nullptr) const;
    QString remoteFormatAxisCoordinate(int axis, double voxelIndex) const;
    QString remoteAxisTitle(int axis) const;
    int selectedWcsFrame() const;
    int remoteNativeCelestialFrame() const;
    bool remoteHasCelestialAxes() const;
    bool convertRemoteCelestialCoordinates(double nativeX, double nativeY, double &frameX,
                                           double &frameY) const;
    QString formatRemoteOverlayCoordinate(int axis, double value) const;
    QString remoteOverlayAxisTitle(int axis) const;
    QString formatDegreeCoordinate(double value) const;
};

#endif
