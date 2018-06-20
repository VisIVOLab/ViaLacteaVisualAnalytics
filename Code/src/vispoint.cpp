#include "vispoint.h"
#include "vstabledesktop.h"
#include "treeitem.h"


VisPoint::VisPoint()
{
    setX("");
    setY("");
    setZ("");
    setVectorX("");
    setVectorY("");
    setVectorZ("");
    setLutScalar("");
    setRadiusScalar("");
    setHeightScalar("");
    setShowPoints(true);
    setShowVectors(false);
    setShowAxes(true);
    setScale(true);
    setShowBox(true);
    setLogX(false);
    setLogY(false);
    setLogZ(false);
    setName("Visual Object");
    m_Table=NULL;
    m_isSetOrigin=false;
}
VisPoint::VisPoint(VSTableDesktop *table)
{
    m_OriginType = TreeItem::PointTable;
    setX("");
    setY("");
    setZ("");
    setVectorX("");
    setVectorY("");
    setVectorZ("");
    setLutScalar("");
    setRadiusScalar("");
    setHeightScalar("");
    setShowPoints(true);
    setShowVectors(false);
    setShowAxes(true);
    setScale(true);
    setShowBox(true);
    setLogX(false);
    setLogY(false);
    setLogZ(false);
    setName(QString(table->getName().c_str()));
    m_Table=table;
    m_isSetOrigin=true;
}
void  VisPoint::setX(QString value)
{
    m_X=value;
}
void  VisPoint::setY(QString value)
{
    m_Y=value;
}
void  VisPoint::setZ(QString value)
{
    m_Z=value;
}
void  VisPoint::setVectorX(QString value)
{
    m_VectorX=value;
}
void  VisPoint::setVectorY(QString value)
{
    m_VectorY=value;
}
void  VisPoint::setVectorZ(QString value)
{
    m_VectorZ=value;
}
void  VisPoint::setLutScalar(QString value)
{
    m_LutScalar=value;
}
void  VisPoint::setRadiusScalar(QString value)
{
    m_RadiusScalar=value;
}
void  VisPoint::setHeightScalar(QString value)
{
    m_HeightScalar=value;
}
void  VisPoint::setLut(vtkLookupTable* lut)
{
    m_lut=lut;
}


void VisPoint::setShowPoints(bool value)
{
    m_ShowPoints=value;
}
void VisPoint::setShowVectors(bool value)
{
    m_ShowVectors=value;
}
void VisPoint::setScale(bool value)
{
    m_Scale=value;
}
void VisPoint::setLogX(bool value)
{
    m_LogX=value;
}
void VisPoint::setLogY(bool value)
{
    m_LogY=value;
}
void VisPoint::setLogZ(bool value)
{
    m_LogY=value;
}
QString  VisPoint::getX()
{
    return m_X;
}
QString  VisPoint::getY()
{
    return m_Y;
}
QString  VisPoint::getZ()
{
    return m_Z;
}
QString  VisPoint::getVectorX()
{
    return m_VectorX;
}
QString  VisPoint::getVectorY()
{
    return m_VectorY;
}
QString VisPoint::getVectorZ()
{
    return m_VectorZ;
}
QString VisPoint::getLutScalar()
{
    return m_LutScalar;
}
QString VisPoint::getRadiusScalar()
{
    return m_RadiusScalar;
}
QString VisPoint::getHeightScalar()
{
    return m_HeightScalar;
}
bool VisPoint::isEnabledShowPoints()
{
    return  m_ShowPoints;
}
bool VisPoint::isEnabledShowVectors()
{
    return  m_ShowVectors;
}
bool VisPoint::isEnabledScale()
{
    return  m_Scale;
}

bool VisPoint::isEnabledLogX()
{
    return  m_LogX;
}
bool VisPoint::isEnabledLogY()
{
    return  m_LogY;
}
bool VisPoint::isEnabledLogZ()
{
    return  m_LogZ;
}
VSTableDesktop *VisPoint::getOrigin()
{
    return m_Table;
}
