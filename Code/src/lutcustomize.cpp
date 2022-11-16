#include "lutcustomize.h"
#include "ui_lutcustomize.h"

#include "ui_vtkwindow_new.h"
#include "vispoint.h"
#include "vtkDoubleArray.h"
#include "vtkextracthistogram.h"
#include "vtkTable.h"

#include "vtkImageHistogram.h"


LutCustomize::LutCustomize(vtkwindow_new *v, QWidget *parent)
    : QWidget(parent), ui(new Ui::LutCustomize)
{
    ui->setupUi(this);
    vtkwin = v;
    isPoint3D=false;
    ui->histogramWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);
}

void LutCustomize::configurePoint3D()
{
    isPoint3D=true;
    setRange();

    ui->fromSlider->setMinimum(range[0]);
    ui->fromSlider->setMaximum(range[1]);

    ui->toSlider->setMinimum(range[0]);
    ui->toSlider->setMaximum(range[1]);

    ui->fromSlider->setValue(range[0]);
    ui->toSlider->setValue(range[1]);
    plotHistogram();
}

void LutCustomize::configureFitsImage()
{
    setRange();
    ui->fromSlider->setMinimum(range[0]);
    ui->fromSlider->setMaximum(range[1]);

    ui->toSlider->setMinimum(range[0]);
    ui->toSlider->setMaximum(range[1]);

    ui->fromSlider->setValue(range[0]);
    ui->toSlider->setValue(range[1]);
    ui->fromValue->setText(QString::number(range[0]));
    ui->toValue->setText(QString::number(range[1]));
    plotHistogram();
}

LutCustomize::~LutCustomize()
{
    delete ui;
}

void LutCustomize::setRange()
{
    range = new double[2];
    if (isPoint3D)
    {
        vtkwin->pp->getPolyData()
                ->GetPointData()
                ->GetScalars(vtkwin->ui->scalarComboBox->currentText().toStdString().c_str())
                ->GetRange(range);

        ui->fromValue->setText(QString::number(vtkwin->pp->actualFrom));
        ui->toValue->setText(QString::number(vtkwin->pp->actualTo));
    }
    else
    {
        range[0]=vtkwin->getFitsImage()->GetMin();
        range[1]=vtkwin->getFitsImage()->GetMax();
    }
}

void LutCustomize::plotHistogram()
{
    vtkSmartPointer<vtkExtractHistogram> extraction = vtkSmartPointer<vtkExtractHistogram>::New();
    int numberOfBins=-1;
    extraction->UseCustomBinRangesOn();

    if(isPoint3D)
    {
        extraction->SetInputData(vtkwin->pp->getPolyData());
        numberOfBins = vtkwin->vispoint->getOrigin()->getbinNumber();

    }else
    {   extraction->SetInputData(vtkwin->getFitsImage()->GetOutputDataObject(0));
        numberOfBins=(vtkwin->getFitsImage()->GetNaxes(0)*vtkwin->getFitsImage()->GetNaxes(1))/10;
    }
    extraction->SetBinCount(numberOfBins);
    extraction->SetCustomBinRanges(range);
    extraction->Update();
    vtkTable *const histogram = extraction->GetOutput();
    vtkDoubleArray *const bin_extents =
            vtkDoubleArray::SafeDownCast(histogram->GetRowData()->GetArray("bin_extents"));
    vtkIntArray *const bin_values =
            vtkIntArray::SafeDownCast(histogram->GetRowData()->GetArray((int)1));
    y_range = new double[2];
    bin_values->GetRange(y_range);
    QVector<double> x(numberOfBins);
    QVector<double> y(numberOfBins);
    vtkIdType nPoints = bin_extents->GetDataSize();
    int cnt = 0;
    for (vtkIdType i = 0; i < nPoints; ++i) {
        x.push_back(bin_extents->GetValue(i));
        y.push_back(bin_values->GetValue(i));
        cnt++;
    }
    // create graph and assign data to it:
    ui->histogramWidget->addGraph();
    ui->histogramWidget->graph(0)->setData(x, y);
    if (vtkwin->getSelectedScale() == "Log") {
        ui->histogramWidget->xAxis->setScaleType(QCPAxis::stLogarithmic);
        QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
        logTicker->setLogBase(10);
        ui->histogramWidget->xAxis->setTicker(logTicker);
    }

    ui->histogramWidget->yAxis->setTickLabels(false);
    // set axes ranges, so we see all data:
    ui->histogramWidget->xAxis->setRange(range[0]-(range[0]*0.1), range[1]*1.1f);
    ui->histogramWidget->yAxis->setRange(y_range[0], y_range[1]);
    drawLine(range[0],range[1]);
    ui->histogramWidget->replot();

}

