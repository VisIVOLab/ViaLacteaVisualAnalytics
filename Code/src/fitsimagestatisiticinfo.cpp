#include "fitsimagestatisiticinfo.h"
#include "ui_fitsimagestatisiticinfo.h"
#include <QDebug>

FitsImageStatisiticInfo::FitsImageStatisiticInfo(vtkwindow_new *v, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FitsImageStatisiticInfo)
{
    ui->setupUi(this);

    vtkwin=v;

    qDebug()<<vtkwin->getWindowName();

}

FitsImageStatisiticInfo::~FitsImageStatisiticInfo()
{
    delete ui;
}

void FitsImageStatisiticInfo::setFilename()
{
    ui->filenameLabel->setText(vtkwin->getWindowName());
}

void FitsImageStatisiticInfo::setGalaptic(double l, double b)
{
    ui->lGalapticLabel->setText( QString::number(l) );
    ui->bGalapticLabel->setText( QString::number(b) );
}

void FitsImageStatisiticInfo::setEcliptic(double lat, double lon)
{
    ui->latEclipticLabel->setText( QString::number(lat) );
    ui->longEclipticLabel->setText( QString::number(lon) );
}

void FitsImageStatisiticInfo::setFk5(double ra, double dec)
{
    ui->raFk5Label->setText( QString::number(ra) );
    ui->decFk5Label->setText( QString::number(dec) );
}


void FitsImageStatisiticInfo::setImage(double x, double y)
{
    ui->xLabel->setText( QString::number(x) );
    ui->yLabel->setText( QString::number(y) );
}
