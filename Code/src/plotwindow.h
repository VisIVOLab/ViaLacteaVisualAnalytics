#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QWidget>
#include "vtkwindow_new.h"
#include <QListWidgetItem>
#include "vstabledesktop.h"

namespace Ui {
class PlotWindow;
}

class PlotWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PlotWindow(vtkwindow_new *v, QList<QListWidgetItem*> selItems, int id,QWidget *parent = 0);
    ~PlotWindow();

private slots:
    void on_plotButton_clicked();

private:
    Ui::PlotWindow *ui;
    QList<QListWidgetItem*> selectedItems;
    vtkwindow_new *vtkwin;
    VSTableDesktop *table;
};

#endif // PLOTWINDOW_H
