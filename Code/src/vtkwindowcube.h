#ifndef VTKWINDOWCUBE_H
#define VTKWINDOWCUBE_H

#include <QMainWindow>

#include <vtkSmartPointer.h>

class vtkActor;
class vtkFitsReader;
class vtkFlyingEdges3D;
class vtkResliceImageViewer;
class vtkOrientationMarkerWidget;

namespace Ui {
class vtkWindowCube;
}

class vtkWindowCube : public QMainWindow
{
    Q_OBJECT

public:
    explicit vtkWindowCube(QWidget *parent, vtkSmartPointer<vtkFitsReader> fitsReader,
                           QString velocityUnit = "km/s");
    ~vtkWindowCube();

private slots:
    void on_sliceSlider_valueChanged(int value);
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

private:
    Ui::vtkWindowCube *ui;
    vtkSmartPointer<vtkFitsReader> fitsReader;

    double initialCameraFocalPoint[3];
    double initialCameraPosition[3];

    int currentSlice;
    QString velocityUnit;

    double lowerBound;
    double upperBound;

    vtkSmartPointer<vtkActor> planeActor;
    vtkSmartPointer<vtkFlyingEdges3D> isosurface;
    vtkSmartPointer<vtkOrientationMarkerWidget> axesWidget;
    vtkSmartPointer<vtkResliceImageViewer> sliceViewer;
    vtkSmartPointer<vtkActor> contoursActor;

    void showStatusBarMessage(const std::string &msg);

    void updateSliceDatacube();
    void updateVelocityText();

    void setThreshold(double threshold);

    void showContours();
    void removeContours();

    void resetCamera();
    void setCameraAzimuth(double az);
    void setCameraElevation(double el);
};

#endif // VTKWINDOWCUBE_H
