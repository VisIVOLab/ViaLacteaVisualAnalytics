#include "sedplotpointcustom.h"
#include <QMouseEvent>
#include <QToolTip>
#include "vtkellipse.h"
#include "vtkCleanPolyData.h"
#include "vialacteastringdictwidget.h"
#include "singleton.h"

SEDPlotPointCustom::SEDPlotPointCustom(QCustomPlot *parentPlot, double halfSize, vtkwindow_new *v)
    : QCPItemEllipse(parentPlot)
    , mCenterTracer(new QCPItemTracer(parentPlot))
    , mGripDelta()
    , mInitialPos()
    , mLastWantedPos()
    , mMoveTimer(new QTimer(this))
    , mCurWantedPosPx()
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

    connect(this,SIGNAL(selectionChanged(bool)),this, SLOT(selected(bool)));

}

void SEDPlotPointCustom::selected(bool s)
{
    vtkEllipse *el;
    if(s)
    {

        qDebug()<<"semiMajorAxisLength: "<<semiMajorAxisLength;
        qDebug()<<"semiMinorAxisLength: "<<semiMinorAxisLength;
        qDebug()<<"angle: "<<angle;
        qDebug()<<"image_x: "<<image_x;
        qDebug()<<"image_y: "<<image_y;
        qDebug()<<"arcpix: "<<arcpix;
        qDebug()<<"designation: "<<designation;
        qDebug()<<"error: "<<designation;

        if(image_x!=0 && image_y!=0 && designation.compare("")!=0)
        {
            QString wav=QString::number(pos().x());
            qDebug()<<"vlkb_compactsources.sed_view_final.designation."<<wav <<" "<<stringDictWidget->getColUtypeStringDict().value("vlkb_compactsources.sed_view_final.designation"+wav);

            //QToolTip::showText(QCursor::pos(), QString("designation: %1\nwavelength: %2\nfint: %3\nerr_fint: %8\nglon: %4\nglat: %5\nx: %6\ny:%7").arg(designation).arg(pos().x()).arg(pos().y()).arg(glon).arg(glat).arg(image_x).arg(image_y).arg(error_flux));
            QToolTip::showText(QCursor::pos(), QString(stringDictWidget->getColUtypeStringDict().value("vlkb_compactsources.sed_view_final.designation"+wav)+
                                                       ":\n%1\nwavelength: %2\nfint: %3\nerr_fint: %8\nglon: %4\nglat: %5\nx: %6\ny:%7").arg(designation).arg(pos().x()).arg(pos().y()).arg(glon).arg(glat).arg(image_x).arg(image_y).arg(error_flux));
            el= new vtkEllipse(semiMajorAxisLength/2,semiMinorAxisLength/2,angle, image_x, image_y, arcpix, 0,0, designation, NULL);
            drawSingleEllipse(el);
        }
        else
        {
            QToolTip::showText(QCursor::pos(), QString("wavelength: %1\nfint: %2\nerr_fin: %3").arg(pos().x()).arg(pos().y()).arg(error_flux));
        }
    }
    else
    {
        if(vtkwin!=0){
        vtkwin->removeSingleEllipse(ellipseActor);
        }
    }
    isSelected=s;
}