void LutCustomize::drawLine(double from, double to)
{
    if (fromLine != 0 && toLine != 0) {
        ui->histogramWidget->removeItem(fromLine);
        ui->histogramWidget->removeItem(toLine);
    }

    fromLine = new QCPItemLine(ui->histogramWidget);
    toLine = new QCPItemLine(ui->histogramWidget);

    QPen pen;
    pen.setColor(Qt::red);

    fromLine->setPen(pen);
    pen.setColor(Qt::green);
    toLine->setPen(pen);

    fromLine->start->setCoords(from, 0);
    fromLine->end->setCoords(from, y_range[1]);

    toLine->start->setCoords(to, 0);
    toLine->end->setCoords(to, y_range[1]);
    ui->histogramWidget->replot();
}

void LutCustomize::on_cancelPushButton_clicked()
{
    this->close();
}

void LutCustomize::closeEvent(QCloseEvent *event)
{
    vtkwin->activateWindow();
    vtkwin->show();
}

void LutCustomize::on_ShowColorbarCheckBox_clicked(bool checked)
{
    if(isPoint3D)
        vtkwin->showColorbar(checked);
    else
        vtkwin->showColorbarFits(checked);
}

void LutCustomize::on_fromSlider_sliderMoved(int position)
{
    ui->fromValue->setText(QString::number(position));
    drawLine(position, ui->toValue->text().toDouble());
}

void LutCustomize::on_toSlider_sliderMoved(int position)
{
    ui->toValue->setText(QString::number(position));
    drawLine(ui->fromValue->text().toDouble(), position);
}

void LutCustomize::on_okPushButton_clicked()
{
    if(isPoint3D)
    {
        vtkwin->pp->setLookupTable(ui->fromValue->text().toDouble(), ui->toValue->text().toDouble());
        // ES
        if (vtkwin->ui->glyphShapeComboBox->isEnabled()) {
            vtkwin->getGlyphActor()->GetMapper()->SetLookupTable(vtkwin->pp->getLookupTable());
            vtkwin->getGlyphActor()->GetMapper()->SetScalarRange(
                        vtkwin->pp->getLookupTable()->GetRange());
        }
    }
    // End ES
    this->close();
}

void LutCustomize::on_lutComboBox_activated(const QString &arg1)
{
    if(isPoint3D)
    {
        vtkwin->ui->lut3dComboBox->setCurrentText(arg1);
        vtkwin->on_lut3dComboBox_activated(arg1);
    }
    else
    {
        vtkwin->ui->lutComboBox->setCurrentText(arg1);
        vtkwin->on_lutComboBox_activated(arg1);
    }
}

void LutCustomize::on_scalingComboBox_activated(const QString &arg1)
{
    if(isPoint3D)
    {
        if (arg1=="Linear")
            vtkwin->ui->linear3dRadioButton->setChecked(true);
        else
            vtkwin->ui->log3dRadioButton->setChecked(true);


    }
    else
    {
        if (arg1=="Linear")
            vtkwin->ui->linearadioButton->setChecked(true);
        else
            vtkwin->ui->logRadioButton->setChecked(true);

        //vtkwin->on_lutComboBox_activated(arg1);
    }
}


