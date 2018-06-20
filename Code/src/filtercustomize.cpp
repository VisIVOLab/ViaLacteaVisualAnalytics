#include "filtercustomize.h"
#include "ui_filtercustomize.h"
#include "vispoint.h"

FilterCustomize::FilterCustomize(vtkwindow_new *v, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilterCustomize)
{
    ui->setupUi(this);

    vtkwin=v;
    //add lut on main window FV
    for (unsigned int i =0; i < vtkwin->vispoint->getOrigin()->getNumberOfColumns();i++)
    {
        QString field =  QString(vtkwin->vispoint->getOrigin()->getColName(i).c_str());
        ui->fieldComboBox->addItem(field);
    }


    setRangeId(0);
}

FilterCustomize::~FilterCustomize()
{
    delete ui;
}

void FilterCustomize::on_fieldComboBox_activated(const QString &arg1)
{
    int id=vtkwin->vispoint->getOrigin()->getColId(arg1.toStdString());
    setRangeId(id);

}

void FilterCustomize::setRangeId(int id)
{

    float range[3] ={};// = new float[3];
    vtkwin->vispoint->getOrigin()->getRange(id,range);
    ui->minValueLineEdit->setText(QString::number(range[0]));
    ui->maxValueLineEdit->setText(QString::number(range[1]));
}

