#ifndef OBSERVEDOBJECT_H
#define OBSERVEDOBJECT_H
#include"treeitem.h"

class ObservedObject
{


public:
    ObservedObject();
    QString m_Name;
    bool m_isVisible;
    bool m_isSetOrigin;
    bool m_ShowAxes;
    bool m_ShowBox;
    TreeItem::TreeItemType m_OriginType;
    bool isOriginSpecified();
    bool isVisible();
    void setName(QString name);
    TreeItem::TreeItemType getOriginType();
    void setShowAxes(bool value);
    void setShowBox(bool value);

    QString getName();
    bool isEnabledShowBox();
    bool isEnabledShowAxes();
};

#endif // OBSERVEDOBJECT_H
