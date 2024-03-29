#include "lutcustomize.h"
#include "ui_lutcustomize.h"

#include "ui_vtkwindow_new.h"
#include "ui_vtkwindowcube.h"

#include "vispoint.h"
#include "vtkDoubleArray.h"
#include "vtkextracthistogram.h"
#include "vtkTable.h"
#include "vtkwindowcube.h"

LutCustomize::LutCustomize(vtkWindowCube *v, QWidget *parent)
    : QWidget(parent), ui(new Ui::LutCustomize)
{
    ui->setupUi(this);
    this->setWindowTitle("LUT Customizer");
    vtkwincube = v;
    isPoint3D = false;
    isFits2D = false;
    isFits3D = false;
    isMoment = false;
    ui->histogramWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);
    ui->histogramWidget->addGraph();
}

LutCustomize::LutCustomize(vtkwindow_new *v, QWidget *parent)
    : QWidget(parent), ui(new Ui::LutCustomize)
{
    ui->setupUi(this);
    this->setWindowTitle("LUT Customizer");
    vtkwin = v;
    isPoint3D = false;
    isFits2D = false;
    isFits3D = false;
    isMoment = false;
    ui->histogramWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);
    ui->histogramWidget->addGraph();
}

void LutCustomize::configurePoint3D()
{
    isPoint3D = true;
    setRange();
    plotHistogram();

    ui->fromSpinBox->setMinimum(range[0]);
    ui->fromSpinBox->setMaximum(range[1]);
    ui->toSpinBox->setMinimum(range[0]);
    ui->toSpinBox->setMaximum(range[1]);
    ui->fromSpinBox->setValue(range[0]);
    ui->toSpinBox->setValue(range[1]);
}

void LutCustomize::configureFitsImage()
{
    isFits2D = true;
    setRange();
    plotHistogram();

    ui->fromSpinBox->setMinimum(range[0]);
    ui->fromSpinBox->setMaximum(range[1]);
    ui->toSpinBox->setMinimum(range[0]);
    ui->toSpinBox->setMaximum(range[1]);
    ui->fromSpinBox->setValue(range[0]);
    ui->toSpinBox->setValue(range[1]);
}

void LutCustomize::configureFits3D()
{
    isFits3D = true;
    isMoment = false;
    setRange();
    plotHistogram();

    ui->fromSpinBox->setMinimum(range[0]);
    ui->fromSpinBox->setMaximum(range[1]);
    ui->toSpinBox->setMinimum(range[0]);
    ui->toSpinBox->setMaximum(range[1]);
    ui->fromSpinBox->setValue(range[0]);
    ui->toSpinBox->setValue(range[1]);
}

void LutCustomize::configureMoment()
{
    isFits3D = false;
    isMoment = true;
    setRange();
    plotHistogram();

    ui->fromSpinBox->setMinimum(range[0]);
    ui->fromSpinBox->setMaximum(range[1]);
    ui->toSpinBox->setMinimum(range[0]);
    ui->toSpinBox->setMaximum(range[1]);
    ui->fromSpinBox->setValue(range[0]);
    ui->toSpinBox->setValue(range[1]);
}

void LutCustomize::setScaling(QString scaling)
{
    ui->scalingComboBox->setCurrentText(scaling);
}
void LutCustomize::setLut(QString lut)
{
    ui->lutComboBox->setCurrentText(lut);
}

LutCustomize::~LutCustomize()
{
    delete ui;
}

void LutCustomize::setRange()
{
    if (isPoint3D) {
        vtkwin->pp->getPolyData()
                ->GetPointData()
                ->GetScalars(vtkwin->ui->scalarComboBox->currentText().toStdString().c_str())
                ->GetRange(range);

        range[0] = vtkwin->pp->actualFrom;
        range[1] = vtkwin->pp->actualTo;
    } else if (isFits2D) {
        int pos = 0;
        if (vtkwin->ui->listWidget->selectionModel()->selectedRows().count() != 0
            && vtkwin->getLayerListImages()
                            .at(vtkwin->ui->listWidget->selectionModel()
                                        ->selectedRows()
                                        .at(0)
                                        .row())
                            ->getType()
                    == 0) {
            pos = vtkwin->ui->listWidget->selectionModel()->selectedRows().at(0).row();
        }

        range[0] = vtkwin->getLayerListImages().at(pos)->getFits()->GetMin();
        range[1] = vtkwin->getLayerListImages().at(pos)->getFits()->GetMax();
    } else if (isFits3D) {
        range[0] = vtkwincube->readerSlice->GetValueRange()[0];
        range[1] = vtkwincube->readerSlice->GetValueRange()[1];
    } else if (isMoment) {
        range[0] = vtkwincube->moment->GetMin();
        range[1] = vtkwincube->moment->GetMax();
    }
}

