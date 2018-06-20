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



#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QStandardItemModel>
#include <QModelIndex>
#include <QVariant>
#include "treeitem.h"
#include <vtkSmartPointer.h>
#include "vtkfitsreader.h"

class  VisPoint;
class TreeItem;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT
    QModelIndexList m_activeViewIndexList;
public:
    TreeModel(const QStringList &headers, const QString &data,
              QObject *parent = 0);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role = Qt::EditRole);

    bool insertColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex());
    bool removeColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex());
    bool insertRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex());
    QModelIndexList getActiveViewList();
    void activeViewListTraverse(QModelIndex index);

    /*VisIVO*/
    bool setTable(const QModelIndex &index, VSTableDesktop *table);
    bool setVTI(const QModelIndex &index, vtkVolume *volume);
    bool setVTP(const QModelIndex &index, vtkLODActor *pActor);
    bool setVisualObject(const QModelIndex &index, VisPoint* vis );
   // bool setFITSIMG(const QModelIndex &index, vtkSmartPointer<vtkImageActor> imgActor);
    bool setFITSIMG(const QModelIndex &index, vtkSmartPointer<vtkFitsReader> fitsReader);
    VisPoint* getVisualObject(const QModelIndex &index);
    VSTableDesktop *getTable(const QModelIndex &index);
    vtkLODActor *getVTP(const QModelIndex &index);
    vtkVolume *getVTI(const QModelIndex &index);
    vtkSmartPointer<vtkFitsReader> getFITSIMG(const QModelIndex &index);

   // vtkImageActor *getFITSIMG(const QModelIndex &index);
    void setupModelData(const QStringList &lines, TreeItem *parent);
    TreeItem *getItem(const QModelIndex &index) const;
    TreeItem::TreeItemType getType(const QModelIndex &index);
    QModelIndex  getLastInsertItem();
    void  setLastInsertItem(QModelIndex item);



private:

    TreeItem *rootItem;
    QModelIndex  lastInsertItem;
};

#endif



