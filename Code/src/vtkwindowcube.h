#ifndef VTKWINDOWCUBE_H
#define VTKWINDOWCUBE_H

// #include "vtkwindowimage.h"

#include <QDockWidget>
#include <QMainWindow>
#include <QMap>
#include <QPointer>

#include <vtkSmartPointer.h>

class vtkActor;
class vtkFitsReader;
class vtkFitsReader2;
class vtkFlyingEdges3D;
class vtkResliceImageViewer;
class vtkOrientationMarkerWidget;
class vtkLegendScaleActorWCS;
class vtkwindow_new;
class vtkRenderer;
class vtkLookupTable;
class vtkScalarBarActor;
class FitsImageStatisiticInfo;
class LutCustomize;
class ProfileWindow;

namespace Ui {
class vtkWindowCube;
}

class vtkWindowCube : public QMainWindow
{
    using FitsHeaderMap = QMap<QString, QString>;

    Q_OBJECT

public:
    explicit vtkWindowCube(QWidget *parent, const QString &filepath, int ScaleFactor = 1,
                           QString velocityUnit = "km/s");
    ~vtkWindowCube();
    void showColorbar(bool checked, double min, double max);
    void changeFitsScale(std::string palette, std::string scale, float min, float max);
    Ui::vtkWindowCube *ui;
    vtkSmartPointer<vtkFitsReader2> readerSlice;
    vtkSmartPointer<vtkFitsReader> moment;

private slots:
    void on_sliceSlider_valueChanged(int value);
    void on_sliceSlider_sliderReleased();
    void on_sliceSpinBox_valueChanged(int value);

    void on_actionFront_triggered();
    void on_actionBack_triggered();
    void on_actionTop_triggered();
    void on_actionRight_triggered();
    void on_actionBottom_triggered();
    void on_actionLeft_triggered();

    void on_thresholdText_editingFinished();
    void on_thresholdSlider_sliderReleased();

    void on_contourCheckBox_toggled(bool checked);
    void on_levelText_editingFinished();
    void on_lowerBoundText_editingFinished();
    void on_upperBoundText_editingFinished();

    void on_actionCalculate_order_0_triggered();
    void on_actionCalculate_order_1_triggered();
    void on_actionCalculate_order_6_triggered();
    void on_actionCalculate_order_8_triggered();
    void on_actionCalculate_order_10_triggered();

    void on_actionShowStats_triggered();
    void on_actionSlice_Lookup_Table_triggered();
    void changeSliceView(int mode);
    void setInteractorStyleImage();
    void setInteractorStyleProfile(bool liveMode = false);
    void setInteractorStyleDrawLine();

    void on_actionExtract_spectrum_triggered();
    void extractSpectrum(double x, double y, bool live = false);

    void on_actionPV_triggered();

private:
    QString filepath;
    QString bunit;
    int ScaleFactor;
    QPointer<vtkwindow_new> parentWindow;
    vtkSmartPointer<vtkFitsReader2> readerCube;
    FitsImageStatisiticInfo *fitsStatsWidget;
    QPointer<QDockWidget> dock;
    QPointer<ProfileWindow> profileWin;

    FitsHeaderMap fitsHeader;

    double initialCameraFocalPoint[3];
    double initialCameraPosition[3];

    int currentVisOnSlicePanel;
    int currentSlice;
    QString velocityUnit;

    double lowerBound;
    double upperBound;

    vtkSmartPointer<vtkRenderer> rendererSlice;
    vtkSmartPointer<vtkRenderer> rendererMoment;
    vtkSmartPointer<vtkLegendScaleActorWCS> legendActorCube;
    vtkSmartPointer<vtkLegendScaleActorWCS> legendActorSlice;
    vtkSmartPointer<vtkLegendScaleActorWCS> legendActorMoment;
    vtkSmartPointer<vtkActor> planeActor;
    vtkSmartPointer<vtkFlyingEdges3D> isosurface;
    vtkSmartPointer<vtkOrientationMarkerWidget> axesWidget;
    vtkSmartPointer<vtkResliceImageViewer> sliceViewer;
    vtkSmartPointer<vtkResliceImageViewer> momViewer;
    vtkSmartPointer<vtkActor> contoursActor;
    vtkSmartPointer<vtkActor> contoursActorForParent;
    QPointer<LutCustomize> lcustom;
    QString lutName;
    vtkSmartPointer<vtkLookupTable> lutSlice;
    vtkSmartPointer<vtkScalarBarActor> scalarBar;

    void changeLegendWCS(int wcs);
    int readFitsHeader();
    void showStatusBarMessage(const std::string &msg);
    void updateSliceDatacube();
    void updateVelocityText();
    void setThreshold(double threshold);
    void showContours();
    void removeContours();
    void calculateAndShowMomentMap(int order);
    void generatePositionVelocityPlot(int x1, int y1, int x2, int y2);
    void resetCamera();
    void setCameraAzimuth(double az);
    void setCameraElevation(double el);
};

#endif // VTKWINDOWCUBE_H
