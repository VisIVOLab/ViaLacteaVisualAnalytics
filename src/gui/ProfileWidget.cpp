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
      astro(std::make_unique<AstroUtils>(filepath)),
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

ProfileWidget::ProfileWidget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::ProfileWidget),
      dataset(nullptr),
      astro(nullptr),
      ScalarPointer(nullptr),
      Dimensions{ 0, 0, 0 },
      Increments{ 0, 0, 0 },
      interactor(nullptr),
      refLineX(nullptr),
      refLineY(nullptr)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowFlag(Qt::Window);
}

ProfileWidget::~ProfileWidget()
{
    delete ui;
}

void ProfileWidget::setUsageMode(UsageMode mode, const QString &windowTitle)
{
    this->usageMode = mode;
    this->applyUsageModeUi(windowTitle);
}

void ProfileWidget::applyUsageModeUi(const QString &windowTitle)
{
    const bool liveMode = this->usageMode == UsageMode::ProbeLive;
    ui->checkLive->setVisible(liveMode);
    ui->checkLive->setEnabled(liveMode);
    if (!liveMode) {
        ui->checkLive->setChecked(false);
        if (this->interactor) {
            this->interactor->SetLiveMode(false);
        }
    }

    if (!windowTitle.isEmpty()) {
        this->setWindowTitle(windowTitle);
    } else {
        this->setWindowTitle(liveMode ? u"Profile"_s : u"Region Profile"_s);
    }
}

