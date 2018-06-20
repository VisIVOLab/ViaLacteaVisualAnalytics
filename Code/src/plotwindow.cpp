#include "plotwindow.h"
#include "ui_plotwindow.h"

PlotWindow::PlotWindow(vtkwindow_new *v, QList<QListWidgetItem*> selItems, int id, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlotWindow)
{
    ui->setupUi(this);

    QString title="Plot #"+QString::number(id);

    this->setWindowTitle(title);

    ui->customPlot->addGraph();
    // ui->customPlot->xAxis2->setVisible(true);
    // ui->customPlot->yAxis2->setVisible(true);



    vtkwin=v;
    selectedItems=selItems;

    table =vtkwin->getEllipseList().value(selItems.at(0)->text())->getTable();

    for(int i=0;i<table->getNumberOfColumns();i++)
    {
        ui->xComboBox->addItem(QString::fromStdString(table->getColName(i)));
        ui->yComboBox->addItem(QString::fromStdString(table->getColName(i)));
    }


}

PlotWindow::~PlotWindow()
{
    delete ui;
}

void PlotWindow::on_plotButton_clicked()
{
    qDebug()<<"index: "<<ui->xComboBox->currentIndex();
    QVector<double> x(selectedItems.size()), y(selectedItems.size());


    int row;
    for (int i=0; i<selectedItems.size(); i++)
    {

        row = vtkwin->getEllipseList().value(selectedItems.at(i)->text())->getRowInTable();

        x[i] = atof(table->getTableData()[ui->xComboBox->currentIndex()][row].c_str());
        y[i] = atof(table->getTableData()[ui->yComboBox->currentIndex()][row].c_str());


    }




    // create graph and assign data to it:
    ui->customPlot->graph(0)->setData(x, y);

    if(ui->logxCheckBox->isChecked())
    {
        ui->customPlot->xAxis->setScaleType(QCPAxis::stLogarithmic);
        ui->customPlot->xAxis->setLabel("Log "+ui->xComboBox->currentText() );
    }
    else
    {
        ui->customPlot->xAxis->setScaleType(QCPAxis::stLinear);
        ui->customPlot->xAxis->setLabel(ui->xComboBox->currentText() );
    }
    if(ui->logyCheckBox->isChecked())
    {
        ui->customPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
        ui->customPlot->yAxis->setLabel("Log "+ui->yComboBox->currentText());

    }
    else
    {
        ui->customPlot->yAxis->setScaleType(QCPAxis::stLinear);
        ui->customPlot->yAxis->setLabel(ui->yComboBox->currentText());

    }


    ui->customPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
    ui->customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));

    ui->customPlot->graph(0)->rescaleAxes();

    //ui->customPlot->xAxis->setRange(x.at(x.first()), x.at(x.last()));
    // ui->customPlot->yAxis->setRange(y.at(y.first()), y.at(y.last()));
    ui->customPlot->replot();

}
