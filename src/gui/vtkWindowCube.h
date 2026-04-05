#ifndef vtkWindowCube_H
#define vtkWindowCube_H

#include "AstroUtils.h"
#include "CatalogueOverlayUtils.h"
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
#include <QStringList>
#include <QTimer>

#include <array>
#include <limits>
#include <memory>
#include <vector>

class CubeViewController;
class QAction;
class QCheckBox;
class CatalogueTableModel;
class QDockWidget;
class QLabel;
class LUTCustomizerDialog;
class ProfileWidget;
class PvDiagramWidget;
class QTableView;
class vtkAxisActor2D;
class vtkActor;
class vtkColorTransferFunction;
class vtkContourTriangulator;
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
class vtkLineSource;
class vtkLookupTable;
class vtkMomentMapFilter;
class vtkOrientationMarkerWidget;
class vtkPlaneSource;
class vtkPiecewiseFunction;
class vtkPoints;
class vtkCellArray;
class vtkPolyData;
class vtkRegularPolygonSource;
class vtkRenderer;
class vtkTextActor;
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
    bool meshInDisplayCoordinates{ false };
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
                  const std::array<double, 3> &remoteOrigin,
                  const std::array<QString, 3> &remoteCtype,
                  const std::array<QString, 3> &remoteCunit,
                  const std::array<double, 3> &remoteCrval,
                  const std::array<double, 3> &remoteCrpix,
                  const std::array<double, 3> &remoteCdelt,
                  const QString &remoteDegenerateAxesSummary,
                  const QString &remoteWcsStatus = QStringLiteral("ok"),
                  const QString &remoteWcsWarningMessage = QString(),
                  const QString &remoteSessionId = QString(),
                  const QString &remoteBackendToken = QString(), QWidget *parent = nullptr);
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
    void setInteractorStyleRegion();
    void toggleProbeFreeze();
    void finishRegionInteraction();
    void finalizePvInteraction();
    void extractSpectrumAtCurrentProbe();
    void setPvModeActive(bool active);
    void setProbeModeActive(bool active);
    void syncCatalogueTableSelection(int index);

signals:
    void catalogueSourceSelectionChanged(int index);

