#ifndef LUTCUSTOMIZE_H
#define LUTCUSTOMIZE_H

#include "qcustomplot.h"
#include "vtkwindow_new.h"

#include <QPointer>
#include <QWidget>

namespace Ui {
class LutCustomize;
}

class vtkWindowCube;

class LutCustomize : public QWidget
{
    Q_OBJECT

public:
    explicit LutCustomize(vtkwindow_new *v, QWidget *parent = 0);
    explicit LutCustomize(vtkWindowCube *v, QWidget *parent = 0);
    ~LutCustomize();
    void configurePoint3D();
    void configureFitsImage();
    void configureFits3D();
    void configureMoment();
    Ui::LutCustomize *ui;
    void setScaling(QString scaling);
    void setLut(QString lut);
    void plotHistogram();
    void refreshColorbar();

private slots:
    void on_cancelPushButton_clicked();
    void closeEvent(QCloseEvent *event);
    void on_ShowColorbarCheckBox_clicked(bool checked);
    void drawLine(double from, double to);
    void setRange();
    void on_okPushButton_clicked();
    void on_fromSpinBox_valueChanged(double arg1);
    void on_toSpinBox_valueChanged(double arg1);

    void on_resetMinPushButton_clicked();
    void on_resetMaxPushButton_clicked();

private:
    QPointer<vtkwindow_new> vtkwin;
    QPointer<vtkWindowCube> vtkwincube;
    QCPItemLine *fromLine;
    QCPItemLine *toLine;
    double range[2];
    double y_range[2];
    bool isPoint3D;
    bool isFits2D;
    bool isFits3D;
    bool isMoment;
};

#endif // LUTCUSTOMIZE_H
