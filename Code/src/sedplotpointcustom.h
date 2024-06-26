#ifndef SEDPLOTPOINTCUSTOM_H
#define SEDPLOTPOINTCUSTOM_H

#include "qcustomplot.h"
#include "sednode.h"
#include "vialacteastringdictwidget.h"
#include "vtkellipse.h"
#include "vtkwindow_new.h"

#include <QApplication>
#include <QMouseEvent>
#include <QTimer>

class SEDPlotPointCustom : public QCPItemEllipse
{
    Q_OBJECT
public:
    explicit SEDPlotPointCustom(QCustomPlot *parentPlot, double halfSize = 5,
                                vtkwindow_new *v = NULL);

    QPointF pos() const;
    const QColor &color() const;
    void setColor(const QColor &color);
    void startMoving(const QPointF &mousePos, bool shiftIsPressed);
    void setDesignation(QString d) { designation = d; }
    void setLon(double lon) { glon = lon; }
    void setLat(double lat) { glat = lat; }
    void setX(double x) { image_x = x; }
    void setY(double y) { image_y = y; }
    void setErrorFlux(double ef) { error_flux = ef; }
    void setNode(SEDNode *n) { sedNode = n; }

    double getSemiMajorAxisLength() { return semiMajorAxisLength; }
    double getSemiMinorAxisLength() { return semiMinorAxisLength; }
    double getAngle() { return angle; }
    SEDNode *getNode() { return sedNode; }

    void setEllipse(double smin, double smax, double a, double ar);

public slots:
    void setVisible(bool on);

signals:
    void activated(); ///< emitted on mouse over
    void disactivated(); ///< emitted when cursor leave us

    void moved(const QPointF &pos);
    void stoppedMoving();

public slots:
    void setPos(double x, double y);

private:
    QCPItemTracer *mCenterTracer;
    QPointF mGripDelta;
    QPointF mInitialPos;
    bool mIsChangingOnlyOneCoordinate;
    QList<QCPAbstractItem *> mHelperItems;
    QPointF mLastWantedPos;
    QTimer *mMoveTimer;
    QPointF mCurWantedPosPx;
    vtkwindow_new *vtkwin;
    QString designation;

    double image_x;
    double image_y;
    double glon;
    double glat;
    double error_flux;

    double semiMajorAxisLength;
    double semiMinorAxisLength;
    double angle;
    double arcpix;

    SEDNode *sedNode;

    VialacteaStringDictWidget *stringDictWidget;
};

#endif // SEDPLOTPOINTCUSTOM_H
