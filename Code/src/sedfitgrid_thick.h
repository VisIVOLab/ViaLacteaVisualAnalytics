#ifndef SEDFITGRID_THICK_H
#define SEDFITGRID_THICK_H

#include <QWidget>
#include "sedvisualizerplot.h"

class SEDVisualizerPlot;

namespace Ui {
class SedFitgrid_thick;
}

class SedFitgrid_thick : public QWidget
{
    Q_OBJECT

public:
    explicit SedFitgrid_thick(SEDVisualizerPlot *s, QWidget *parent = 0);
    ~SedFitgrid_thick();
    Ui::SedFitgrid_thick *ui;

private slots:
    void closeEvent(QCloseEvent *event);

private:
    SEDVisualizerPlot *sedwin;
};

#endif // SEDFITGRID_THICK_H
