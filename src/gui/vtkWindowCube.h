#ifndef vtkWindowCube_H
#define vtkWindowCube_H

#include "AstroUtils.h"

#include <vtkNew.h>

#include <QMainWindow>
#include <QPointer>

#include <memory>

class CubeViewController;
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
class vtkImageReslice;
class vtkLegendScaleActorWCS;
class vtkLookupTable;
class vtkMomentMapFilter;
class vtkOrientationMarkerWidget;
class vtkPiecewiseFunction;
class vtkVolume;

QT_BEGIN_NAMESPACE
namespace Ui {
class vtkWindowCube;
}
QT_END_NAMESPACE

class vtkWindowCube : public QMainWindow
{
    Q_OBJECT

public:
    vtkWindowCube(const QString &filepath, QWidget *parent = nullptr);
    ~vtkWindowCube() override;

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
    Ui::vtkWindowCube *ui;
    const QString filepath;
    AstroUtils astro;

    QPointer<LUTCustomizerDialog> lutCustomizer;
    QPointer<ProfileWidget> profileWidget;

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
    vtkNew<vtkFlyingEdges3D> isosurfaceFilter;
    vtkNew<vtkActor> isosurface;
    vtkNew<vtkVolume> volume;
    vtkNew<vtkColorTransferFunction> volumeColorTransferFunction;
    vtkNew<vtkPiecewiseFunction> volumeOpacity;
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
    vtkNew<vtkLookupTable> lutMoment;
    vtkNew<vtkLegendScaleActorWCS> legendMoment;
    void setMomentOrder(int order);
};
#endif
