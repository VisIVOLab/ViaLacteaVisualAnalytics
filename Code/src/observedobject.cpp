#include "observedobject.h"
#include"treeitem.h"

ObservedObject::ObservedObject()
{
    m_OriginType = TreeItem::Null;
    setShowAxes(true);
    setShowBox(true);
    setName("Visual Object");
    m_isSetOrigin=false;
    m_OriginType = TreeItem::TreeItemType(NULL);
}
void ObservedObject::setShowAxes(bool value)
{
    m_ShowAxes=value;
}
void ObservedObject::setShowBox(bool value)
{
    m_ShowBox=value;
}
void ObservedObject::setName(QString name)
{
    m_Name=name;
}
bool ObservedObject::isOriginSpecified()
{
    return  m_isSetOrigin;
}
bool ObservedObject::isVisible()
{
    return m_isVisible;
}
QString ObservedObject::getName()
{
    return m_Name;
}
bool ObservedObject::isEnabledShowAxes()
{
    return m_ShowAxes;
}
bool ObservedObject::isEnabledShowBox()
{
    return m_ShowBox;
}
