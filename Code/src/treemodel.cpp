/*
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (qt-info@nokia.com)
 *
 * This file is part of the examples of the Qt Toolkit.
 *
 * $QT_BEGIN_LICENSE:LGPL$
 * Modified by Alessandro Costa 2011
 *
 */

#include <QtGui>

#include "treeitem.h"
#include "treemodel.h"
#include "vispoint.h"


TreeModel::TreeModel(const QStringList &headers, const QString &data,
                     QObject *parent)
    : QAbstractItemModel(parent)
{
    /**
 * Constructor.
 *
 * The constructor creates a root item and initializes
 * it with the header data supplied
 *
 * @param headers   headers for the object initialization
 * @param data textual data being converted to a data structure
 * we can use with the model
 * @param parent
 *
 */
    QVector<QVariant> rootData;
    foreach (QString header, headers)
        rootData << header;

    rootItem = new TreeItem(rootData);
    setupModelData(data.split(QString("\n")), rootItem);
    m_activeViewIndexList = QModelIndexList();
}

TreeModel::~TreeModel()
{
    delete rootItem;
}

QModelIndex TreeModel::getLastInsertItem()
{
    return lastInsertItem;
}

void  TreeModel::setLastInsertItem(QModelIndex item)
{
    lastInsertItem=item;
}

int TreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::DecorationRole)
        return QVariant();

    TreeItem *item = getItem(index);

    return item->data(index.column());
}



TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item) return item;
    }
    return rootItem;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    TreeItem *parentItem = getItem(parent);

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginInsertColumns(parent, position, position + columns - 1);
    success = rootItem->insertColumns(position, columns);
    endInsertColumns();

    return success;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, rootItem->columnCount());
    endInsertRows();



    return success;
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginRemoveColumns(parent, position, position + columns - 1);
    success = rootItem->removeColumns(position, columns);
    endRemoveColumns();

    if (rootItem->columnCount() == 0)
        removeRows(0, rowCount());

    return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem = getItem(parent);

    return parentItem->childCount();
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value,
                        int role)
{
    if (role != Qt::EditRole)
        return false;

    TreeItem *item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    bool result = rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

void TreeModel::setupModelData(const QStringList &lines, TreeItem *parent)
{
    QList<TreeItem*> parents;
    QList<int> indentations;
    parents << parent;
    indentations << 0;

    int number = 0;

    while (number < lines.count()) {
        int position = 0;
        while (position < lines[number].length()) {
            if (lines[number].mid(position, 1) != " ")
                break;
            position++;
        }

        QString lineData = lines[number].mid(position).trimmed();

        if (!lineData.isEmpty()) {
            // Read the column data from the rest of the line.
            QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
            QVector<QVariant> columnData;
            for (int column = 0; column < columnStrings.count(); ++column)
                columnData << columnStrings[column];

            if (position > indentations.last()) {
                // The last child of the current parent is now the new parent
                // unless the current parent has no children.

                if (parents.last()->childCount() > 0) {
                    parents << parents.last()->child(parents.last()->childCount()-1);
                    indentations << position;
                }
            } else {
                while (position < indentations.last() && parents.count() > 0) {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }

            // Append a new item to the current parent's list of children.
            TreeItem *parent = parents.last();
            parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
            for (int column = 0; column < columnData.size(); ++column)
                parent->child(parent->childCount() - 1)->setData(column, columnData[column]);
        }

        number++;
    }
}
//****************

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    /*!
    Qt.ItemFlag
    This enum describes the properties of an item:

    Qt:ItemIsSelectable          	It can be selected.
    Qt:ItemIsEditable            	It can be edited.
    Qt:ItemIsDragEnabled         	It can be dragged.
    Qt:ItemIsDropEnabled         	It can be used as a drop target.
    Qt:ItemIsUserCheckable       	It can be checked or unchecked by the user.
    Qt:ItemIsEnabled             	The user can interact with the item.
    Qt:ItemIsTristate            	The item is checkable with three separate states.
*/
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;// | Qt::ItemIsUserCheckable;
}
bool TreeModel::setTable(const QModelIndex &index, VSTableDesktop *table)
{
    TreeItem *item = getItem(index);
    bool result = item->setTable(table);
    if (result)
        emit dataChanged(index, index);
    return result;
}

bool TreeModel::setVisualObject(const QModelIndex &index, VisPoint *vis)
{
    TreeItem *item = getItem(index);
    bool result =item->setVisualObject(vis);
    if (result)
        emit dataChanged(index, index);
    return result;
}

bool TreeModel::setVTP(const QModelIndex &index, vtkLODActor *pActor)
{
    TreeItem *item = getItem(index);
    bool result = item->setVTP(pActor);
    if (result)
        emit dataChanged(index, index);
    return result;
}

//bool TreeModel::setFITSIMG(const QModelIndex &index, vtkSmartPointer<vtkImageActor> imgActor)
bool TreeModel::setFITSIMG(const QModelIndex &index, vtkSmartPointer<vtkFitsReader> fitsReader)
{
    TreeItem *item = getItem(index);

    bool result = item->setFITSIMG(fitsReader);

    if (result)
        emit dataChanged(index, index);
    return result;
}

bool TreeModel::setVTI(const QModelIndex &index, vtkVolume *volume)
{
    TreeItem *item = getItem(index);
    bool result = item->setVTI(volume);
    if (result)
        emit dataChanged(index, index);
    return result;
}

VSTableDesktop *TreeModel::getTable(const QModelIndex &index)
{
    TreeItem *item = getItem(index);
    if (item->getType() == TreeItem::VisualObject) return item->getVisualObject()->getOrigin();
    return item->getTable();
}

VisPoint*TreeModel::getVisualObject(const QModelIndex &index)
{
    TreeItem *item = getItem(index);
    return item->getVisualObject();
}
vtkLODActor *TreeModel::getVTP(const QModelIndex &index)
{
    TreeItem *item = getItem(index);
    return item->getVTP();
}

//vtkImageActor *TreeModel::getFITSIMG(const QModelIndex &index)

vtkSmartPointer<vtkFitsReader> TreeModel::getFITSIMG(const QModelIndex &index)
{
    TreeItem *item = getItem(index);
    return item->getFITSIMG();
}

vtkVolume *TreeModel::getVTI(const QModelIndex &index)
{
    TreeItem *item = getItem(index);
    return item->getVTI();
}

TreeItem::TreeItemType TreeModel::getType(const QModelIndex &index)
{
    TreeItem *item = getItem(index);
    return item->getType();
}
QModelIndexList TreeModel::getActiveViewList()
{
    /**
      * It returns a QModelIndexList containing
      * the whole list of VisIVOObject with m_isSetOrigin == true
      * this list is passed to the rendering window (VisIVORW) for
      * the visualization
      */

    m_activeViewIndexList.clear();
    int rows = this->rowCount();
    for (int i=0;i<rows;i++)
    {
        activeViewListTraverse(this->index(i,0));
    }
    return m_activeViewIndexList;
}
void TreeModel::activeViewListTraverse(QModelIndex index)
{
    TreeItem *currentItem = this->getItem(index);
    TreeItem::TreeItemType currentType = this->getItem(index)->getType();

    if (currentType==TreeItem::VisualObject)
    {
        if(currentItem->getVisualObject()->isVisible()) // if the index item is a VisualObject;
            m_activeViewIndexList.append(index);
    }
    int rows = this->getItem(index)->childCount();
    if (rows > 0)
    {
        for (int i=0;i<rows;i++)
        {
            //TreeItem nextItem = this->getItem(index)->child(i);
            QModelIndex  nextIndex = this->index(i,0,index);
            activeViewListTraverse(nextIndex);
        }
    }
}

