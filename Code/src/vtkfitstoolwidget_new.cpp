#include "vtkfitstoolwidget_new.h"
#include "ui_vtkfitstoolwidget_new.h"
#include <QFileInfo>
#include <QDebug>
#include "vtkProperty.h"

vtkfitstoolwidget_new::vtkfitstoolwidget_new(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::vtkfitstoolwidget_new)
{
    ui->setupUi(this);
    ui->deletePushButton->setStyleSheet("background-color: red");
}

vtkfitstoolwidget_new::~vtkfitstoolwidget_new()
{
    delete ui;
}

void vtkfitstoolwidget_new::addLayer(vtkfitstoolwidgetobject *o)
{

    if (o->getType() == 0)
    {
        QTreeWidgetItem *treeItem=addTreeRoot(o->getName());
        o->setTreeWidgetItem( treeItem );
    }

    if (o->getType() == 1)
    {

        double r=o->getActor()->GetProperty()->GetColor()[0]*255;
        double g=o->getActor()->GetProperty()->GetColor()[1]*255;
        double b=o->getActor()->GetProperty()->GetColor()[2]*255;

        QBrush brush (QColor(r,g,b));
        addTreeChild(o->getParent()->getTreeWidgetItem(),  o->getName(),brush);
    }


    layerList.append(o);
}

QTreeWidgetItem* vtkfitstoolwidget_new::addTreeRoot(QString name)
{
    QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->layerTreeWidget);
    treeItem->setText(1, name);

    return treeItem;
}

void vtkfitstoolwidget_new::addTreeChild(QTreeWidgetItem *parent, QString name, QBrush brush)
{

    QTreeWidgetItem *treeItem = new QTreeWidgetItem();



    treeItem->setBackground(0,brush);

    treeItem->setText(1, name);

    parent->addChild(treeItem);
}



void vtkfitstoolwidget_new::setWavelength(QString w)
{
    wavelength=w;
}

void vtkfitstoolwidget_new::on_layerTreeWidget_itemSelectionChanged()
{
    ui->layerTreeWidget->selectionModel()->select(ui->layerTreeWidget->model()->index(ui->layerTreeWidget ->indexOfTopLevelItem(ui->layerTreeWidget->currentItem()),0),QItemSelectionModel::Select);
    int pos = ui->layerTreeWidget->currentIndex().row()+ qAbs(ui->layerTreeWidget->indexOfTopLevelItem ( ui->layerTreeWidget->currentItem()));
    ui->nameLineEdit->setText( layerList.at(pos)->getName() );
    ui->wavelenghtLineEdit->setText( layerList.at(pos)->getWavelength() );

}

void vtkfitstoolwidget_new::on_savePushButton_clicked()
{
    int pos = ui->layerTreeWidget->currentIndex().row()+ qAbs(ui->layerTreeWidget->indexOfTopLevelItem ( ui->layerTreeWidget->currentItem()));
    qDebug()<<"pos: "<<pos;
    qDebug()<<"type: "<<layerList.at(pos)->getType();
    qDebug()<<"name: "<<layerList.at(pos)->getName();

    layerList.at(pos)->setName(ui->nameLineEdit->text());
    layerList.at(pos)->setWavelength(ui->wavelenghtLineEdit->text());
}
