/***************************************************************************
 *   Copyright (C) 2010 by Alessandro Costa                                *
 *   alessandro.costa@oact.inaf.it                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
class TreeModel;

#ifndef VISIVOIMPORTERDESKTOP_H
#define VISIVOIMPORTERDESKTOP_H

/**
        @author Alessandro Costa <alessandro.costa@oact.inaf.it>
        @author Ugo Becciani <ugo.becciani@oact.inaf.it>
*/

#include <QWidget>
#include "vstabledesktop.h"
#include <QFileInfo>
#include <QProcess>

#include "operationqueue.h"
#include "vtkwindow_new.h"
#include "sednode.h"
#include "sed.h"

class VisIVOImporterDesktop : public QWidget
{
    Q_OBJECT
public:
    VisIVOImporterDesktop(QString t, QString f, TreeModel * m=NULL);
    VisIVOImporterDesktop(QString f, TreeModel * m=NULL, bool isBandMergedCatalogue=false, vtkwindow_new *v=NULL, QString wavelen="");
    VisIVOImporterDesktop(QString f, vtkwindow_new* v, bool isFilament=true, bool isBubble=false);
    void doImport(QString wavelen="", bool usingAPI=false);
    int getStatus();
    VSTableDesktop *getTable();
    void setVtkWin(vtkwindow_new* v);
    void setBm(bool bm);



signals:

private:
    //QFileInfo infoFile;
    int m_impStatus;
    VSTableDesktop *m_VisIVOTable;
    TreeModel * model;
    QString type;
    QString fileName;
    QString headerFileName;
    QString binaryFileName;
    QFile *headerFile;
    QFile *importedFile;
    QFile *binaryFile;
    QFileInfo infoFile;
    OperationQueue *queue;
    int operation_queue_row;
    QString wavelength;
    bool isHigal;
    bool isBandMergedCatalogue;
    QString outputName_atlas;
    QString outputName_designation;
    vtkwindow_new *vtkwin;
    QHash < QString,SEDNode*> nodelist;
    SEDNode* bmContainsNode(QString nodename);
    QList <SED* > sedlist;
    bool firstSEDNode;

    QHash < int,SED*> sedMap;

public slots:

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

};


#endif // VISIVOIMPORTERDESKTOP_H
