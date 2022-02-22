#ifndef VTKWINDOWCUBE_H
#define VTKWINDOWCUBE_H

#include <QMainWindow>

#include <vtkSmartPointer.h>

class vtkFitsReader;
class vtkOrientationMarkerWidget;

namespace Ui {
class vtkWindowCube;
}

class vtkWindowCube : public QMainWindow
{
    Q_OBJECT

public:
    explicit vtkWindowCube(QWidget *parent, vtkSmartPointer<vtkFitsReader> fitsReader);
    ~vtkWindowCube();

private:
    Ui::vtkWindowCube *ui;
    vtkSmartPointer<vtkFitsReader> fitsReader;
    vtkSmartPointer<vtkOrientationMarkerWidget> axesWidget;
};

#endif // VTKWINDOWCUBE_H
