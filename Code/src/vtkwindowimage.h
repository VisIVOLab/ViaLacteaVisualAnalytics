#ifndef VTKWINDOWIMAGE_H
#define VTKWINDOWIMAGE_H

#include <QListWidgetItem>
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

    void addLayerImage(vtkSmartPointer<vtkFitsReader> fitsReader, const QString &survey = QString(),
                       const QString &species = QString(), const QString &transition = QString());

private slots:
    void on_opacitySlider_valueChanged(int value);
    void on_lutComboBox_textActivated(const QString &lut);
    void on_logRadioBtn_toggled(bool checked);

    void on_layersList_itemClicked(QListWidgetItem *item);
    void on_layersList_itemChanged(QListWidgetItem *item);
    void layerList_rowsMoved(const QModelIndex &parent, int start, int end,
                             const QModelIndex &destination, int row);

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
    void changeLayerVisibility(int index, bool visibility);
};

#endif // VTKWINDOWIMAGE_H
