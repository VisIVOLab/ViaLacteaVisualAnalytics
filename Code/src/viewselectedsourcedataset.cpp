#include "viewselectedsourcedataset.h"
#include "ui_viewselectedsourcedataset.h"
#include <QDebug>

ViewSelectedSourceDataset::ViewSelectedSourceDataset(vtkwindow_new *v, QHash <int,QString> selFields, QList<QListWidgetItem*> selSources, QWidget *parent) :
    QWidget(parent),ui(new Ui::ViewSelectedSourceDataset)
{
    ui->setupUi(this);
    selectedFields=selFields;
    selectedSources=selSources;
    vtkwin=v;

    table=vtkwin->getEllipseList().value(selectedSources.at(0)->text())->getTable();

    QHeaderView* header = ui->datasetTableWidget->horizontalHeader();
    header->setVisible(true);

    ui->datasetTableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    ui->datasetTableWidget->setColumnCount( selFields.size() );

    for (int j=0;j<selFields.size();j++)
    {
        ui->datasetTableWidget->setHorizontalHeaderItem(j, new QTableWidgetItem(selFields.values().at(j)) );
    }

    int row;
    for (int i=0;i<selSources.size();i++)
    {
        row = vtkwin->getEllipseList().value(selectedSources.at(i)->text())->getRowInTable();
        ui->datasetTableWidget->insertRow(i);

        for (int j=0;j<selFields.size();j++)
        {
            QTableWidgetItem *item=new QTableWidgetItem (QString::fromStdString(table->getTableData()[selFields.keys()[j]][row]) );
            ui->datasetTableWidget->setItem(i,j,item);
        }
    }

    ui->datasetTableWidget->resizeColumnsToContents();


}

ViewSelectedSourceDataset::~ViewSelectedSourceDataset()
{
    delete ui;
}
