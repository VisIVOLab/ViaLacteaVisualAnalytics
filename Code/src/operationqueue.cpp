#include "operationqueue.h"
#include "ui_operationqueue.h"
#include "qdebug.h"

OperationQueue::OperationQueue(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OperationQueue)
{
    ui->setupUi(this);

    QHeaderView* header = ui->tableWidget->horizontalHeader();
    header->setVisible(true);
    header->sectionResizeMode(QHeaderView::Stretch);
}

OperationQueue::~OperationQueue()
{
    delete ui;
}

int OperationQueue::addOperation(QString name)
{

    int id= ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(id);

    QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(id));
    QTableWidgetItem* nameItem = new QTableWidgetItem(name);
    QTableWidgetItem* statusItem = new QTableWidgetItem("in progress...");
    ui->tableWidget->setItem(id, 0, idItem);
    ui->tableWidget->setItem(id, 1, nameItem);
    ui->tableWidget->setItem(id, 2, statusItem);

    QApplication::processEvents();


    return id;

}

void OperationQueue::editOperation(int id, QString status)
{

    QTableWidgetItem* statusItem = new QTableWidgetItem(status);

    ui->tableWidget->setItem(id,2, statusItem);
    QApplication::processEvents();

}