void SEDPlotPointCustom::drawSingleEllipse(vtkEllipse * ellipse )
{

    vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanFilter->SetInputData(ellipse->getPolyData());


    cleanFilter->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cleanFilter->GetOutputPort());

    ellipseActor = vtkSmartPointer<vtkLODActor>::New();
    ellipseActor->SetMapper(mapper);

    ellipseActor->GetProperty()->SetColor(0, 0, 0);

    if(vtkwin!=0){
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

void SEDPlotPointCustom::setColor(const QColor& color)
{
    setBrush(color);
    setSelectedBrush(color);
}

void SEDPlotPointCustom::startMoving(const QPointF &mousePos, bool shiftIsPressed)
{
    /*
    qDebug()<<"startMoving";
    mGripDelta.setX(parentPlot()->xAxis->coordToPixel(mCenterTracer->position->key()) - mousePos.x());
    mGripDelta.setY(parentPlot()->yAxis->coordToPixel(mCenterTracer->position->value()) - mousePos.y());

    mInitialPos = pos();
    mLastWantedPos = mInitialPos;
    mCurWantedPosPx = QPointF();
    mIsChangingOnlyOneCoordinate = shiftIsPressed;

    mMoveTimer->start();

    QCPItemStraightLine *horizontal = new QCPItemStraightLine(parentPlot());
    horizontal->setAntialiased(false);
    horizontal->point1->setCoords(mInitialPos.x(), mInitialPos.y());
    horizontal->point2->setCoords(mInitialPos.x() + 1, mInitialPos.y());
    parentPlot()->addItem(horizontal);

    QCPItemStraightLine *vertical = new QCPItemStraightLine(parentPlot());
    vertical->setAntialiased(false);
    vertical->point1->setCoords(mInitialPos.x(), mInitialPos.y());
    vertical->point2->setCoords(mInitialPos.x(), mInitialPos.y() + 1);
    parentPlot()->addItem(vertical);

    static const QPen linesPen(Qt::darkGray, 0, Qt::DashLine);
    horizontal->setPen(linesPen);
    vertical->setPen(linesPen);

    mHelperItems << vertical << horizontal;

    if (!mIsChangingOnlyOneCoordinate) {
        vertical->setVisible(false);
        horizontal->setVisible(false);
    }

    connect(parentPlot(), SIGNAL(mouseMove(QMouseEvent*)),
            this, SLOT(onMouseMove(QMouseEvent*)));

    connect(parentPlot(), SIGNAL(mouseRelease(QMouseEvent*)),
            this, SLOT(stopMoving()));

    connect(parentPlot(), SIGNAL(shiftStateChanged(bool)),
            this, SLOT(onShiftStateChanged(bool)));

    parentPlot()->grabKeyboard();
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);
    */
}

void SEDPlotPointCustom::setVisible(bool on)
{
    setSelectable(on);  // we are movable only when visible
    QCPItemEllipse::setVisible(on);
}

void SEDPlotPointCustom::stopMoving()
{
    /*
    disconnect(parentPlot(), SIGNAL(mouseMove(QMouseEvent*)),
            this, SLOT(onMouseMove(QMouseEvent*)));

    disconnect(parentPlot(), SIGNAL(mouseRelease(QMouseEvent*)),
            this, SLOT(stopMoving()));

    disconnect(parentPlot(), SIGNAL(shiftStateChanged(bool)),
            this, SLOT(onShiftStateChanged(bool)));

    mMoveTimer->stop();
    moveToWantedPos();

    if (!mHelperItems.isEmpty()) {
        while (!mHelperItems.isEmpty()) {
            QCPAbstractItem *item = mHelperItems.takeFirst();
            mParentPlot->removeItem(item);
        }

        mParentPlot->replot();
    }

    parentPlot()->releaseKeyboard();
    QApplication::restoreOverrideCursor();

    emit stoppedMoving();
    */
}

void SEDPlotPointCustom::setPos(double x, double y)
{
    mCenterTracer->position->setCoords(x, y);
    parentPlot()->replot();
}
void SEDPlotPointCustom::move(double x, double y, bool signalNeeded)
{
    /*
    mLastWantedPos.setX(x);
    mLastWantedPos.setY(y);
    if (mIsChangingOnlyOneCoordinate) {
        double x1 = parentPlot()->xAxis->coordToPixel(x);
        double x2 = parentPlot()->xAxis->coordToPixel(mInitialPos.x());
        double y1 = parentPlot()->yAxis->coordToPixel(y);
        double y2 = parentPlot()->yAxis->coordToPixel(mInitialPos.y());
        if (qAbs(x1 - x2) < qAbs(y1 - y2)) {
            x = mInitialPos.x();
        } else {
            y = mInitialPos.y();
        }
    }

    mCenterTracer->position->setCoords(x, y);

    parentPlot()->replot();

    if(signalNeeded) {
        emit moved(QPointF(x, y));
    }
    */
}

void SEDPlotPointCustom::movePx(double x, double y)
{
    move(parentPlot()->xAxis->pixelToCoord(x),
         parentPlot()->yAxis->pixelToCoord(y));
}

void SEDPlotPointCustom::setActive(bool isActive)
{
    /*
    qDebug()<<"setActive";

    setSelected(isActive);
    emit (isActive ? activated() : disactivated());
*/
}

bool SEDPlotPointCustom::selected(){
    return isSelected;
}

void SEDPlotPointCustom::onMousePress(QMouseEvent *event){
    selected(true);
    bool b=this->selected();
    qDebug()<<"Click on SEDPlotPointCustom "<<b;
}

void SEDPlotPointCustom::onMouseMove(QMouseEvent *event)
{
    /*

    qDebug()<<"mousemove";

    mCurWantedPosPx = QPointF(event->pos().x() + mGripDelta.x(),
                              event->pos().y() + mGripDelta.y());
                              */
}

void SEDPlotPointCustom::moveToWantedPos()
{
    /*
    if (!mCurWantedPosPx.isNull()) {
        movePx(mCurWantedPosPx.x(), mCurWantedPosPx.y());
        mCurWantedPosPx = QPointF();
    }
    */
}

void SEDPlotPointCustom::onShiftStateChanged(bool shiftPressed)
{
    if (shiftPressed != mIsChangingOnlyOneCoordinate) {
        mIsChangingOnlyOneCoordinate = shiftPressed;
        foreach (QCPAbstractItem *item, mHelperItems) {
            item->setVisible(shiftPressed);
        }
        move(mLastWantedPos.x(), mLastWantedPos.y());
    }
}


void  SEDPlotPointCustom::setEllipse(double smin, double smax, double a, double ar)
{
    semiMajorAxisLength= smin;
    semiMinorAxisLength=smax;
    angle=a;
    arcpix=ar;
}
