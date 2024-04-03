#include "sedplotpointcustom.h"
#include "singleton.h"
#include "vialacteastringdictwidget.h"
#include "vtkCleanPolyData.h"
#include "vtkellipse.h"
#include <QMouseEvent>
#include <QToolTip>

SEDPlotPointCustom::SEDPlotPointCustom(QCustomPlot *parentPlot, double halfSize, vtkwindow_new *v)
    : QCPItemEllipse(parentPlot),
      mCenterTracer(new QCPItemTracer(parentPlot)),
      mGripDelta(),
      mInitialPos(),
      mLastWantedPos(),
      mMoveTimer(new QTimer(this)),
      mCurWantedPosPx()
{
    mCenterTracer->setStyle(QCPItemTracer::tsNone);

    topLeft->setParentAnchor(mCenterTracer->position);
    bottomRight->setParentAnchor(mCenterTracer->position);
    topLeft->setType(QCPItemPosition::ptAbsolute);
    bottomRight->setType(QCPItemPosition::ptAbsolute);
    topLeft->setCoords(-halfSize, -halfSize);
    bottomRight->setCoords(halfSize, halfSize);
    setSelectable(true); // plot moves only selectable points, see Plot::mouseMoveEvent
    setColor(QColor(Qt::yellow));
    setPen(QPen(Qt::black));
    setSelectedPen(QPen(Qt::red, halfSize));
    vtkwin = v;
    stringDictWidget = &Singleton<VialacteaStringDictWidget>::Instance();
    connect(this, SIGNAL(selectionChanged(bool)), this, SLOT(selected(bool)));
}

QPointF SEDPlotPointCustom::pos() const
{
    return mCenterTracer->position->coords();
}

const QColor &SEDPlotPointCustom::color() const
{
    return brush().color();
}

void SEDPlotPointCustom::setColor(const QColor &color)
{
    setBrush(color);
    setSelectedBrush(color);
}

void SEDPlotPointCustom::setVisible(bool on)
{
    setSelectable(on); // we are movable only when visible
    QCPItemEllipse::setVisible(on);
}

void SEDPlotPointCustom::setPos(double x, double y)
{
    mCenterTracer->position->setCoords(x, y);
    parentPlot()->replot();
}

void SEDPlotPointCustom::setEllipse(double smin, double smax, double a, double ar)
{
    semiMajorAxisLength = smin;
    semiMinorAxisLength = smax;
    angle = a;
    arcpix = ar;
}