public:
    enum class RemoteCubeDisplayState
    {
        Preview,
        LoadingFullResolution,
        FullResolution,
    };

    enum class SpectralAxisKind
    {
        Channel,
        Frequency,
        RadioVelocity,
        OpticalVelocity,
        GenericVelocity,
        GenericSpectral,
    };

    struct SpectralAxisDescriptor
    {
        SpectralAxisKind kind{ SpectralAxisKind::Channel };
        QString unit;
        QString label;
        QString sourceLabel;
        bool trusted{ false };
        bool inferred{ false };
        bool physical{ false };
    };

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
    struct MomentMapApplyResult
    {
        vtkSmartPointer<vtkImageData> imageData;
        std::array<double, 2> imageRange;
    };

    struct MomentGenerationConfig
    {
        int order{ 0 };
        int channelStart{ 0 };
        int channelEnd{ 0 };
        bool maskEnabled{ false };
        double thresholdValue{ 0.0 };
    };

    struct MomentProvenanceState
    {
        bool valid{ false };
        QString summary;
        QString details;
    };

    Ui::vtkWindowCube *ui;
    const QString filepath;
    const bool isRemoteMode;
    const QString remoteBackendUrl;
    const QString remoteDatasetId;
    const QString remoteSessionId;
    const QString remoteBackendToken;
    const int remoteDatasetWidth;
    const int remoteDatasetHeight;
    const int remoteDatasetDepth;
    const std::array<double, 3> remoteDatasetSpacing;
    const std::array<double, 3> remoteDatasetOrigin;
    const std::array<QString, 3> remoteDatasetCtype;
    const std::array<QString, 3> remoteDatasetCunit;
    const std::array<double, 3> remoteDatasetCrval;
    const std::array<double, 3> remoteDatasetCrpix;
    const std::array<double, 3> remoteDatasetCdelt;
    const QString remoteDegenerateAxesSummary;
    const QString remoteWcsStatus;
    const QString remoteWcsWarningMessage;
    std::unique_ptr<AstroUtils> astro;

    QPointer<LUTCustomizerDialog> lutCustomizer;
    QPointer<ProfileWidget> profileWidget;
    QPointer<ProfileWidget> probePlotWidget;
    QPointer<PvDiagramWidget> pvDiagramWidget;
    QPointer<QLabel> cubeOpenStateLabel;
    QPointer<QLabel> hoverReadoutLabel;
    QPointer<QLabel> dataStateLabel;
    QPointer<QLabel> sanityLabel;
    QPointer<QLabel> wcsStatusLabel;
    QPointer<QLabel> momentProvenanceLabel;
    QPointer<QLabel> catalogueInfoLabel;
    QPointer<QDockWidget> catalogueDock;
    QPointer<QTableView> catalogueTableView;
    QPointer<CatalogueTableModel> catalogueTableModel;
    bool syncingCatalogueSelection{ false };
    QPointer<QCheckBox> remoteRoiRefinementCheck;
    QPointer<QCheckBox> wcsAxesCheck;
    QPointer<QAction> actionWcsSexagesimal;
    QPointer<QAction> actionWcsDecimal;
    QPointer<QAction> actionBoxRegion;
    QPointer<QAction> actionCircleRegion;
    QPointer<QAction> actionPolygonRegion;
    QPointer<QAction> actionAnnulusRegion;
    QPointer<QAction> actionExtractPvDiagram;
    QPointer<QAction> actionLoadCatalogueOverlay;
    QPointer<QAction> actionShowCatalogueOverlay;
    QPointer<QAction> actionShowCatalogueLabels;
    QPointer<QAction> actionClearCatalogueOverlay;
    QFutureWatcher<CubeOpenStageResult> cubeOpenWatcher;
    QFutureWatcher<RemoteCubePreviewResult> remotePreviewWatcher;
    QFutureWatcher<RemoteCubeSubvolumeResult> remoteHighResCubeWatcher;
    QFutureWatcher<MomentMapComputeResult> momentComputeWatcher;
    QFutureWatcher<AsyncIsosurfaceResult> isosurfaceWatcher;
    int currentRemotePreviewRequestId{ 0 };
    int currentRemoteHighResRequestId{ 0 };
    int currentRemoteSliceRequestId{ 0 };
    std::array<int, 6> currentRemoteRoi{ 0, 0, 0, 0, 0, 0 };
    QString currentRemoteRefinementModeLabel{ QStringLiteral("Full") };
    bool currentRemoteRoiThicknessExpanded{ false };
    int currentRequestedRemoteSliceIndex{ 0 };
    int pendingRemoteSliceIndex{ 0 };
    int activeRemoteSliceRequests{ 0 };
    int activeRemoteIsosurfaceRequests{ 0 };
    QTimer isosurfaceDebounceTimer;
    QTimer remoteSliceDebounceTimer;
    QTimer remoteFullResolutionStateTimer;
    int currentMomentRequestId{ 0 };
    int currentIsosurfaceRequestId{ 0 };
    int currentFullCubeGeneration{ 0 };
    int lastIsosurfacePrewarmGeneration{ 0 };
    QTimer statusMessageClearTimer;
    QElapsedTimer statusMessageElapsed;
    int statusMessageMinDurationMs{ 0 };
    bool persistentStatusActive{ false };
    bool usingHighResCube{ false };
    bool useCameraRoiRefinement{ false };
    bool pendingRemoteRefinementReload{ false };
    bool probeModeActive{ false };
    RegionMode regionMode{ RegionMode::None };
    bool pvModeActive{ false };
    bool pvDragging{ false };
    bool pvValid{ false };
    bool pvCursorValid{ false };
    int pvWidthPixels{ 1 };
    bool useSexagesimalWcsFormat{ false };
    bool wcsFormatExplicitlyChosen{ false };
    bool sliceWcsOverlayInitialized{ false };
    bool momentWcsOverlayInitialized{ false };
    bool probeFrozen{ false };
    bool probeValid{ false };
    bool regionDragging{ false };
    bool regionValid{ false };
    bool catalogueOverlayLoaded{ false };
    bool ignoreNextPolygonRelease{ false };
    MomentGenerationConfig currentMomentConfig;
    MomentProvenanceState momentProvenanceState;
    std::array<int, 3> probeVoxel{ -1, -1, -1 };
    std::array<int, 2> regionAnchorVoxel{ -1, -1 };
    std::array<int, 2> regionCurrentVoxel{ -1, -1 };
    std::vector<std::array<int, 2>> regionPolygonVertices;
    std::vector<CatalogueOverlayEntry> catalogueOverlayEntries;
    std::vector<std::array<double, 2>> catalogueOverlayPixels;
    std::vector<std::vector<std::array<double, 2>>> catalogueOverlayPolylines;
    std::vector<int> catalogueOverlaySourceFirstPolyline;
    std::vector<int> catalogueOverlaySourcePolylineCount;
    std::vector<int> catalogueOverlayLabelIndices;
    QStringList catalogueOverlayLabels;
    QString catalogueOverlaySummary;
    int hoveredCatalogueSourceIndex{ -1 };
    int selectedCatalogueSourceIndex{ -1 };
    double regionAnnulusInnerRadius{ 0.0 };
    std::array<int, 2> pvCurrentVoxel{ -1, -1 };
    std::vector<std::array<int, 2>> pvPolylineVertices;
    RemoteCubeDisplayState remoteCubeDisplayState{ RemoteCubeDisplayState::Preview };
    static constexpr int remoteLoadingStateDelayMs = 250;
    static constexpr int remoteSliceDebounceDelayMs = 100;
    bool remoteIsosurfaceRequestInFlight{ false };
    double inFlightRemoteIsosurfaceThreshold{ std::numeric_limits<double>::quiet_NaN() };

    vtkNew<vtkFITSReader> reader;
    std::array<double, 2> currentCubeVisibleRange{ 0., 0. };
    double currentCubeInvisibleSentinel{ -1.0 };
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
    bool updateProbeFromDisplayPosition(int displayX, int displayY);
    bool updateRegionFromDisplayPosition(int displayX, int displayY);
    bool updatePvFromDisplayPosition(int displayX, int displayY);
    void refreshProbeOverlay();
    void refreshRegionOverlay();
    void refreshPvOverlay();
    void clearProbe();
    void clearRegion();
    void clearPv();
    void analyzeCurrentRegion();
    bool finalizePolygonRegion();
    void extractCurrentPvDiagram();
    void loadCatalogueOverlay();
    void clearCatalogueOverlay();
    void setCatalogueOverlayVisible(bool visible);
    void rebuildCatalogueOverlay();
    void updateCatalogueOverlayLabels();
    void refreshCatalogueOverlayInteraction(int displayX, int displayY, bool fromClick);
    int catalogueSourceIndexNearDisplayPosition(int displayX, int displayY,
                                                vtkRenderer *renderer, double *distancePx = nullptr) const;
    void rebuildCatalogueHighlightOverlay(vtkPoints *points, vtkCellArray *cells, vtkPolyData *data,
                                          int sourceIndex);
    void updateCatalogueInfoPanel();
    QString catalogueSourceSummary(int sourceIndex) const;
    void ensureCatalogueDock();
    void refreshCatalogueTable();
    void setSelectedCatalogueSourceIndex(int index);
    void centerViewOnCatalogueSource(int index, double zoomFactor = 2.5);
    bool catalogueWorldToPixel(double raDeg, double decDeg, std::array<double, 2> &pixel) const;
    void updateProbeReadout(vtkImageData *imageData);
    void updateProbePlot();
    QString formatSpatialPointSummary(const std::array<int, 2> &voxel) const;
    std::vector<std::array<int, 2>> pvSampledPath(
            const std::vector<std::array<int, 2>> &vertices) const;
    std::array<double, 2> pvLocalNormalForSample(
            const std::vector<std::array<int, 2>> &sampledPoints, std::size_t index) const;
    QString formatLocalProbeCoordinate(int axis, const std::array<int, 3> &voxel) const;
    QString selectedFrameAxisTitle(int axis) const;
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
    std::array<int, 6> computeVisibleROI();
    bool requestHighResCube();
    void updateRemoteCuttingPlane(int sliceIndex);
    int remoteSliceCount() const;
    int clampRemoteSliceIndex(int sliceIndex) const;
    double remoteSliceCoordinate(int sliceIndex) const;
    SpectralAxisDescriptor spectralAxisDescriptor() const;
    double spectralAxisValue(double datasetVoxelIndex, bool *ok = nullptr) const;
    QString formatSpectralAxisValue(double datasetVoxelIndex) const;
    QString spectralAxisTitle() const;
    QString spectralAxisTooltip() const;
    void refreshSpectralAxisUi();
    bool remoteHasWcsAxis(int axis) const;
    double remoteVoxelToWcs(int axis, double voxelIndex, bool *ok = nullptr) const;
    QString remoteFormatAxisCoordinate(int axis, double voxelIndex) const;
    QString remoteAxisTitle(int axis) const;
    void updateWcsStatusIndicator();
    int selectedWcsFrame() const;
    int remoteNativeCelestialFrame() const;
    bool remoteHasCelestialAxes() const;
    bool convertRemoteCelestialCoordinates(double nativeX, double nativeY, double &frameX,
                                           double &frameY) const;
    QString formatRemoteOverlayCoordinate(int axis, double value) const;
    QString remoteOverlayAxisTitle(int axis) const;
    QString formatDegreeCoordinate(double value) const;
    void updateDataStatePanel();
    QString currentWcsFrameLabel() const;
    void setRegionMode(RegionMode mode, bool active);
    void updateSliceWcsOverlay();
    void updateMomentWcsOverlay();
    void set2dWcsOverlayVisible(bool visible);
    void ensureOverlayTickActors(vtkRenderer *renderer,
                                 std::vector<vtkSmartPointer<vtkTextActor>> &xActors,
                                 std::vector<vtkSmartPointer<vtkTextActor>> &yActors);
    void invalidateWcsOverlayCache();
    void applyDefaultWcsFormatForSelectedFrame();
    void requestWcsOverlayRender();
    bool configureMomentRequest(int defaultOrder, MomentGenerationConfig &config);
    QString describeMomentOrder(int order) const;
    QString describeMomentScope() const;
    QString formatMomentChannelRange(const MomentGenerationConfig &config) const;
    void updateMomentProvenancePanel();
    void updateSanityPanel();

    // Slice
    vtkNew<vtkImageReslice> slice;
    vtkNew<vtkImageMapToColors> sliceColors;
    vtkNew<vtkLookupTable> lutSlice;
    vtkNew<vtkExtractVOI> sliceOnCube;
    vtkNew<vtkLookupTable> lutSliceOnCube;
    vtkNew<vtkTrivialProducer> remoteSliceDisplaySource;
    vtkNew<vtkLegendScaleActorWCS> legendSlice;
    vtkNew<vtkAxisActor2D> sliceOverlayXAxis;
    vtkNew<vtkAxisActor2D> sliceOverlayYAxis;
    vtkNew<vtkTextActor> sliceOverlayXTitleActor;
    vtkNew<vtkTextActor> sliceOverlayYTitleActor;
    vtkNew<vtkLineSource> sliceProbeHorizontalLine;
    vtkNew<vtkLineSource> sliceProbeVerticalLine;
    vtkNew<vtkActor> sliceProbeHorizontalActor;
    vtkNew<vtkActor> sliceProbeVerticalActor;
    vtkNew<vtkActor> sliceCatalogueOverlayActor;
    vtkNew<vtkActor> sliceCatalogueHoverOverlayActor;
    vtkNew<vtkActor> sliceCatalogueSelectionOverlayActor;
    vtkNew<vtkLineSource> sliceRegionTopLine;
    vtkNew<vtkLineSource> sliceRegionBottomLine;
    vtkNew<vtkLineSource> sliceRegionLeftLine;
    vtkNew<vtkLineSource> sliceRegionRightLine;
    vtkNew<vtkActor> sliceRegionTopActor;
    vtkNew<vtkActor> sliceRegionBottomActor;
    vtkNew<vtkActor> sliceRegionLeftActor;
    vtkNew<vtkActor> sliceRegionRightActor;
    vtkNew<vtkRegularPolygonSource> sliceRegionCircleSource;
    vtkNew<vtkActor> sliceRegionCircleActor;
    vtkNew<vtkRegularPolygonSource> sliceRegionAnnulusOuterSource;
    vtkNew<vtkRegularPolygonSource> sliceRegionAnnulusInnerSource;
    vtkNew<vtkActor> sliceRegionAnnulusOuterActor;
    vtkNew<vtkActor> sliceRegionAnnulusInnerActor;
    vtkNew<vtkPoints> sliceRegionAnnulusFillPoints;
    vtkNew<vtkCellArray> sliceRegionAnnulusFillCells;
    vtkNew<vtkPolyData> sliceRegionAnnulusFillData;
    vtkNew<vtkActor> sliceRegionAnnulusFillActor;
    vtkNew<vtkPoints> sliceRegionPolygonPoints;
    vtkNew<vtkCellArray> sliceRegionPolygonCells;
    vtkNew<vtkPolyData> sliceRegionPolygonData;
    vtkNew<vtkActor> sliceRegionPolygonActor;
    vtkNew<vtkPolyData> sliceRegionPolygonFillData;
    vtkNew<vtkContourTriangulator> sliceRegionPolygonTriangulator;
    vtkNew<vtkActor> sliceRegionPolygonFillActor;
    vtkNew<vtkPoints> slicePvPoints;
    vtkNew<vtkCellArray> slicePvCells;
    vtkNew<vtkPolyData> slicePvData;
    vtkNew<vtkActor> slicePvActor;
    vtkNew<vtkPoints> slicePvUpperPoints;
    vtkNew<vtkCellArray> slicePvUpperCells;
    vtkNew<vtkPolyData> slicePvUpperData;
    vtkNew<vtkActor> slicePvUpperActor;
    vtkNew<vtkPoints> slicePvLowerPoints;
    vtkNew<vtkCellArray> slicePvLowerCells;
    vtkNew<vtkPolyData> slicePvLowerData;
    vtkNew<vtkActor> slicePvLowerActor;
    vtkNew<vtkPoints> catalogueHoverOverlayPoints;
    vtkNew<vtkCellArray> catalogueHoverOverlayCells;
    vtkNew<vtkPolyData> catalogueHoverOverlayData;
    vtkNew<vtkPoints> catalogueSelectionOverlayPoints;
    vtkNew<vtkCellArray> catalogueSelectionOverlayCells;
    vtkNew<vtkPolyData> catalogueSelectionOverlayData;
    std::vector<vtkSmartPointer<vtkTextActor>> sliceCatalogueOverlayLabelActors;
    std::vector<vtkSmartPointer<vtkTextActor>> sliceOverlayXTickActors;
    std::vector<vtkSmartPointer<vtkTextActor>> sliceOverlayYTickActors;
    std::array<double, 4> lastSliceOverlayVisibleBounds{ std::numeric_limits<double>::quiet_NaN(),
                                                         std::numeric_limits<double>::quiet_NaN(),
                                                         std::numeric_limits<double>::quiet_NaN(),
                                                         std::numeric_limits<double>::quiet_NaN() };
    std::array<int, 2> lastSliceOverlayViewportSize{ -1, -1 };
    QHash<QString, RemoteCubeSliceResult> remoteSliceCache;
    QList<QString> remoteSliceCacheLru;
    QSet<QString> remoteSliceFetchesInFlight;
    static constexpr int remoteSliceCacheCapacity = 8;
    void updateSlice();
    void updateRemoteSliceDragFeedback(int sliceIndex);
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
    vtkNew<vtkAxisActor2D> momentOverlayXAxis;
    vtkNew<vtkAxisActor2D> momentOverlayYAxis;
    vtkNew<vtkTextActor> momentOverlayXTitleActor;
    vtkNew<vtkTextActor> momentOverlayYTitleActor;
    vtkNew<vtkLineSource> momentProbeHorizontalLine;
    vtkNew<vtkLineSource> momentProbeVerticalLine;
    vtkNew<vtkActor> momentProbeHorizontalActor;
    vtkNew<vtkActor> momentProbeVerticalActor;
    vtkNew<vtkActor> momentCatalogueOverlayActor;
    vtkNew<vtkActor> momentCatalogueHoverOverlayActor;
    vtkNew<vtkActor> momentCatalogueSelectionOverlayActor;
    vtkNew<vtkLineSource> momentRegionTopLine;
    vtkNew<vtkLineSource> momentRegionBottomLine;
    vtkNew<vtkLineSource> momentRegionLeftLine;
    vtkNew<vtkLineSource> momentRegionRightLine;
    vtkNew<vtkActor> momentRegionTopActor;
    vtkNew<vtkActor> momentRegionBottomActor;
    vtkNew<vtkActor> momentRegionLeftActor;
    vtkNew<vtkActor> momentRegionRightActor;
    vtkNew<vtkRegularPolygonSource> momentRegionCircleSource;
    vtkNew<vtkActor> momentRegionCircleActor;
    vtkNew<vtkRegularPolygonSource> momentRegionAnnulusOuterSource;
    vtkNew<vtkRegularPolygonSource> momentRegionAnnulusInnerSource;
    vtkNew<vtkActor> momentRegionAnnulusOuterActor;
    vtkNew<vtkActor> momentRegionAnnulusInnerActor;
    vtkNew<vtkPoints> momentRegionAnnulusFillPoints;
    vtkNew<vtkCellArray> momentRegionAnnulusFillCells;
    vtkNew<vtkPolyData> momentRegionAnnulusFillData;
    vtkNew<vtkActor> momentRegionAnnulusFillActor;
    vtkNew<vtkPoints> momentRegionPolygonPoints;
    vtkNew<vtkCellArray> momentRegionPolygonCells;
    vtkNew<vtkPolyData> momentRegionPolygonData;
    vtkNew<vtkActor> momentRegionPolygonActor;
    vtkNew<vtkPolyData> momentRegionPolygonFillData;
    vtkNew<vtkContourTriangulator> momentRegionPolygonTriangulator;
    vtkNew<vtkActor> momentRegionPolygonFillActor;
    vtkNew<vtkPoints> momentPvPoints;
    vtkNew<vtkCellArray> momentPvCells;
    vtkNew<vtkPolyData> momentPvData;
    vtkNew<vtkActor> momentPvActor;
    vtkNew<vtkPoints> momentPvUpperPoints;
    vtkNew<vtkCellArray> momentPvUpperCells;
    vtkNew<vtkPolyData> momentPvUpperData;
    vtkNew<vtkActor> momentPvUpperActor;
    vtkNew<vtkPoints> momentPvLowerPoints;
    vtkNew<vtkCellArray> momentPvLowerCells;
    vtkNew<vtkPolyData> momentPvLowerData;
    vtkNew<vtkActor> momentPvLowerActor;
    vtkNew<vtkPoints> catalogueOverlayPoints;
    vtkNew<vtkCellArray> catalogueOverlayCells;
    vtkNew<vtkPolyData> catalogueOverlayData;
    std::vector<vtkSmartPointer<vtkTextActor>> momentCatalogueOverlayLabelActors;
    std::vector<vtkSmartPointer<vtkTextActor>> momentOverlayXTickActors;
    std::vector<vtkSmartPointer<vtkTextActor>> momentOverlayYTickActors;
    std::array<double, 4> lastMomentOverlayVisibleBounds{ std::numeric_limits<double>::quiet_NaN(),
                                                          std::numeric_limits<double>::quiet_NaN(),
                                                          std::numeric_limits<double>::quiet_NaN(),
                                                          std::numeric_limits<double>::quiet_NaN() };
    std::array<int, 2> lastMomentOverlayViewportSize{ -1, -1 };
    bool showWcsAxes{ true };
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
