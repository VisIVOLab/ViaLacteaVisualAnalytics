#include "LUTCustomizerDialog.h"
#include "ui_LUTCustomizerDialog.h"

#include "ColorMaps.h"

#include <vtkDataSetAttributes.h>
#include <vtkDoubleArray.h>
#include <vtkExtractHistogram.h>
#include <vtkImageData.h>
#include <vtkIntArray.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkTable.h>

#include <QDoubleValidator>

using namespace Qt::StringLiterals;

LUTCustomizerDialog::LUTCustomizerDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::LUTCustomizerDialog), selectedLine(nullptr)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);

    this->setupPlot();

    // Setup line edits
    ui->lineMin->setValidator(new QDoubleValidator(ui->lineMin));
    ui->lineMax->setValidator(new QDoubleValidator(ui->lineMax));
    QObject::connect(ui->lineMin, &QLineEdit::textEdited, this,
                     &LUTCustomizerDialog::updateReferenceLines);
    QObject::connect(ui->lineMax, &QLineEdit::textEdited, this,
                     &LUTCustomizerDialog::updateReferenceLines);

    // Setup reset buttons
    ui->btnResetMin->setIcon(QIcon(u":/icons/RESET.png"_s));
    ui->btnResetMax->setIcon(QIcon(u":/icons/RESET.png"_s));
    QObject::connect(ui->btnResetMin, &QToolButton::clicked, this, &LUTCustomizerDialog::resetMin);
    QObject::connect(ui->btnResetMax, &QToolButton::clicked, this, &LUTCustomizerDialog::resetMax);

    // Color Maps Combobox
    auto cmaps = ColorMaps::GetColorMapNames();
    std::for_each(cmaps.cbegin(), cmaps.cend(), [this](const std::string &name) {
        ui->comboLut->addItem(QString::fromStdString(name));
    });
    ui->comboLut->setCurrentText(QString::fromStdString(ColorMaps::DefaultColorMap));

    // Axis scaling
    QObject::connect(ui->comboScale, &QComboBox::currentIndexChanged, this,
                     &LUTCustomizerDialog::changeAxisScaling);

    // Apply button
    QObject::connect(ui->buttonBox, &QDialogButtonBox::clicked, this,
                     &LUTCustomizerDialog::buttonClicked);
}

LUTCustomizerDialog::~LUTCustomizerDialog()
{
    delete ui;
}

void LUTCustomizerDialog::setupPlot()
{
    ui->plot->setInteractions(QCP::iSelectItems | QCP::iRangeDrag);
    ui->plot->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->plot->addGraph();

    this->refLineMin = new QCPItemLine(ui->plot);
    this->refLineMin->setPen({ Qt::red });

    this->refLineMax = new QCPItemLine(ui->plot);
    this->refLineMax->setPen({ Qt::green });

    QObject::connect(ui->plot, &QCustomPlot::itemClick, this,
                     &LUTCustomizerDialog::selectReferenceLine);
    QObject::connect(ui->plot, &QCustomPlot::mouseMove, this,
                     &LUTCustomizerDialog::moveReferenceLine);
    QObject::connect(ui->plot, &QCustomPlot::mouseRelease, this,
                     &LUTCustomizerDialog::deselectReferenceLine);
}

void LUTCustomizerDialog::plotHistogram()
{
    const vtkIdType nels = this->dataset->GetNumberOfPoints();
    const int nbins = nels / 10;

    vtkNew<vtkExtractHistogram> filter;
    filter->SetInputData(this->dataset);
    filter->UseCustomBinRangesOn();
    filter->SetCustomBinRanges(this->datasetRange);
    filter->SetBinCount(nbins);
    filter->Update();

    auto bin_extents = static_cast<double *>(
            filter->GetOutput()->GetRowData()->GetAbstractArray("bin_extents")->GetVoidPointer(0));
    auto bin_values = static_cast<int *>(
            filter->GetOutput()->GetRowData()->GetAbstractArray("bin_values")->GetVoidPointer(0));

    QVector<double> x(bin_extents, bin_extents + nbins);
    QVector<double> y(bin_values, bin_values + nbins);
    ui->plot->graph()->setData(x, y, true);
    ui->plot->graph()->setBrush(Qt::blue);
    ui->plot->rescaleAxes();
    if (this->datasetRange[0] < 0) {
        ui->plot->xAxis->setRange(this->datasetRange[0] * 1.1, this->datasetRange[1] * 1.1);
    } else {
        ui->plot->xAxis->setRange(this->datasetRange[0] * 0.9, this->datasetRange[1] * 1.1);
    }

    const double *lutRange = this->lut->GetTableRange();
    this->refLineMin->start->setCoords(lutRange[0], 0.);
    this->refLineMin->end->setCoords(lutRange[0], ui->plot->yAxis->range().upper);
    this->refLineMax->start->setCoords(lutRange[1], 0.);
    this->refLineMax->end->setCoords(lutRange[1], ui->plot->yAxis->range().upper);

    this->changeAxisScaling(this->lut->UsingLogScale());
}

