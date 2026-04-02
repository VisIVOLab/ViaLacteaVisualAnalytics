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
#include <vtkPointData.h>
#include <vtkTable.h>

#include <QDoubleValidator>
#include <QElapsedTimer>
#include <QDebug>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace Qt::StringLiterals;

namespace {
constexpr vtkIdType histogramTargetSamples = 250000;
constexpr int histogramBinCount = 512;
constexpr double logAxisEpsilon = 1e-12;

double sampledPositiveFloor(vtkImageData *dataset)
{
    if (!dataset) {
        return 0.0;
    }
    auto *scalars =
            dataset->GetPointData() ? dataset->GetPointData()->GetScalars() : nullptr;
    const vtkIdType nels = dataset->GetNumberOfPoints();
    if (!scalars || nels <= 0) {
        return 0.0;
    }

    const vtkIdType stride =
            std::max<vtkIdType>(1, static_cast<vtkIdType>(std::ceil(
                                           static_cast<double>(nels) / histogramTargetSamples)));
    std::vector<double> positives;
    positives.reserve(static_cast<std::size_t>(std::min<vtkIdType>(nels / stride, 32768)));
    for (vtkIdType i = 0; i < nels; i += stride) {
        const double value = scalars->GetComponent(i, 0);
        if (std::isfinite(value) && value > 0.0) {
            positives.push_back(value);
        }
    }
    if (positives.empty()) {
        return 0.0;
    }
    std::sort(positives.begin(), positives.end());
    const std::size_t index =
            std::min<std::size_t>(positives.size() - 1,
                                  static_cast<std::size_t>(std::floor((positives.size() - 1) * 0.01)));
    return std::max(logAxisEpsilon, positives[index]);
}
}

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
    QElapsedTimer timer;
    timer.start();
    qDebug().noquote() << QStringLiteral("[lut] histogram update started");
    if (!this->dataset) {
        return;
    }

    const vtkIdType nels = this->dataset->GetNumberOfPoints();
    if (nels <= 0 || !std::isfinite(this->datasetRange[0]) || !std::isfinite(this->datasetRange[1])
        || this->datasetRange[0] >= this->datasetRange[1]) {
        return;
    }

    auto *scalars =
            this->dataset->GetPointData() ? this->dataset->GetPointData()->GetScalars() : nullptr;
    if (!scalars) {
        return;
    }

    const vtkIdType stride =
            std::max<vtkIdType>(1, static_cast<vtkIdType>(std::ceil(
                                           static_cast<double>(nels) / histogramTargetSamples)));
    const double minValue = this->datasetRange[0];
    const double maxValue = this->datasetRange[1];
    const double binWidth = (maxValue - minValue) / static_cast<double>(histogramBinCount);
    if (!std::isfinite(binWidth) || binWidth <= 0.0) {
        return;
    }

    std::vector<double> bins(histogramBinCount, 0.0);
    for (vtkIdType i = 0; i < nels; i += stride) {
        const double value = scalars->GetComponent(i, 0);
        if (!std::isfinite(value)) {
            continue;
        }
        int bin = static_cast<int>(std::floor((value - minValue) / binWidth));
        bin = std::clamp(bin, 0, histogramBinCount - 1);
        bins[static_cast<std::size_t>(bin)] += 1.0;
    }

    QVector<double> x(histogramBinCount), y(histogramBinCount);
    for (int i = 0; i < histogramBinCount; ++i) {
        x[i] = minValue + (i + 0.5) * binWidth;
        y[i] = bins[static_cast<std::size_t>(i)];
    }
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
    qDebug().noquote()
            << QStringLiteral("[lut] histogram update completed ms=%1").arg(timer.elapsed());
}

void LUTCustomizerDialog::changeAxisScaling(bool logarithmic)
{
    const double positiveFloor = sampledPositiveFloor(this->dataset);
    const bool safeLogarithmic = logarithmic && positiveFloor > 0.0;
    if (safeLogarithmic) {
        ui->plot->xAxis->setScaleType(QCPAxis::stLogarithmic);
        ui->plot->xAxis->setTicker(QSharedPointer<QCPAxisTickerLog>(new QCPAxisTickerLog));
        const double upper = std::max(this->datasetRange[1], positiveFloor * 10.0);
        ui->plot->xAxis->setRange(positiveFloor, upper);
    } else {
        ui->plot->xAxis->setScaleType(QCPAxis::stLinear);
        ui->plot->xAxis->setTicker(QSharedPointer<QCPAxisTicker>(new QCPAxisTicker));
    }

    ui->plot->replot();
}

void LUTCustomizerDialog::init(vtkImageData *dataset, vtkLookupTable *lut)
{
    if (!dataset || !lut) {
        return;
    }
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
    if (!this->lut) {
        return;
    }
    ColorMaps::SetColorMap(this->lut, ui->comboLut->currentText().toStdString());
    double minValue = ui->lineMin->text().toDouble();
    double maxValue = ui->lineMax->text().toDouble();
    if (!std::isfinite(minValue) || !std::isfinite(maxValue) || minValue >= maxValue) {
        return;
    }
    if (ui->comboScale->currentIndex() == 1) {
        const double positiveFloor = sampledPositiveFloor(this->dataset);
        if (positiveFloor <= 0.0) {
            return;
        }
        maxValue = std::max(maxValue, positiveFloor);
        minValue = std::max(minValue, positiveFloor);
        this->lut->SetScaleToLog10();
    } else {
        this->lut->SetScaleToLinear();
    }
    this->lut->SetTableRange(minValue, maxValue);
    this->lut->Build();
    emit this->lutUpdated();
}

void LUTCustomizerDialog::buttonClicked(QAbstractButton *btn)
{
    if (ui->buttonBox->buttonRole(btn) == QDialogButtonBox::ApplyRole) {
        this->updateLut();
    }
}
