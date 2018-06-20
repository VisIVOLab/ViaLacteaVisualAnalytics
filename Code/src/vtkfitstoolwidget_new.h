#ifndef VTKFITSTOOLWIDGET_NEW_H
#define VTKFITSTOOLWIDGET_NEW_H

#include <QMainWindow>
#include "vtkfitstoolwidgetobject.h"
#include <QTreeWidgetItem>

namespace Ui {
class vtkfitstoolwidget_new;
}

class vtkfitstoolwidget_new : public QMainWindow
{
    Q_OBJECT

public:
    explicit vtkfitstoolwidget_new(QWidget *parent = 0);
    ~vtkfitstoolwidget_new();
    void addLayer(vtkfitstoolwidgetobject *o);
//    void setName(QString n);
    void setWavelength(QString w);


private slots:
    void on_layerTreeWidget_itemSelectionChanged();

    void on_savePushButton_clicked();

private:
    Ui::vtkfitstoolwidget_new *ui;
    QList<vtkfitstoolwidgetobject*> layerList;
    QString wavelength;
    //void addTreeRoot(QString name );
    QTreeWidgetItem* addTreeRoot(QString name );

    void addTreeChild(QTreeWidgetItem *parent, QString name, QBrush brush);


};

#endif // VTKFITSTOOLWIDGET_NEW_H
