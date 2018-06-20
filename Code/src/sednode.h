#ifndef SEDNODE_H
#define SEDNODE_H

#include <QObject>
#include <QHash>
#include <QDataStream>

class SEDNode : public QObject
{
    Q_OBJECT
public:
    explicit SEDNode(QObject *parent = 0);
    void setDesignation(QString d){designation=d;}
    QString getDesignation(){return designation;}
    void setChild(SEDNode *node);
    void setParent(SEDNode *node);


    QHash < QString,SEDNode*> getChild(){return childNodes;}
    QHash < QString,SEDNode*> getParent(){return parentNodes;}
    void setFlux(double f);
    void setErrFlux(double f);
    double getFlux() {return flux;}
    double getErrFlux() {return e_flux;}
    double getWavelength() {return wavelength;}
    void setWavelength(double w ) {wavelength=w;}
    bool hasChild();

    void setSky(float lon, float lat);
    void setXY(double x, double y);
    float getLon() {return glon;}
    float getLat() {return glat;}
    double getX() {return image_x;}
    double getY() {return image_y;}

    double getSemiMajorAxisLength() {return semiMajorAxisLength;}
    double getSemiMinorAxisLength() {return semiMinorAxisLength;}
    double getAngle() {return angle;}
    double getArcpix() {return arcpixel;}

    void setEllipse( double smin, double smax,double a,double ar);
    QString designation;


signals:

public slots:


private:
    QHash < QString,SEDNode*> parentNodes;
    QHash < QString,SEDNode*> childNodes;
    double flux;
    double e_flux;
    double wavelength;

    float glon;//=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("GLON")][j].c_str());
    float glat;//dec=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("GLAT")][j].c_str());
    double image_x;
    double image_y;

    double semiMajorAxisLength;
    double semiMinorAxisLength;
    double angle;
    double arcpixel;

};

QDataStream &operator<<(QDataStream& out, SEDNode* node);
QDataStream &operator>>(QDataStream& in, SEDNode* node);

#endif // SEDNODE_H
