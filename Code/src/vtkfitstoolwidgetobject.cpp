#include "vtkfitstoolwidgetobject.h"
#include <QFileInfo>

/*
 * type
 * 0 = image
 * 1 = sigleband
 * 2 = centroid
 * 3 = isocontour
*/

vtkfitstoolwidgetobject::vtkfitstoolwidgetobject(int t)
{
    type=t;
    wavelength="";
    name="";
}

void vtkfitstoolwidgetobject::setTreeWidgetItem( QTreeWidgetItem *i)
{
    item=i;
}

void vtkfitstoolwidgetobject::setParentItem( vtkfitstoolwidgetobject *p)
{
    parent=p;
}

void vtkfitstoolwidgetobject::setName( QString n)
{
    if(type == 0)
    {
        path=n;
        QFileInfo fileInfo(n);
        name =fileInfo.fileName();
    }
    else
        name=n;
}

void vtkfitstoolwidgetobject::setWavelength(QString w)
{
    wavelength=w;
}

void vtkfitstoolwidgetobject::setActor(vtkSmartPointer<vtkLODActor> a)
{
    actor=a;
}


