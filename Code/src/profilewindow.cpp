#include "profilewindow.h"
#include "ui_profilewindow.h"

ProfileWindow::ProfileWindow(const QString &bunit, QWidget *parent)
    : QWidget(parent, Qt::Window), ui(new Ui::ProfileWindow)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    connect(ui->liveUpdate, &QCheckBox::stateChanged, this, &ProfileWindow::liveUpdateStateChanged);

    ui->xPlotQt->addGraph();
    ui->xPlotQt->plotLayout()->insertRow(0);
    ui->xPlotQt->plotLayout()->addElement(0, 0, new QCPTextElement(ui->xPlotQt, "X Profile"));
    ui->xPlotQt->xAxis->setLabel("X Coordinate");
    ui->xPlotQt->yAxis->setLabel("Value (" + bunit + ")");
    ui->xPlotQt->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);

    ui->yPlotQt->addGraph();
    ui->yPlotQt->plotLayout()->insertRow(0);
    ui->yPlotQt->plotLayout()->addElement(0, 0, new QCPTextElement(ui->yPlotQt, "Y Profile"));
    ui->yPlotQt->xAxis->setLabel("Y Coordinate");
    ui->yPlotQt->yAxis->setLabel("Value (" + bunit + ")");
    ui->yPlotQt->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);
}

ProfileWindow::~ProfileWindow()
{
    delete ui;
}

void ProfileWindow::setLiveProfileFlag(bool flag)
{
    ui->liveUpdate->setChecked(flag);
}

void ProfileWindow::plotProfiles(const QVector<double> &xProfile, double xRef,
                                 const QVector<double> &yProfile, double yRef)
{
    plotProfile(xProfile, xRef, ui->xPlotQt);
    plotProfile(yProfile, yRef, ui->yPlotQt);
}

void ProfileWindow::plotSpectrum(const QVector<double> &spectrum)
{
    ui->yPlotQt->hide();
    auto title = qobject_cast<QCPTextElement *>(ui->xPlotQt->plotLayout()->element(0, 0));
    title->setText("Z Profile");
    ui->xPlotQt->xAxis->setLabel("Slice");

    QVector<double> key(spectrum.size());
    std::iota(key.begin(), key.end(), 0);

    ui->xPlotQt->graph()->setData(key, spectrum);
    ui->xPlotQt->rescaleAxes();
    ui->xPlotQt->replot();
}

void ProfileWindow::plotProfile(const QVector<double> &profile, double ref, QCustomPlot *plot)
{
    QVector<double> key(profile.size());
    std::iota(key.begin(), key.end(), 0);

    plot->graph()->setData(key, profile);
    plot->rescaleAxes();

    // Add reference line
    plot->clearItems();
    auto line = new QCPItemStraightLine(plot);
    line->setPen(QPen(Qt::red));
    line->point1->setCoords(ref, 0);
    line->point2->setCoords(ref, 1);

    plot->replot();
}
