#ifndef VTKWINDOWIMAGE_H
#define VTKWINDOWIMAGE_H

#include <QMainWindow>

#include <vtkSmartPointer.h>

class vtkImageStack;
class vtkFitsReader;
class vtkfitstoolwidgetobject;

namespace Ui {
class vtkWindowImage;
}

class vtkWindowImage : public QMainWindow
{
    Q_OBJECT

public:
    explicit vtkWindowImage(QWidget *parent, vtkSmartPointer<vtkFitsReader> fitsReader);
    ~vtkWindowImage();

    vtkSmartPointer<vtkFitsReader> getFitsReader() const;

    void showStatusBarMessage(std::string msg);

private slots:
    void on_opacitySlider_valueChanged(int value);

private:
    Ui::vtkWindowImage *ui;
    vtkSmartPointer<vtkFitsReader> fitsReader;
    vtkSmartPointer<vtkImageStack> imageStack;

    void setInteractorStyleImage();
    void addLayerToList(vtkfitstoolwidgetobject *layer, bool enabled = true);

    int selectedLayerIndex() const;
};

#endif // VTKWINDOWIMAGE_H
