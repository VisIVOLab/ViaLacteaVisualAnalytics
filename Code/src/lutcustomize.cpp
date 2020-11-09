#include "lutcustomize.h"
#include "ui_lutcustomize.h"
#include "vtkextracthistogram.h"
#include "vispoint.h"
#include "vtkTable.h"
#include "vtkDoubleArray.h"

LutCustomize::LutCustomize(vtkwindow_new *v, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LutCustomize)
{
    ui->setupUi(this);
    // this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);

    vtkwin=v;

    setRange();
    plotHistogram();

    ui->fromSlider->setMinimum(range[0]);
    ui->fromSlider->setMaximum(range[1]);

    ui->toSlider->setMinimum(range[0]);
    ui->toSlider->setMaximum(range[1]);

    ui->fromSlider->setValue(range[0]);
    ui->toSlider->setValue(range[1]);


}

LutCustomize::~LutCustomize()
{
    delete ui;
}


void LutCustomize::setRange()
{
    range=new double[2];
    vtkwin->pp->getPolyData()->GetPointData()->GetScalars(vtkwin->ui->scalarComboBox->currentText().toStdString().c_str())->GetRange(range);

    ui->fromValue->setText( QString::number(  vtkwin->pp->actualFrom ) );
    ui->toValue->setText(  QString::number( vtkwin->pp->actualTo ) );
    /*
   // drawLine(vtkwin->pp->actualFrom,vtkwin->pp->actualTo);
    ui->fromValue->setText( QString::number( range[0] ) );
    ui->toValue->setText( QString::number( range[1] ) );
*/
}

void LutCustomize::plotHistogram()
{
    qDebug()<<"plotHistogram";
    vtkwin->pp->getPolyData()->GetPointData()->GetScalars(vtkwin->ui->scalarComboBox->currentText().toStdString().c_str())->GetRange(range);
    qDebug()<<"get range";
    vtkSmartPointer<vtkExtractHistogram> extraction = vtkSmartPointer<vtkExtractHistogram>::New();
    extraction->SetInputData( vtkwin->pp->getPolyData() );
    extraction->UseCustomBinRangesOn();
    extraction->SetBinCount(vtkwin->vispoint->getOrigin()->getbinNumber());
    extraction->SetCustomBinRanges(range);
    extraction->Update();
    qDebug()<<"set extraction";
    vtkTable* const histogram = extraction->GetOutput();
    qDebug()<<"create histogram";
    vtkDoubleArray* const bin_extents = vtkDoubleArray::SafeDownCast(histogram->GetRowData()->GetArray("bin_extents"));
    vtkIntArray* const bin_values = vtkIntArray::SafeDownCast(histogram->GetRowData()->GetArray((int)1));
    qDebug()<<"create bin "<<QString::number(bin_extents->GetSize())<<" "<<QString::number(bin_values->GetSize());
    y_range=new double[2];
    bin_values->GetRange(y_range);
    qDebug()<<"set range";
    QVector<double> x(vtkwin->vispoint->getOrigin()->getbinNumber());
    QVector<double> y(vtkwin->vispoint->getOrigin()->getbinNumber());
    qDebug()<<"set x y "<<QString::number(x.size())<<" "<<QString::number(y.size());
    vtkIdType nPoints = bin_extents->GetDataSize();
    qDebug()<<"nPoints "<<QString::number(nPoints);
    int cnt=0;
    for (vtkIdType i = 0; i < nPoints; ++i)
    {
        qDebug()<<"i "<<QString::number(i);
        qDebug()<<QString::number(bin_extents->GetValue(i));
        //x[cnt]=bin_extents->GetValue(i);
        x.push_back(bin_extents->GetValue(i));
        qDebug()<<QString::number(bin_values->GetValue(i));
        //y[cnt]=bin_values->GetValue(i);
        y.push_back(bin_values->GetValue(i));
        cnt++;
    }
    qDebug()<<"calculated histogram";

    // create graph and assign data to it:
    ui->histogramWidget->addGraph();
    ui->histogramWidget->graph(0)->setData(x, y);
    if (vtkwin->getSelectedScale()=="Log")
    {
        ui->histogramWidget->xAxis->setScaleType(QCPAxis::stLogarithmic);
        ui->histogramWidget->xAxis->setScaleLogBase(10);
    }
    // give the axes some labels:
    // ui->histogramWidget->xAxis->setTickLabels(false);
    ui->histogramWidget->yAxis->setTickLabels(false);


    // set axes ranges, so we see all data:
    ui->histogramWidget->xAxis->setRange(range[0],range[1]);
    ui->histogramWidget->yAxis->setRange(y_range[0], y_range[1]);
    ui->histogramWidget->replot();



}

void LutCustomize::drawLine(double from, double to)
{


    if(fromLine!=0 && toLine!=0 )
    {

        ui->histogramWidget->removeItem(fromLine);
        ui->histogramWidget->removeItem(toLine);
    }


    fromLine = new QCPItemLine(ui->histogramWidget);
    toLine = new QCPItemLine(ui->histogramWidget);

    double *range=new double[2];

    QPen pen;
    pen.setColor( Qt::red );

    fromLine->setPen( pen );
    pen.setColor( Qt::green );
    toLine->setPen( pen);

    fromLine->start->setCoords(from, 0);
    fromLine->end->setCoords(from, y_range[1]);

    toLine->start->setCoords(to, 0);
    toLine->end->setCoords(to, y_range[1]);

    ui->histogramWidget->addItem(fromLine);
    ui->histogramWidget->addItem(toLine);

    ui->histogramWidget->replot();



}


void LutCustomize::on_cancelPushButton_clicked()
{
    this->close();
}

void LutCustomize::closeEvent ( QCloseEvent * event )
{
    vtkwin->activateWindow();
    vtkwin->show();

}

void LutCustomize::on_ShowColorbarCheckBox_clicked(bool checked)
{
    vtkwin->showColorbar(checked);
}

void LutCustomize::on_fromSlider_sliderMoved(int position)
{
    ui->fromValue->setText(QString::number(position));
    drawLine(position,ui->toValue->text().toDouble());
}



void LutCustomize::on_toSlider_sliderMoved(int position)
{
    ui->toValue->setText(QString::number(position));
    drawLine(ui->fromValue->text().toDouble(),position);
}

void LutCustomize::on_okPushButton_clicked()
{
    vtkwin->pp->setLookupTable(ui->fromValue->text().toDouble(), ui->toValue->text().toDouble());
    //ES
    if(vtkwin->ui->glyphShapeComboBox->isEnabled()){
        qDebug()<<"Re draw glyph";
        vtkwin->getGlyphActor()->GetMapper()->SetLookupTable(vtkwin->pp->getLookupTable());
        vtkwin->getGlyphActor()->GetMapper()->SetScalarRange(vtkwin->pp->getLookupTable()->GetRange());
    }
    //End ES
    this->close();
}