void LUTCustomizerDialog::changeAxisScaling(bool logarithmic)
{
    if (logarithmic) {
        ui->plot->xAxis->setScaleType(QCPAxis::stLogarithmic);
        ui->plot->xAxis->setTicker(QSharedPointer<QCPAxisTickerLog>(new QCPAxisTickerLog));
    } else {
        ui->plot->xAxis->setScaleType(QCPAxis::stLinear);
        ui->plot->xAxis->setTicker(QSharedPointer<QCPAxisTicker>(new QCPAxisTicker));
    }

    ui->plot->replot();
}

void LUTCustomizerDialog::init(vtkImageData *dataset, vtkLookupTable *lut)
{
    this->dataset = dataset;
    this->dataset->GetScalarRange(this->datasetRange);

    this->lut = lut;
    const double *lutRange = this->lut->GetTableRange();
    ui->lineMin->setText(QString::number(lutRange[0]));
    ui->lineMax->setText(QString::number(lutRange[1]));
    ui->comboLut->setCurrentText(QString::fromStdString(this->lut->GetObjectName()));
    ui->comboScale->setCurrentIndex(this->lut->UsingLogScale());

    this->plotHistogram();
}

void LUTCustomizerDialog::resetMin()
{
    ui->lineMin->setText(QString::number(this->datasetRange[0]));
    this->refLineMin->start->setCoords(this->datasetRange[0], 0.);
    this->refLineMin->end->setCoords(this->datasetRange[0], ui->plot->yAxis->range().upper);
    ui->plot->replot();
}

void LUTCustomizerDialog::resetMax()
{
    ui->lineMax->setText(QString::number(this->datasetRange[1]));
    this->refLineMax->start->setCoords(this->datasetRange[1], 0.);
    this->refLineMax->end->setCoords(this->datasetRange[1], ui->plot->yAxis->range().upper);
    ui->plot->replot();
}

void LUTCustomizerDialog::selectReferenceLine(QCPAbstractItem *item)
{
    this->selectedLine = qobject_cast<QCPItemLine *>(item);
    ui->plot->setInteraction(QCP::iRangeDrag, false);
}

void LUTCustomizerDialog::moveReferenceLine(QMouseEvent *event)
{
    if (!this->selectedLine || event->buttons().testFlag(Qt::NoButton)) {
        return;
    }

    const double x = ui->plot->xAxis->pixelToCoord(event->pos().x());
    this->selectedLine->start->setCoords(x, 0.);
    this->selectedLine->end->setCoords(x, ui->plot->yAxis->range().upper);
    ui->plot->replot();

    if (this->selectedLine == this->refLineMin) {
        ui->lineMin->setText(QString::number(x));
    } else {
        ui->lineMax->setText(QString::number(x));
    }
}

void LUTCustomizerDialog::deselectReferenceLine()
{
    this->selectedLine = nullptr;
    ui->plot->setInteraction(QCP::iRangeDrag, true);
    ui->plot->deselectAll();
    ui->plot->replot();
}

void LUTCustomizerDialog::updateReferenceLines()
{
    const double minX = ui->lineMin->text().toDouble();
    const double maxX = ui->lineMax->text().toDouble();
    this->refLineMin->start->setCoords(minX, 0.);
    this->refLineMin->end->setCoords(minX, ui->plot->yAxis->range().upper);
    this->refLineMax->start->setCoords(maxX, 0.);
    this->refLineMax->end->setCoords(maxX, ui->plot->yAxis->range().upper);
    ui->plot->replot();
}

void LUTCustomizerDialog::updateLut()
{
    ColorMaps::SetColorMap(this->lut, ui->comboLut->currentText().toStdString());
    this->lut->SetTableRange(ui->lineMin->text().toDouble(), ui->lineMax->text().toDouble());
    this->lut->SetScale(ui->comboScale->currentIndex());
    emit this->lutUpdated();
}

void LUTCustomizerDialog::buttonClicked(QAbstractButton *btn)
{
    if (ui->buttonBox->buttonRole(btn) == QDialogButtonBox::ApplyRole) {
        this->updateLut();
    }
}
