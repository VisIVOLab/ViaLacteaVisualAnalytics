#include "vtktoolswidget.h"
#include "ui_vtktoolswidget.h"
#include "qdebug.h"

#include "vtkSmartPointer.h"

#include <vtkXYPlotActor.h>
#include <vtkDataSet.h>
#include "vtkProperty2D.h"




vtktoolswidget::vtktoolswidget(vtkwindow_new  *v,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::vtktoolswidget)
{

    ui->setupUi(this);
    vtkwin=v;

    qDebug()<<"#col: "<<vtkwin->table->getNumberOfColumns();

    for (unsigned int i =0; i < vtkwin->table->getNumberOfColumns();i++)
    {
        QString field =  QString(vtkwin->table->getColName(i).c_str());
        ui->scalarComboBox->addItem(field);
    }
}



vtktoolswidget::~vtktoolswidget()
{
    delete ui;
}


void vtktoolswidget::plotHistogram(int i)
{

    QVector<double> y(vtkwin->table->getbinNumber());
    qDebug()<<"y: "<<y;

    y=vtkwin->table->getHistogram(i);
    qDebug()<<"y post: "<<y;


    QVector<double> x(vtkwin->table->getbinNumber());
    qDebug()<<"x "<<vtkwin->table->getbinNumber();

    x=vtkwin->table->getHistogramValue(i);

    range=new float[3];
    qDebug()<<"dddd";

    vtkwin->table->getRange(i,range);
    qDebug()<<"d2";
/*
    // create graph and assign data to it:
    ui->histogramWidget->addGraph();
    ui->histogramWidget->graph(0)->setData(x, y);
    // give the axes some labels:
    // ui->histogramWidget->xAxis->setTickLabels(false);
    ui->histogramWidget->yAxis->setTickLabels(false);


    // set axes ranges, so we see all data:
    ui->histogramWidget->xAxis->setRange(range[0],range[1]);
    ui->histogramWidget->yAxis->setRange(0, range[2]);
    ui->histogramWidget->replot();

*/


}

void vtktoolswidget::drawLine(double from, double to)
{

    if(fromLine!=0 && toLine!=0 )
    {

        ui->histogramWidget->removeItem(fromLine);
        ui->histogramWidget->removeItem(toLine);
    }


    fromLine = new QCPItemLine(ui->histogramWidget);

    toLine = new QCPItemLine(ui->histogramWidget);



    QPen pen;
    pen.setColor( Qt::red );

    fromLine->setPen( pen );
    pen.setColor( Qt::green );
    toLine->setPen( pen);

    fromLine->start->setCoords(from, 0);
    fromLine->end->setCoords(from, range[2]);

    toLine->start->setCoords(to, 0);
    toLine->end->setCoords(to, range[2]);

    ui->histogramWidget->addItem(fromLine);
    ui->histogramWidget->addItem(toLine);

    ui->histogramWidget->replot();


    vtkwin->pp->setLookupTable(from,to);

}

void vtktoolswidget::on_ShowBoxCheckBox_clicked(bool checked)
{
    vtkwin->showBox(checked);
}

void vtktoolswidget::on_ShowAxesCheckBox_clicked(bool checked)
{
    vtkwin->showAxes(checked);

}

void vtktoolswidget::on_activateLutCheckBox_clicked(bool checked)
{
    ui->lutLabel->setEnabled(checked);
    ui->lutComboBox->setEnabled(checked);
    ui->scalarLabel->setEnabled(checked);
    ui->scalarComboBox->setEnabled(checked);

    std::string palette="AllWhite";
    if (checked)
    {
        palette=ui->lutComboBox->currentText().toStdString().c_str();
    }

    changeLut(palette);
}


void vtktoolswidget::changeLut(std::string palette)
{
    vtkwin->changePalette(palette);
}

void vtktoolswidget::changeScalar(std::string scalar)
{
    vtkwin->changeScalar(scalar);
}

void vtktoolswidget::on_lutComboBox_currentIndexChanged(const QString &arg1)
{
    changeLut(ui->lutComboBox->currentText().toStdString().c_str());
}

void vtktoolswidget::on_ShowColorbarCheckBox_clicked(bool checked)
{
    vtkwin->showColorbar(checked);
}

void vtktoolswidget::on_scalarComboBox_currentIndexChanged(const QString &arg1)
{
    qDebug()<<"on_scalarComboBox_currentIndexChanged: "<<ui->scalarComboBox->currentText().toStdString().c_str();
  //  changeScalar(ui->scalarComboBox->currentText().toStdString().c_str());
}

void vtktoolswidget::on_scalarComboBox_currentIndexChanged(int index)
{

    qDebug()<<"bbbb";


   plotHistogram(index);
    qDebug()<<"range[0]: "<<range[0];
    qDebug()<<"range[1]: "<<range[1];


    /*
    drawLine(range[0],range[1]);

    ui->fromValue->setText(QString::number(range[0]));
    ui->toValue->setText(QString::number(range[1]));



    ui->fromSlider->setMinimum(range[0]);
    ui->fromSlider->setMaximum(range[1]);

    ui->toSlider->setMinimum(range[0]);
    ui->toSlider->setMaximum(range[1]);

    ui->fromSlider->setValue(range[0]);
    ui->toSlider->setValue(range[1]);
*/

}

void vtktoolswidget::on_fromSlider_sliderMoved(int position)
{


    ui->fromValue->setText(QString::number(position));

    drawLine(position,ui->toValue->text().toDouble());


}

void vtktoolswidget::on_toSlider_sliderMoved(int position)
{


    ui->toValue->setText(QString::number(position));

    drawLine(ui->fromValue->text().toDouble(),position);

}

void vtktoolswidget::on_fromValue_textChanged(const QString &arg1)
{
    ui->fromSlider->setValue(arg1.toDouble());

    drawLine(ui->fromValue->text().toDouble(),ui->toValue->text().toDouble());

}

void vtktoolswidget::on_toValue_textChanged(const QString &arg1)
{
    ui->toSlider->setValue(arg1.toDouble());

    drawLine(ui->fromValue->text().toDouble(),ui->toValue->text().toDouble());
}


void vtktoolswidget::on_scaleCheckBox_clicked(bool checked)
{
    vtkwin->scale(checked);
}

void vtktoolswidget::on_cameraLeft_clicked()
{
    vtkwin->setCameraAzimuth(-90);
}

void vtktoolswidget::on_cameraBack_clicked()
{
    vtkwin->setCameraAzimuth(-180);

}

void vtktoolswidget::on_cameraRight_clicked()
{
    vtkwin->setCameraAzimuth(90);

}

void vtktoolswidget::on_frontCamera_clicked()
{
    vtkwin->setCameraAzimuth(0);

}

void vtktoolswidget::on_topCamera_clicked()
{
    vtkwin->setCameraElevation(90);

}

void vtktoolswidget::on_bottomCamera_clicked()
{
    vtkwin->setCameraElevation(-90);

}

void vtktoolswidget::on_gridCheckBox_clicked(bool checked)
{

    vtkwin->showGrid(checked);
}

void vtktoolswidget::on_scalarComboBox_activated(const QString &arg1)
{

}

void vtktoolswidget::on_scaleCheckBox_clicked()
{

}

void vtktoolswidget::on_ShowColorbarCheckBox_clicked()
{

}
