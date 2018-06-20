#include "selectedsourcefieldsselect.h"
#include "ui_selectedsourcefieldsselect.h"
#include "qsignalmapper.h"
#include <QDebug>
#include <QCheckBox>

#include "viewselectedsourcedataset.h"

selectedSourceFieldsSelect::selectedSourceFieldsSelect(vtkwindow_new *v, QList<QListWidgetItem *> sel, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::selectedSourceFieldsSelect)
{
    ui->setupUi(this);
    vtkwin=v;

    selectedSources=sel;


    //Solo per la prima?
    table=vtkwin->getEllipseList().value(selectedSources.at(0)->text())->getTable();

    QHeaderView* header = ui->fieldListTableWidget->horizontalHeader();
    header->setVisible(true);
    header->sectionResizeMode(QHeaderView::Stretch);


    int row;
    QSignalMapper* mapper = new QSignalMapper(this);

    for (int i=0;i<table->getNumberOfColumns();i++)
    {


        row= ui->fieldListTableWidget->model()->rowCount();


        ui->fieldListTableWidget->insertRow(row);
        QCheckBox  *cb1= new QCheckBox();
        cb1->setChecked(true);
        ui->fieldListTableWidget->setCellWidget(row,0,cb1);
        connect(cb1, SIGNAL(stateChanged(int)),mapper, SLOT(map()));
        mapper->setMapping(cb1, row);

        checkboxList.append(cb1);

        selectedFields.insert(i,QString::fromStdString(table->getColName(i)));

        QTableWidgetItem *item_1 = new QTableWidgetItem();
        item_1->setText(QString::fromStdString(table->getColName(i)));
        ui->fieldListTableWidget->setItem(row,1,item_1);

        connect(mapper, SIGNAL(mapped(int)), this, SLOT(checkboxClicked(int)));
    }


}

void selectedSourceFieldsSelect::checkboxClicked(int cb)
{
    if(checkboxList.at(cb)->isChecked())
        selectedFields.insert(cb,QString::fromStdString(table->getColName(cb)));
    else
        selectedFields.remove(cb);
}

selectedSourceFieldsSelect::~selectedSourceFieldsSelect()
{
    delete ui;
}

void selectedSourceFieldsSelect::on_selectAllButton_clicked()
{
    for (int i=0;i<checkboxList.size();i++)
    {
        checkboxList.at(i)->setChecked(true);
        selectedFields.insert(i,QString::fromStdString(table->getColName(i)));

    }
}

void selectedSourceFieldsSelect::on_deselectAllButton_clicked()
{

    for (int i=0;i<checkboxList.size();i++)
    {
        checkboxList.at(i)->setChecked(false);
        selectedFields.remove(i);
    }
}

void selectedSourceFieldsSelect::on_okButton_clicked()
{

    ViewSelectedSourceDataset *selDataset =new ViewSelectedSourceDataset(vtkwin,selectedFields,selectedSources);
    this->close();

    selDataset->show();
}
