/****************************************************************************
 **
 ** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 ** All rights reserved.
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL$
 ** Commercial Usage
 ** Licensees holding valid Qt Commercial licenses may use this file in
 ** accordance with the Qt Commercial License Agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and Nokia.
 **
 ** GNU Lesser General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU Lesser
 ** General Public License version 2.1 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.LGPL included in the
 ** packaging of this file.  Please review the following information to
 ** ensure the GNU Lesser General Public License version 2.1 requirements
 ** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 **
 ** In addition, as a special exception, Nokia gives you certain additional
 ** rights.  These rights are described in the Nokia Qt LGPL Exception
 ** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
 **
 ** GNU General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU
 ** General Public License version 3.0 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL included in the
 ** packaging of this file.  Please review the following information to
 ** ensure the GNU General Public License version 3.0 requirements will be
 ** met: http://www.gnu.org/copyleft/gpl.html.
 **
 ** If you have questions regarding the use of this file, please contact
 ** Nokia at qt-info@nokia.com.
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#ifndef TREEITEM_H
#define TREEITEM_H

#include "vstabledesktop.h"
#include "vtkfitsreader.h"
#include "vtkPVLODActor.h"
#include "vtkSmartPointer.h"
#include "vtkVolume.h"

#include <QList>
#include <QVariant>
#include <QVector>

class VisPoint;

class TreeItem
{
public:
    TreeItem(const QVector<QVariant> &data, TreeItem *parent = 0);
    enum TreeItemType { Null, PointTable, VolumeTable, VTI, VTP, VisualObject, FITSIMAGE };

    ~TreeItem();

    TreeItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    bool insertChildren(int position, int count, int columns);
    bool insertColumns(int position, int columns);
    TreeItem *parent();
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int childNumber() const;
    bool setData(int column, const QVariant &value);

    /*VisIVOItem specific method */
    bool setTable(VSTableDesktop *table);
    bool setVTP(vtkPVLODActor *pActor);
    //   bool setFITSIMG(vtkImageActor *pActor);
    bool setFITSIMG(vtkSmartPointer<vtkFitsReader> pActor);
    bool setVTI(vtkVolume *volume);
    bool setVisualObject(VisPoint *vis);
    VSTableDesktop *getTable();
    vtkPVLODActor *getVTP();
    vtkVolume *getVTI();
    // vtkImageActor *getFITSIMG();
    vtkSmartPointer<vtkFitsReader> getFITSIMG();
    VisPoint *getVisualObject();
    /*end VisIVOItem specific method */

private:
    QList<TreeItem *> childItems;
    QVector<QVariant> itemData;
    /*VisIVOItem specific*/
    enum TreeItemType m_Type;
    TreeItem *m_parentItem;
    VSTableDesktop *m_table;
    vtkPVLODActor *m_pActor;
    // vtkImageActor *m_imgActor;
    vtkSmartPointer<vtkFitsReader> m_fitsImg;
    vtkVolume *m_volume;
    VisPoint *m_visualObject;
    /*end VisIVOItem specific*/
public:
    enum TreeItemType getType();
};

#endif