void ProfileWidget::setupImagePlots()
{
    this->applyUsageModeUi();
    if (!this->interactor || !this->astro) {
        return;
    }
    this->interactor->SetCallback(
            [this](double x, double y, bool live) { this->plotProfile(x, y, live); });

    // Setup plots
    const std::string unit = this->astro->getPhysicalUnit();
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

void ProfileWidget::setupImagePlots(const QString &xLabel, const QString &yLabel)
{
    this->applyUsageModeUi();
    ui->plot1->clearGraphs();
    ui->plot2->clearGraphs();
    ui->plot1->addGraph();
    ui->plot2->addGraph();
    ui->plot1->setInteractions(QCP::iRangeDrag);
    ui->plot1->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->plot2->setInteractions(QCP::iRangeDrag);
    ui->plot2->axisRect()->setRangeDrag(Qt::Horizontal);
    if (!ui->plot1->plotLayout()->element(0, 0)) {
        ui->plot1->plotLayout()->insertRow(0);
        ui->plot1->plotLayout()->addElement(0, 0, new QCPTextElement(ui->plot1, u"X Profile"_s));
    }
    if (!ui->plot2->plotLayout()->element(0, 0)) {
        ui->plot2->plotLayout()->insertRow(0);
        ui->plot2->plotLayout()->addElement(0, 0, new QCPTextElement(ui->plot2, u"Y Profile"_s));
    }
    ui->plot1->xAxis->setLabel(u"X Coordinate"_s);
    ui->plot1->yAxis->setLabel(yLabel);
    ui->plot2->xAxis->setLabel(u"Y Coordinate"_s);
    ui->plot2->yAxis->setLabel(yLabel);
    if (!this->refLineX) {
        this->refLineX = new QCPItemLine(ui->plot1);
        this->refLineX->setPen({ Qt::red });
    }
    if (!this->refLineY) {
        this->refLineY = new QCPItemLine(ui->plot2);
        this->refLineY->setPen({ Qt::red });
    }
    Q_UNUSED(xLabel);
}

void ProfileWidget::setupSpectrumPlot()
{
    this->applyUsageModeUi();
    if (!this->astro || !this->interactor) {
        return;
    }
    ui->plot2->hide();

    this->interactor->SetCallback(
            [this](double x, double y, bool live) { this->plotSpectrum(x, y, live); });

    const std::string unit = this->astro->getPhysicalUnit();
    const QString yLabel = unit.empty() ? u"Value"_s : QString::fromStdString(unit);

    const std::string spectralUnit = this->astro->getAxisUnit(2);
    const QString xLabel = spectralUnit.empty() ? u"Value"_s : QString::fromStdString(spectralUnit);

    ui->plot1->addGraph();
    ui->plot1->setInteractions(QCP::iRangeDrag);
    ui->plot1->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->plot1->plotLayout()->insertRow(0);
    ui->plot1->plotLayout()->addElement(0, 0, new QCPTextElement(ui->plot1, u"Z Profile"_s));
    ui->plot1->xAxis->setLabel(xLabel);
    ui->plot1->yAxis->setLabel(yLabel);
}

void ProfileWidget::setupSpectrumPlot(const QString &xLabel, const QString &yLabel)
{
    this->applyUsageModeUi();
    ui->plot2->hide();
    ui->plot1->clearGraphs();
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
    if (this->interactor) {
        this->interactor->SetLiveMode(live);
    }
}

void ProfileWidget::plotProfile(double x, double y, bool live)
{
    if (this->usageMode == UsageMode::ProbeLive) {
        ui->checkLive->setChecked(live);
    }
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
    if (this->usageMode == UsageMode::ProbeLive) {
        ui->checkLive->setChecked(live);
    }
    const long pixX = std::lround(x);
    const long pixY = std::lround(y);

    if (!this->astro) {
        return;
    }

    const double initSpectral = this->astro->getInitialSpectralValue();
    const double *axesInc = this->astro->getIncrements();

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

void ProfileWidget::updateSpectrumPlot(const QVector<double> &key, const QVector<double> &values,
                                       const QString &title, bool live)
{
    if (ui->plot1->graphCount() == 0) {
        ui->plot1->addGraph();
    }

    if (this->usageMode == UsageMode::ProbeLive) {
        ui->checkLive->setChecked(live);
    }
    ui->plot1->graph()->setData(key, values);
    ui->plot1->rescaleAxes();
    if (auto *titleElement = qobject_cast<QCPTextElement *>(ui->plot1->plotLayout()->element(0, 0))) {
        titleElement->setText(title);
    }
    ui->plot1->replot();
    this->show();
    this->raise();
}

void ProfileWidget::updateImageProfiles(const QVector<double> &keyX, const QVector<double> &valuesX,
                                        const QVector<double> &keyY, const QVector<double> &valuesY,
                                        double probeX, double probeY, bool live)
{
    if (ui->plot1->graphCount() == 0 || ui->plot2->graphCount() == 0) {
        this->setupImagePlots(u"X"_s, u"Value"_s);
    }

    if (this->usageMode == UsageMode::ProbeLive) {
        ui->checkLive->setChecked(live);
    }
    ui->plot1->graph()->setData(keyX, valuesX);
    ui->plot1->rescaleAxes();
    ui->plot1->xAxis->setRange(-1., keyX.size() + 1.);
    if (this->refLineX) {
        this->refLineX->start->setCoords(probeX, ui->plot1->yAxis->range().lower);
        this->refLineX->end->setCoords(probeX, ui->plot1->yAxis->range().upper);
    }
    if (auto *titleX = qobject_cast<QCPTextElement *>(ui->plot1->plotLayout()->element(0, 0))) {
        titleX->setText(u"X Profile"_s);
    }
    ui->plot1->replot();

    ui->plot2->graph()->setData(keyY, valuesY);
    ui->plot2->rescaleAxes();
    ui->plot2->xAxis->setRange(-1., keyY.size() + 1.);
    if (this->refLineY) {
        this->refLineY->start->setCoords(probeY, ui->plot2->yAxis->range().lower);
        this->refLineY->end->setCoords(probeY, ui->plot2->yAxis->range().upper);
    }
    if (auto *titleY = qobject_cast<QCPTextElement *>(ui->plot2->plotLayout()->element(0, 0))) {
        titleY->setText(u"Y Profile"_s);
    }
    ui->plot2->replot();

    this->show();
    this->raise();
}
