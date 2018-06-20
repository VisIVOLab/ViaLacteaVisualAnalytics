#ifndef HIGALSELECTEDSOURCES_H
#define HIGALSELECTEDSOURCES_H

#include <QWidget>
#include "vtkwindow_new.h"
#include <QList>
#include "plotwindow.h"

namespace Ui {
class HigalSelectedSources;
}

class HigalSelectedSources : public QWidget
{
    Q_OBJECT

public:
    explicit HigalSelectedSources(vtkwindow_new *v,QWidget *parent = 0);
    ~HigalSelectedSources();
    Ui::HigalSelectedSources *ui;

private slots:
    void plotNewWindow();
    void on_datasetButton_clicked();
    void on_selectAllButton_clicked();
    void on_deselectAllButton_clicked();
    void updateExistingWindowMenu();
    void on_sedButton_clicked();
    void sourceChangedEvent(QListWidgetItem* cur,QListWidgetItem* pre);
    void itemSelectionChanged();
    void itemPressed(QListWidgetItem* cur);
    void drawSingleEllipse(vtkEllipse * ellipse );
    void closeEvent(QCloseEvent *event);
    void on_tabWidget_currentChanged(int index);

public slots:
    void setConnect(QListWidget *list);

private:
        vtkwindow_new *vtkwin;
        QList<PlotWindow*> plotWindowList;
        QMenu* plotMenu;
        QMenu* existingWindowMenu;
        vtkSmartPointer<vtkLODActor>  ellipseActor;
        bool itemSelected;
        bool itemChanged;

};

#endif // HIGALSELECTEDSOURCES_H