void LutCustomize::plotHistogram()
{
    vtkSmartPointer<vtkExtractHistogram> extraction = vtkSmartPointer<vtkExtractHistogram>::New();
    int numberOfBins = -1;
    extraction->UseCustomBinRangesOn();
    QString selected_scale;
    if (isPoint3D) {
        extraction->SetInputData(vtkwin->pp->getPolyData());
        numberOfBins = vtkwin->vispoint->getOrigin()->getbinNumber();
        selected_scale = vtkwin->getSelectedScale();
    } else if (isFits2D) {
        selected_scale = vtkwin->getSelectedScale();
        int pos = 0;
        if (vtkwin->ui->listWidget->selectionModel()->selectedRows().count() != 0
            && vtkwin->getLayerListImages()
                            .at(vtkwin->ui->listWidget->selectionModel()
                                        ->selectedRows()
                                        .at(0)
                                        .row())
                            ->getType()
                    == 0) {
            pos = vtkwin->ui->listWidget->selectionModel()->selectedRows().at(0).row();
        }
        extraction->SetInputData(
                vtkwin->getLayerListImages().at(pos)->getFits()->GetOutputDataObject(0));
        numberOfBins = (vtkwin->getLayerListImages().at(pos)->getFits()->GetNaxes(0)
                        * vtkwin->getLayerListImages().at(pos)->getFits()->GetNaxes(1))
                / 10;
    } else if (isFits3D) {
        selected_scale = ui->scalingComboBox->currentText();
        extraction->SetInputData(vtkwincube->readerSlice->GetOutput());
        float dimX = vtkwincube->readerSlice->GetBounds()[1]
                - vtkwincube->readerSlice->GetBounds()[0] + 1;
        float dimY = vtkwincube->readerSlice->GetBounds()[3]
                - vtkwincube->readerSlice->GetBounds()[2] + 1;
        numberOfBins = (dimX * dimY) / 10;
    } else if (isMoment) {
        selected_scale = ui->scalingComboBox->currentText();
        extraction->SetInputData(vtkwincube->moment->GetOutput());
        numberOfBins =
                vtkwincube->moment->GetOutput()->GetPointData()->GetScalars()->GetNumberOfValues()
                / 10;
    }

    extraction->SetBinCount(numberOfBins);
    extraction->SetCustomBinRanges(range);
    extraction->Update();
    vtkTable *const histogram = extraction->GetOutput();
    vtkDoubleArray *const bin_extents =
            vtkDoubleArray::SafeDownCast(histogram->GetRowData()->GetArray("bin_extents"));
    vtkIntArray *const bin_values =
            vtkIntArray::SafeDownCast(histogram->GetRowData()->GetArray((int)1));

    bin_values->GetRange(y_range);
    QVector<double> x(numberOfBins);
    QVector<double> y(numberOfBins);
    vtkIdType nPoints = bin_extents->GetDataSize();
    for (vtkIdType i = 0; i < nPoints; ++i) {
        x.push_back(bin_extents->GetValue(i));
        y.push_back(bin_values->GetValue(i));
    }
    // create graph and assign data to it:
    ui->histogramWidget->graph(0)->setData(x, y);
    if (selected_scale == "Log") {
        ui->histogramWidget->xAxis->setScaleType(QCPAxis::stLogarithmic);
        QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
        logTicker->setLogBase(10);
        ui->histogramWidget->xAxis->setTicker(logTicker);
    } else if (selected_scale == "Linear") {
        ui->histogramWidget->xAxis->setScaleType(QCPAxis::stLinear);
        QSharedPointer<QCPAxisTicker> linearTicker(new QCPAxisTicker);
        ui->histogramWidget->xAxis->setTicker(linearTicker);
    }

    ui->histogramWidget->yAxis->setTickLabels(false);
    // set axes ranges, so we see all data:
    ui->histogramWidget->xAxis->setRange(range[0] - (range[0] * 0.1), range[1] * 1.1f);
    ui->histogramWidget->yAxis->setRange(y_range[0], y_range[1]);
    drawLine(range[0], range[1]);
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
    Q_UNUSED(event);
    if (vtkwin && vtkwin->isVisible()) {
        vtkwin->activateWindow();
        vtkwin->raise();
    }
}

void LutCustomize::refreshColorbar()
{
    on_ShowColorbarCheckBox_clicked(false);
    on_ShowColorbarCheckBox_clicked(true);
}

