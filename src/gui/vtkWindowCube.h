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
#include <QMainWindow>
#include <QPointer>
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
class vtkImageReslice;
class vtkLegendScaleActorWCS;
class vtkLookupTable;
class vtkMomentMapFilter;
class vtkOrientationMarkerWidget;
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
    int requestId{ 0 };
};

class vtkWindowCube : public QMainWindow
{
    Q_OBJECT

public:
    vtkWindowCube(const QString &filepath, QWidget *parent = nullptr);
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
    struct MomentMapApplyResult
    {
        vtkSmartPointer<vtkImageData> imageData;
        std::array<double, 2> imageRange;
    };

    Ui::vtkWindowCube *ui;
    const QString filepath;
    AstroUtils astro;

    QPointer<LUTCustomizerDialog> lutCustomizer;
    QPointer<ProfileWidget> profileWidget;
    QPointer<QLabel> cubeOpenStateLabel;
    QFutureWatcher<CubeOpenStageResult> cubeOpenWatcher;
    QFutureWatcher<MomentMapComputeResult> momentComputeWatcher;
    QFutureWatcher<AsyncIsosurfaceResult> isosurfaceWatcher;
    QTimer isosurfaceDebounceTimer;
    int currentMomentRequestId{ 0 };
    int currentIsosurfaceRequestId{ 0 };
    int currentFullCubeGeneration{ 0 };
    int lastIsosurfacePrewarmGeneration{ 0 };
    QTimer statusMessageClearTimer;
    QElapsedTimer statusMessageElapsed;
    int statusMessageMinDurationMs{ 0 };
    bool persistentStatusActive{ false };

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
    void applyCubeOpenResult(const CubeOpenStageResult &result);
    void startAsyncIsosurface(double isoValue);
    void scheduleIsosurfaceRecompute();
    void scheduleIsosurfacePrewarm();
    void setCubeRenderModeLocally(bool isosurfaceMode);
    void updateCube();

    // Slice
    vtkNew<vtkImageReslice> slice;
    vtkNew<vtkLookupTable> lutSlice;
    vtkNew<vtkExtractVOI> sliceOnCube;
    vtkNew<vtkLookupTable> lutSliceOnCube;
    vtkNew<vtkLegendScaleActorWCS> legendSlice;
    void updateSlice();

    // Contours
    int level;
    vtkNew<vtkFlyingEdges2D> contours;
    vtkNew<vtkActor> contoursActor;
    void updateContours();
    void updateContoursVisibility();

    // Moment
    vtkNew<vtkMomentMapFilter> moment;
    vtkNew<vtkTrivialProducer> momentDisplaySource;
    vtkNew<vtkLookupTable> lutMoment;
    vtkNew<vtkLegendScaleActorWCS> legendMoment;
    void applyMomentMapResult(const MomentMapApplyResult &result);
    bool isBusy() const;
    void showPersistentStatusMessage(const QString &text, int minDurationMs = 400);
    void clearPersistentStatusMessage();
    void setCubeOpenStateLabel(const QString &text);
    void setCubeOpenActionsEnabled(bool enabled);
    void setMomentActionsEnabled(bool enabled);
    void setMomentOrder(int order);
};
#endif
