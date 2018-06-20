#ifndef VTKFITSTOOLSWIDGET_H
#define VTKFITSTOOLSWIDGET_H

#include <QWidget>
#include "vtkwindow_new.h"
#include <QTableWidgetItem>

namespace Ui {
class vtkfitstoolswidget;
}

class vtkfitstoolswidget : public QWidget
{
    Q_OBJECT

public:
    explicit vtkfitstoolswidget(vtkwindow_new* v=0,QWidget *parent = 0 );
    ~vtkfitstoolswidget();
    void updateList();
    QVector<QSlider*> sliders;
    void changeLut(std::string palette);
    void changeLutScale(std::string palette, std::string scale);
    void addToList(QString name, vtkSmartPointer<vtkLODActor> actor);
    int getNumOfElements();


private slots:
    void on_selectButton_clicked();
    void on_noSelectButton_clicked();
    void checkboxClicked(int cb);
    void opacityValueChanged(int o);
    void on_lutComboBox_currentIndexChanged(int index);
    void on_addRemoteSourcesPushButton_clicked();
    void on_galacticRadioButton_toggled(bool checked);
    void on_eclipticRadioButton_toggled(bool checked);
    void on_addedSourcesListWidget_doubleClicked(const QModelIndex &index);
    void customMenuRequested(QPoint pos);
    void deleteSelected();
    void on_scaleComboBox_currentIndexChanged(int index);
    void on_datacubeButton_clicked();

public slots:
    void on_addLocalSourcesPushButton_clicked();

private:
    Ui::vtkfitstoolswidget *ui;
    vtkwindow_new *vtkwin;

};

#endif // VTKFITSTOOLSWIDGET_H
