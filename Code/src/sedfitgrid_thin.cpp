#include "sedfitgrid_thin.h"
#include "ui_sedfitgrid_thin.h"

SedFitGrid_thin::SedFitGrid_thin(SEDVisualizerPlot *s, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SedFitGrid_thin)
{
    ui->setupUi(this);
    sedwin=s;
}

SedFitGrid_thin::~SedFitGrid_thin()
{
    delete ui;
}

void SedFitGrid_thin::on_pushButton_clicked()
{

}

void SedFitGrid_thin::closeEvent(QCloseEvent *event)
{
    sedwin->activateWindow();
}
