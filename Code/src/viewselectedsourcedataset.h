#ifndef VIEWSELECTEDSOURCEDATASET_H
#define VIEWSELECTEDSOURCEDATASET_H

#include <QWidget>
#include <QListWidgetItem>
#include "vstabledesktop.h"
#include "vtkwindow_new.h"
namespace Ui {
class ViewSelectedSourceDataset;
}

class ViewSelectedSourceDataset : public QWidget
{
    Q_OBJECT

public:
    explicit ViewSelectedSourceDataset(vtkwindow_new *v, QHash <int,QString> selFields, QList<QListWidgetItem*> selSources, QWidget *parent = 0);
    ~ViewSelectedSourceDataset();

private:
    Ui::ViewSelectedSourceDataset *ui;
    QHash <int,QString> selectedFields;
     QList<QListWidgetItem*> selectedSources;
     VSTableDesktop *table;
     vtkwindow_new *vtkwin;
};

#endif // VIEWSELECTEDSOURCEDATASET_H
