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
class FitsImageStatisiticInfo;

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

    void on_actionShowStats_triggered();

private:
    Ui::vtkWindowCube *ui;
    QString filepath;
    int ScaleFactor;
    QPointer<vtkwindow_new> parentWindow;
    vtkSmartPointer<vtkFitsReader2> readerCube;
    vtkSmartPointer<vtkFitsReader2> readerSlice;
    FitsImageStatisiticInfo *fitsStatsWidget;
    QPointer<QDockWidget> dock;

    FitsHeaderMap fitsHeader;

    double initialCameraFocalPoint[3];
    double initialCameraPosition[3];

    int currentSlice;
    QString velocityUnit;

    double lowerBound;
    double upperBound;

    vtkSmartPointer<vtkLegendScaleActorWCS> legendActorCube;
    vtkSmartPointer<vtkLegendScaleActorWCS> legendActorSlice;
    vtkSmartPointer<vtkActor> planeActor;
    vtkSmartPointer<vtkFlyingEdges3D> isosurface;
    vtkSmartPointer<vtkOrientationMarkerWidget> axesWidget;
    vtkSmartPointer<vtkResliceImageViewer> sliceViewer;
    vtkSmartPointer<vtkActor> contoursActor;
    vtkSmartPointer<vtkActor> contoursActorForParent;

    void changeLegendWCS(int wcs);

    int readFitsHeader();
    void showStatusBarMessage(const std::string &msg);

    void updateSliceDatacube();
    void updateVelocityText();

    void setThreshold(double threshold);

    void showContours();
    void removeContours();

    void calculateAndShowMomentMap(int order);

    void resetCamera();
    void setCameraAzimuth(double az);
    void setCameraElevation(double el);
};

#endif // VTKWINDOWCUBE_H