void LutCustomize::on_ShowColorbarCheckBox_clicked(bool checked)
{
    if (isPoint3D)
        vtkwin->showColorbar(checked);
    else if (isFits2D)
        vtkwin->showColorbarFits(checked, ui->fromSpinBox->text().toFloat(),
                                 ui->toSpinBox->text().toFloat());
    else if (isFits3D || isMoment)
        vtkwincube->showColorbar(checked, ui->fromSpinBox->text().toFloat(),
                                 ui->toSpinBox->text().toFloat());
}

void LutCustomize::on_okPushButton_clicked()
{
    if (isPoint3D) {
        if (vtkwin->ui->glyphShapeComboBox->isEnabled()) {
            vtkwin->getGlyphActor()->GetMapper()->SetLookupTable(vtkwin->pp->getLookupTable());
            vtkwin->getGlyphActor()->GetMapper()->SetScalarRange(
                    vtkwin->pp->getLookupTable()->GetRange());
        }
        if (ui->scalingComboBox->currentText() == "Linear")
            vtkwin->ui->linear3dRadioButton->setChecked(true);
        else
            vtkwin->ui->log3dRadioButton->setChecked(true);

        vtkwin->ui->lut3dComboBox->setCurrentText(ui->lutComboBox->currentText());
        vtkwin->changePalette(ui->lutComboBox->currentText().toStdString().c_str());
        vtkwin->showColorbar(ui->ShowColorbarCheckBox->isChecked());
        vtkwin->pp->setLookupTable(ui->fromSpinBox->text().toDouble(),
                                   ui->toSpinBox->text().toDouble());
        vtkwin->ui->qVTK1->renderWindow()->GetInteractor()->Render();

    } else if (isFits2D) {
        vtkwin->ui->lutComboBox->setCurrentText(ui->lutComboBox->currentText());
        if (ui->scalingComboBox->currentText() == "Linear")
            vtkwin->ui->linearadioButton->setChecked(true);
        else
            vtkwin->ui->logRadioButton->setChecked(true);

        vtkwin->showColorbarFits(ui->ShowColorbarCheckBox->isChecked(),
                                 ui->fromSpinBox->text().toFloat(),
                                 ui->toSpinBox->text().toFloat());
        vtkwin->changeFitsScale(ui->lutComboBox->currentText().toStdString().c_str(),
                                ui->scalingComboBox->currentText().toStdString().c_str(),
                                ui->fromSpinBox->text().toFloat(), ui->toSpinBox->text().toFloat());
        vtkwin->ui->qVTK1->renderWindow()->GetInteractor()->Render();

    } else if (isFits3D || isMoment) {
        vtkwincube->showColorbar(ui->ShowColorbarCheckBox->isChecked(),
                                 ui->fromSpinBox->text().toFloat(),
                                 ui->toSpinBox->text().toFloat());
        vtkwincube->changeFitsScale(ui->lutComboBox->currentText().toStdString().c_str(),
                                    ui->scalingComboBox->currentText().toStdString().c_str(),
                                    ui->fromSpinBox->text().toFloat(),
                                    ui->toSpinBox->text().toFloat());
        vtkwincube->ui->qVtkSlice->renderWindow()->Render();
    }
}

void LutCustomize::on_fromSpinBox_valueChanged(double arg1)
{
    drawLine(arg1, ui->toSpinBox->value());
}

void LutCustomize::on_toSpinBox_valueChanged(double arg1)
{
    drawLine(ui->fromSpinBox->value(), arg1);
}

void LutCustomize::on_resetMinPushButton_clicked()
{
    if (isPoint3D)
        ui->fromSpinBox->setValue(
                vtkwin->pp->getPolyData()
                        ->GetPointData()
                        ->GetScalars(
                                vtkwin->ui->scalarComboBox->currentText().toStdString().c_str())
                        ->GetRange()[0]);
    else if (isFits2D)
        ui->fromSpinBox->setValue(vtkwin->getFitsImage()->GetMin());
    else if (isFits3D)
        ui->fromSpinBox->setValue(vtkwincube->readerSlice->GetValueRange()[0]);
    else if (isMoment)
        ui->fromSpinBox->setValue(vtkwincube->moment->GetMin());
}

void LutCustomize::on_resetMaxPushButton_clicked()
{
    if (isPoint3D)
        ui->toSpinBox->setValue(
                vtkwin->pp->getPolyData()
                        ->GetPointData()
                        ->GetScalars(
                                vtkwin->ui->scalarComboBox->currentText().toStdString().c_str())
                        ->GetRange()[1]);
    else if (isFits2D)
        ui->toSpinBox->setValue(vtkwin->getFitsImage()->GetMax());
    else if (isFits3D)
        ui->toSpinBox->setValue(vtkwincube->readerSlice->GetValueRange()[1]);
    else if (isMoment)
        ui->toSpinBox->setValue(vtkwincube->moment->GetMax());
}
