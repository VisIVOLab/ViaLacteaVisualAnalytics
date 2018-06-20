#ifndef SEDFITGRID_THIN_H
#define SEDFITGRID_THIN_H

#include <QWidget>
#include "sedvisualizerplot.h"

class SEDVisualizerPlot;

namespace Ui {
class SedFitGrid_thin;
}

class SedFitGrid_thin : public QWidget
{
    Q_OBJECT

public:
    explicit SedFitGrid_thin(SEDVisualizerPlot *s, QWidget *parent = 0);
    ~SedFitGrid_thin();
    Ui::SedFitGrid_thin *ui;


private slots:
    void on_pushButton_clicked();
    void closeEvent(QCloseEvent *event);


private:
    SEDVisualizerPlot *sedwin;
};

#endif // SEDFITGRID_THIN_H
