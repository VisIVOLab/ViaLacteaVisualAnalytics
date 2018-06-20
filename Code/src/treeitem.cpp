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

/*
    treeitem.cpp

    A container for items of data supplied by the simple tree model.
*/

#include <QStringList>

#include "treeitem.h"
#include "vispoint.h"

TreeItem::TreeItem(const QVector<QVariant> &data, TreeItem *parent)
{
    m_parentItem = parent;
    itemData = data;
}

TreeItem::~TreeItem()
{
    qDeleteAll(childItems);
}

TreeItem *TreeItem::child(int number)
{
    return childItems.value(number);
}

int TreeItem::childCount() const
{
    return childItems.count();
}

int TreeItem::childNumber() const
{
    if (m_parentItem)
        return m_parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

int TreeItem::columnCount() const
{
    return itemData.count();
}

QVariant TreeItem::data(int column) const
{
    return itemData.value(column);
}

bool TreeItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > childItems.size())
        return false;

    for (int row = 0; row < count; ++row) {
        QVector<QVariant> data(columns);
        TreeItem *item = new TreeItem(data, this);
        childItems.insert(position, item);
    }

    return true;
}

bool TreeItem::insertColumns(int position, int columns)
{
    if (position < 0 || position > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.insert(position, QVariant());

    foreach (TreeItem *child, childItems)
        child->insertColumns(position, columns);

    return true;
}

TreeItem *TreeItem::parent()
{
    return m_parentItem;
}

bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}

bool TreeItem::removeColumns(int position, int columns)
{
    if (position < 0 || position + columns > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.remove(position);

    foreach (TreeItem *child, childItems)
        child->removeColumns(position, columns);

    return true;
}

bool TreeItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= itemData.size())
        return false;

    itemData[column] = value;
    return true;
}
bool TreeItem::setTable(VSTableDesktop *table)
{
    if (table->tableExist()){
        m_table = table;
        if(table->getIsVolume())
        {
            m_Type=VolumeTable;
        }
        else
        {
            m_Type=PointTable;
        }
        return true;
    }
    else return false;
}

bool TreeItem::setVisualObject(VisPoint *vis)
{
    if (vis->isOriginSpecified()){
        m_Type=VisualObject;
        m_visualObject=vis;
        return true;
    }
    else return false;
}
bool TreeItem::setVTI(vtkVolume *volume)
{
    m_volume=volume;
    m_Type=VTI;
    return true;
}

bool TreeItem::setVTP(vtkLODActor *pActor)
{
    m_pActor=pActor;
    m_Type=VTP;
    return true;
}

//bool TreeItem::setFITSIMG(vtkImageActor *pActor)
bool TreeItem::setFITSIMG(vtkSmartPointer<vtkFitsReader> fitsReader)
{

    m_Type=FITSIMAGE;
    m_fitsImg= fitsReader;
    return true;
}

VSTableDesktop *TreeItem::getTable()
{
    return m_table;
}

VisPoint *TreeItem::getVisualObject()
{
    return m_visualObject;
}


//vtkImageActor *TreeItem::getFITSIMG()
vtkSmartPointer<vtkFitsReader> TreeItem::getFITSIMG()
{

    return m_fitsImg;
}

vtkVolume *TreeItem::getVTI()
{
    return m_volume;
}

vtkLODActor *TreeItem::getVTP()
{
    return m_pActor;
}

TreeItem::TreeItemType TreeItem::getType()
{
    return m_Type;
}
