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

    void showStatusBarMessage(const std::string &msg);

private slots:
    void on_opacitySlider_valueChanged(int value);
    void on_lutComboBox_textActivated(const QString &lut);
    void on_logRadioBtn_toggled(bool checked);

private:
    Ui::vtkWindowImage *ui;
    vtkSmartPointer<vtkFitsReader> fitsReader;
    vtkSmartPointer<vtkImageStack> imageStack;
    QList<vtkfitstoolwidgetobject *> imageLayersList;

    inline void render();

    void setInteractorStyleImage();

    int selectedLayerIndex() const;
    void addLayerToList(vtkfitstoolwidgetobject *layer, bool enabled = true);
    void changeFitsLut(const QString &palette, const QString &scale = QString());
};

#endif // VTKWINDOWIMAGE_H
