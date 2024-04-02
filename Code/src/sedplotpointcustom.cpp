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

void SEDPlotPointCustom::selected(bool s)
{
    vtkEllipse *el;
    if (s) {

        if (image_x != 0 && image_y != 0 && designation.compare("") != 0) {
            QString wav = QString::number(pos().x());
            QToolTip::showText(
                    QCursor::pos(),
                    QString(stringDictWidget->getColUtypeStringDict().value(
                                    "vlkb_compactsources.sed_view_final.designation" + wav)
                            + ":\n%1\nwavelength: %2\nfint: %3\nerr_fint: %8\nglon: %4\nglat: "
                              "%5\nx: %6\ny:%7")
                            .arg(designation)
                            .arg(pos().x())
                            .arg(pos().y())
                            .arg(glon)
                            .arg(glat)
                            .arg(image_x)
                            .arg(image_y)
                            .arg(error_flux));
            el = new vtkEllipse(semiMajorAxisLength / 2, semiMinorAxisLength / 2, angle, image_x,
                                image_y, arcpix, 0, 0, designation, NULL);
            qDebug() << "++"
                     << "semiMajorAxisLength:" << semiMajorAxisLength / 2
                     << "semiMinorAxisLength:" << semiMinorAxisLength / 2 << "angle:" << angle
                     << "image_x:" << image_x << "image_y:" << image_y << "arcpix:" << arcpix
                     << "designation:" << designation;
            drawSingleEllipse(el);
        } else {
            QToolTip::showText(QCursor::pos(),
                               QString("wavelength: %1\nfint: %2\nerr_fin: %3")
                                       .arg(pos().x())
                                       .arg(pos().y())
                                       .arg(error_flux));
        }
    } else {
        if (vtkwin != 0) {
            vtkwin->removeSingleEllipse(ellipseActor);
        }
    }
    isSelected = s;
}

void SEDPlotPointCustom::drawSingleEllipse(vtkEllipse *ellipse)
{

    vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanFilter->SetInputData(ellipse->getPolyData());
    cleanFilter->Update();
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cleanFilter->GetOutputPort());
    ellipseActor = vtkSmartPointer<vtkLODActor>::New();
    ellipseActor->SetMapper(mapper);
    ellipseActor->GetProperty()->SetColor(0, 0, 0);

    if (vtkwin != 0) {
        vtkwin->drawSingleEllipse(ellipseActor);
    }
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

bool SEDPlotPointCustom::selected()
{
    return isSelected;
}

void SEDPlotPointCustom::onMousePress(QMouseEvent *event)
{
    selected(true);
}

void SEDPlotPointCustom::setEllipse(double smin, double smax, double a, double ar)
{
    semiMajorAxisLength = smax;
    semiMinorAxisLength = smin;
    angle = a;
    arcpix = ar;
}
