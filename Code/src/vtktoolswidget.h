#ifndef VTKTOOLSWIDGET_H
#define VTKTOOLSWIDGET_H

#include <QWidget>
#include "vtkwindow_new.h"
#include "qcustomplot.h"
namespace Ui {
class vtktoolswidget;
}

class vtktoolswidget : public QWidget
{
    Q_OBJECT

public:
    explicit vtktoolswidget( vtkwindow_new* v=0,QWidget *parent = 0 );
    ~vtktoolswidget();
    float *range;

private slots:
    void on_ShowBoxCheckBox_clicked(bool checked);
    void on_ShowAxesCheckBox_clicked(bool checked);
    void on_activateLutCheckBox_clicked(bool checked);
    void on_lutComboBox_currentIndexChanged(const QString &arg1);
    void changeLut(std::string palette);
    void on_ShowColorbarCheckBox_clicked(bool checked);
    void on_scalarComboBox_currentIndexChanged(const QString &arg1);
    void changeScalar(std::string scalar);
    void plotHistogram(int i);
    void on_scalarComboBox_currentIndexChanged(int index);
    void drawLine(double from, double to);
    void on_fromSlider_sliderMoved(int position);
    void on_toSlider_sliderMoved(int position);
    void on_fromValue_textChanged(const QString &arg1);
    void on_toValue_textChanged(const QString &arg1);
    void on_scaleCheckBox_clicked(bool checked);

    void on_cameraLeft_clicked();

    void on_cameraBack_clicked();

    void on_cameraRight_clicked();

    void on_frontCamera_clicked();

    void on_topCamera_clicked();

    void on_bottomCamera_clicked();

    void on_gridCheckBox_clicked(bool checked);
    void on_scalarComboBox_activated(const QString &arg1);

    void on_scaleCheckBox_clicked();

    void on_ShowColorbarCheckBox_clicked();

private:
    Ui::vtktoolswidget *ui;
    vtkwindow_new *vtkwin;
    QCPItemLine *fromLine ;
    QCPItemLine *toLine;
};

#endif // VTKTOOLSWIDGET_H
