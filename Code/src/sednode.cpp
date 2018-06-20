#include "sednode.h"
#include <qdebug.h>
#include <QDataStream>

SEDNode::SEDNode(QObject *parent) :
    QObject(parent)
{

}

void SEDNode::setSky(float lon, float lat)
{

    glon=lon;
    glat=lat;

}

void SEDNode::setXY(double x, double y)
{
    image_x=x;
    image_y=y;
}

void SEDNode::setChild(SEDNode *node)
{
    childNodes.insert(node->getDesignation(),node);
}

void SEDNode::setParent(SEDNode *node)
{
    parentNodes.insert(node->getDesignation(),node);
}

void SEDNode::setErrFlux(double f)
{
    e_flux=f;
}

void SEDNode::setFlux(double f)
{
    flux=f;
}

bool SEDNode::hasChild()
{
    return childNodes.count()>0;
}

void  SEDNode::setEllipse( double smin, double smax,double a, double ar)
{


    semiMajorAxisLength= smax;
    semiMinorAxisLength=smin;
    angle=a;
    arcpixel=ar;

}

QDataStream &operator<<(QDataStream &out, SEDNode* node)
{
    qDebug()<< "printing SEDNode";
    out << node->getDesignation();
    qDebug()<<node->getDesignation();
    out << node->getWavelength();
    out << node->getFlux();
    out << node->getErrFlux();
    out << node->getLat();
    out << node->getLon();
    return out;
}


QDataStream &operator>>(QDataStream &in, SEDNode* node)
{
    qDebug()<< "reading SEDNode";
    QString d;
    int w;
    double f, ef;
    float lat, lon;
    in >> d;
    in >> w;
    in >> f;
    in >> ef;
    in >> lat;
    in >> lon;

    //node=new SEDNode();
    node->setDesignation(d);
    node->setWavelength(w);
    node->setFlux(f);
    node->setErrFlux(ef);
    node->setSky(lon, lat);
    qDebug()<<node->getDesignation();
    //in >> node->getWavelength();
    //in >> node->getFlux();
    return in;
}


