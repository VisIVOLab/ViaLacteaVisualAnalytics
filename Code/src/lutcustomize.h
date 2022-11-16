#ifndef LUTCUSTOMIZE_H
#define LUTCUSTOMIZE_H

#include "qcustomplot.h"
#include "vtkwindow_new.h"

#include <QWidget>

namespace Ui {
class LutCustomize;
}

class LutCustomize : public QWidget
{
    Q_OBJECT

public:
    explicit LutCustomize(vtkwindow_new *v, QWidget *parent = 0);
    ~LutCustomize();
    void configurePoint3D();
    void configureFitsImage();
    Ui::LutCustomize *ui;
    void setScaling(QString scaling);
    void setLut(QString lut);


private slots:
    void on_cancelPushButton_clicked();
    void closeEvent(QCloseEvent *event);
    void on_ShowColorbarCheckBox_clicked(bool checked);
    void plotHistogram();
    void drawLine(double from, double to);
    void setRange();

    void on_fromSlider_sliderMoved(int position);

    void on_toSlider_sliderMoved(int position);
    void on_okPushButton_clicked();
    void on_lutComboBox_activated(const QString &arg1);
    void on_scalingComboBox_activated(const QString &arg1);

private:
    vtkwindow_new *vtkwin;
    QCPItemLine *fromLine;
    QCPItemLine *toLine;
    double *range;
    double *y_range;
    bool isPoint3D;
};

#endif // LUTCUSTOMIZE_H
