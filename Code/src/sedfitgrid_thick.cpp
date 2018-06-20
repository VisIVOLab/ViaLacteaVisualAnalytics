#include "sedfitgrid_thick.h"
#include "ui_sedfitgrid_thick.h"

SedFitgrid_thick::SedFitgrid_thick(SEDVisualizerPlot *s, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SedFitgrid_thick)
{
    ui->setupUi(this);
     sedwin=s;
}

SedFitgrid_thick::~SedFitgrid_thick()
{
    delete ui;
}

void SedFitgrid_thick::closeEvent(QCloseEvent *event)
{
    sedwin->activateWindow();
}
