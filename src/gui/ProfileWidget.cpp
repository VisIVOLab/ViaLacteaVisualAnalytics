#include "ProfileWidget.h"
#include "ui_ProfileWidget.h"

#include "vtkInteractorStyleProfile.h"

#include <vtkImageData.h>

using namespace Qt::StringLiterals;

ProfileWidget::ProfileWidget(vtkInteractorStyleProfile *style, vtkImageData *dataset,
                             const std::string &filepath, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::ProfileWidget),
      dataset(dataset),
      astro(filepath),
      interactor(style)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowFlag(Qt::Window);

    // Get info from dataset
    this->ScalarPointer = static_cast<float *>(this->dataset->GetScalarPointer());
    this->dataset->GetIncrements(this->Increments);
    this->dataset->GetDimensions(this->Dimensions);

    // Setup Interactor
    this->interactor->SetLengths(this->Dimensions[0] - 1, this->Dimensions[1] - 1);
    QObject::connect(ui->checkLive, &QCheckBox::toggled, this, &ProfileWidget::setLiveMode);
}

ProfileWidget::~ProfileWidget()
{
    delete ui;
}

void ProfileWidget::setupImagePlots()
{
    this->interactor->SetCallback(
            [this](double x, double y, bool live) { this->plotProfile(x, y, live); });

    // Setup plots
    const std::string unit = this->astro.getPhysicalUnit();
    const QString yLabel = unit.empty() ? u"Value"_s : QString::fromStdString(unit);

    ui->plot1->addGraph();
    ui->plot1->setInteractions(QCP::iRangeDrag);
    ui->plot1->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->plot1->plotLayout()->insertRow(0);
    ui->plot1->plotLayout()->addElement(0, 0, new QCPTextElement(ui->plot1, u"X Profile"_s));
    ui->plot1->xAxis->setLabel(u"X Coordinate"_s);
    ui->plot1->yAxis->setLabel(yLabel);
    this->refLineX = new QCPItemLine(ui->plot1);
    this->refLineX->setPen({ Qt::red });

    ui->plot2->addGraph();
    ui->plot2->setInteractions(QCP::iRangeDrag);
    ui->plot2->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->plot2->plotLayout()->insertRow(0);
    ui->plot2->plotLayout()->addElement(0, 0, new QCPTextElement(ui->plot2, u"Y Profile"_s));
    this->refLineY = new QCPItemLine(ui->plot2);
    this->refLineY->setPen({ Qt::red });
    ui->plot2->xAxis->setLabel(u"Y Coordinate"_s);
    ui->plot2->yAxis->setLabel(yLabel);
}

void ProfileWidget::setupSpectrumPlot()
{
    ui->plot2->hide();

    this->interactor->SetCallback(
            [this](double x, double y, bool live) { this->plotSpectrum(x, y, live); });

    const std::string unit = this->astro.getPhysicalUnit();
    const QString yLabel = unit.empty() ? u"Value"_s : QString::fromStdString(unit);

    const std::string spectralUnit = this->astro.getAxisUnit(2);
    const QString xLabel = spectralUnit.empty() ? u"Value"_s : QString::fromStdString(spectralUnit);

    ui->plot1->addGraph();
    ui->plot1->setInteractions(QCP::iRangeDrag);
    ui->plot1->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->plot1->plotLayout()->insertRow(0);
    ui->plot1->plotLayout()->addElement(0, 0, new QCPTextElement(ui->plot1, u"Z Profile"_s));
    ui->plot1->xAxis->setLabel(xLabel);
    ui->plot1->yAxis->setLabel(yLabel);
}

void ProfileWidget::setLiveMode(bool live)
{
    this->interactor->SetLiveMode(live);
}

void ProfileWidget::plotProfile(double x, double y, bool live)
{
    ui->checkLive->setChecked(live);
    const long pixX = std::lround(x);
    const long pixY = std::lround(y);

    // X Profile
    QVector<double> keyX(this->Dimensions[0]), valuesX(this->Dimensions[0]);
    for (vtkIdType i = 0; i < this->Dimensions[0]; ++i) {
        const vtkIdType idx = i * this->Increments[0] + pixY * this->Increments[1];
        keyX[i] = i;
        valuesX[i] = this->ScalarPointer[idx];
    }
    ui->plot1->graph()->setData(keyX, valuesX);
    ui->plot1->rescaleAxes();
    ui->plot1->xAxis->setRange(-1., this->Dimensions[0] + 1.);
    this->refLineX->start->setCoords(pixX, ui->plot1->yAxis->range().lower);
    this->refLineX->end->setCoords(pixX, ui->plot1->yAxis->range().upper);
    ui->plot1->replot();

    // Y Profile
    QVector<double> keyY(this->Dimensions[1]), valuesY(this->Dimensions[1]);
    for (vtkIdType j = 0; j < this->Dimensions[1]; ++j) {
        const vtkIdType idx = pixX * this->Increments[0] + j * this->Increments[1];
        keyY[j] = j;
        valuesY[j] = this->ScalarPointer[idx];
    }
    ui->plot2->graph()->setData(keyY, valuesY);
    ui->plot2->rescaleAxes();
    ui->plot2->xAxis->setRange(-1., this->Dimensions[1] + 1.);
    this->refLineY->start->setCoords(pixY, ui->plot2->yAxis->range().lower);
    this->refLineY->end->setCoords(pixY, ui->plot2->yAxis->range().upper);
    ui->plot2->replot();

    this->show();
    this->raise();
}

void ProfileWidget::plotSpectrum(double x, double y, bool live)
{
    ui->checkLive->setChecked(live);
    const long pixX = std::lround(x);
    const long pixY = std::lround(y);

    const double initSpectral = this->astro.getInitialSpectralValue();
    const double *axesInc = this->astro.getIncrements();

    QVector<double> key(this->Dimensions[2]), values(this->Dimensions[2]);
    for (vtkIdType k = 0; k < this->Dimensions[2]; ++k) {
        const vtkIdType idx = 1ll * pixX * this->Increments[0] + pixY * this->Increments[1]
                + k * this->Increments[2];
        key[k] = initSpectral + k * axesInc[2];
        values[k] = this->ScalarPointer[idx];
    }
    ui->plot1->graph()->setData(key, values);
    ui->plot1->rescaleAxes();
    qobject_cast<QCPTextElement *>(ui->plot1->plotLayout()->element(0, 0))
            ->setText(u"Z Profile (%1, %2)"_s.arg(pixX).arg(pixY));
    ui->plot1->replot();

    this->show();
    this->raise();
}
