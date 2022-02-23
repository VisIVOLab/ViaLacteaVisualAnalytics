#ifndef VTKWINDOWCUBE_H
#define VTKWINDOWCUBE_H

#include <QMainWindow>

#include <vtkSmartPointer.h>

class vtkActor;
class vtkFitsReader;
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

private:
    Ui::vtkWindowCube *ui;
    vtkSmartPointer<vtkFitsReader> fitsReader;
    int currentSlice;
    QString velocityUnit;

    vtkSmartPointer<vtkActor> planeActor;
    vtkSmartPointer<vtkOrientationMarkerWidget> axesWidget;
    vtkSmartPointer<vtkResliceImageViewer> sliceViewer;

    void updateSliceDatacube();
    void updateVelocityText();
};

#endif // VTKWINDOWCUBE_H
