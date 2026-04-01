#include "PvDiagramWidget.h"

#include "ColorMaps.h"
#include "qcustomplot.h"

#include <QLabel>
#include <QVBoxLayout>

#include <algorithm>
#include <limits>

using namespace Qt::StringLiterals;

PvDiagramWidget::PvDiagramWidget(QWidget *parent)
    : QWidget(parent), infoLabel(new QLabel(this)), plot(new QCustomPlot(this)), colorMap(nullptr),
      titleElement(nullptr)
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowFlag(Qt::Window);
    this->setWindowTitle(u"PV Diagram"_s);
    this->resize(900, 640);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    this->infoLabel->setWordWrap(true);
    this->infoLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(this->infoLabel);
    layout->addWidget(this->plot, 1);

    this->plot->plotLayout()->insertRow(0);
    this->titleElement = new QCPTextElement(this->plot, u"PV Diagram"_s);
    this->plot->plotLayout()->addElement(0, 0, this->titleElement);
    this->colorMap = new QCPColorMap(this->plot->xAxis, this->plot->yAxis);
    this->colorMap->setInterpolate(false);
    this->plot->axisRect()->setupFullAxesBox(true);
}

void PvDiagramWidget::setPvData(const QVector<double> &positions, const QVector<double> &spectral,
                                const QVector<double> &intensities, int nx, int ny,
                                const QString &xLabel, const QString &yLabel, const QString &title,
                                const QString &details)
{
    if (!this->colorMap || nx <= 0 || ny <= 0 || positions.size() != nx || spectral.size() != ny
        || intensities.size() != nx * ny) {
        return;
    }

    this->setWindowTitle(u"PV Diagram"_s);
    this->infoLabel->setText(details);
    this->titleElement->setText(title);
    this->plot->xAxis->setLabel(xLabel);
    this->plot->yAxis->setLabel(yLabel);

    const double xMin = positions.first();
    const double xMax = positions.last();
    const double yMin = *std::min_element(spectral.cbegin(), spectral.cend());
    const double yMax = *std::max_element(spectral.cbegin(), spectral.cend());

    this->colorMap->data()->setSize(nx, ny);
    this->colorMap->data()->setRange(QCPRange(xMin, xMax), QCPRange(yMin, yMax));

    double minValue = std::numeric_limits<double>::infinity();
    double maxValue = -std::numeric_limits<double>::infinity();
    for (int y = 0; y < ny; ++y) {
        for (int x = 0; x < nx; ++x) {
            const double value = intensities[y * nx + x];
            this->colorMap->data()->setCell(x, y, value);
            if (std::isfinite(value)) {
                minValue = std::min(minValue, value);
                maxValue = std::max(maxValue, value);
            }
        }
    }

    if (!std::isfinite(minValue) || !std::isfinite(maxValue) || minValue == maxValue) {
        minValue = 0.0;
        maxValue = minValue + 1.0;
    }

    this->colorMap->setDataRange(QCPRange(minValue, maxValue));
    this->colorMap->setGradient(QCPColorGradient::gpJet);
    this->plot->rescaleAxes();
    this->plot->replot();
    this->show();
    this->raise();
    this->activateWindow();
}
