#ifndef LUTCUSTOMIZE_H
#define LUTCUSTOMIZE_H

#include <QWidget>
#include "vtkwindow_new.h"
#include "qcustomplot.h"

namespace Ui {
class LutCustomize;
}

class LutCustomize : public QWidget
{
    Q_OBJECT

public:
    explicit LutCustomize( vtkwindow_new *v,QWidget *parent = 0);
    ~LutCustomize();

private slots:
    void on_cancelPushButton_clicked();
    void closeEvent ( QCloseEvent * event );
    void on_ShowColorbarCheckBox_clicked(bool checked);
    void plotHistogram();
    void drawLine(double from, double to);
    void setRange();


    void on_fromSlider_sliderMoved(int position);


    void on_toSlider_sliderMoved(int position);

    void on_okPushButton_clicked();

private:
    Ui::LutCustomize *ui;
    vtkwindow_new *vtkwin;
    QCPItemLine *fromLine ;
    QCPItemLine *toLine;
    double *range;
    double *y_range;
};

#endif // LUTCUSTOMIZE_H
