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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H
/**
        @author Alessandro Costa <alessandro.costa@oact.inaf.it>
*/

#include <QMainWindow>
#include <QProcess>
#include "ui_mainwindow.h"
#include <QFile>
#include <QString>
#include <QDir>
#include <map>
#include "vstabledesktop.h"
#include "treemodel.h"
#include "treeitem.h"
#include "vispoint.h"
#include "operationqueue.h"

class vtkwindow_new;

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QString workingPath;
    QString importer;
    QString filter;
    QString util;
    void setCallingVtkWindow(vtkwindow_new *v){myCallingVtkWindow=v;}
    Ui::MainWindow *ui;
    QString selectedFile;
    TreeModel *m_VisIVOTreeModel;

public slots:
    void importAscii();
    void importAscii(QString fileName, QString wavelen="", bool higal=false, bool bm=false, vtkwindow_new *v=NULL);
    void importAsciiFilaments(QString fileName, vtkwindow_new *v);
    void importAsciiBubbles(QString fileName, vtkwindow_new *v);
    void importAscii3dSelection(QString fileName, vtkwindow_new *v);
    void genericImport(int t);
    void importCSV(QString fileName);
    void importVOTable(QString fileName);
    void importBinaryTable(QString fileName);
    void setSelectedSurveyMap(QList < QPair<QString, QString> > s){selectedSurvey=s;}


    void importBinary();
    void importVoTable();
    void importVTP();
    void importVTI();
    void viewSetting();
    void switchTab(int index );
    void viewSettingOk();
    void itemSelected(QList<QMap<QString, QString> > elementsOnDb=QList<QMap<QString, QString> >(), bool layer=false);
    void importFitsImage(QString filename, QList<QMap<QString, QString> > elementsOnDb=QList<QMap<QString, QString> >(), QString l="", QString b="", QString r="", QString db="", QString dl="", bool layer=false);
    TreeModel* getTreeModel();
    vtkwindow_new* getLastVtkWin(){return m_OldRenderingWindow;}
    void on_actionTEST_DC3D_triggered();
    void importFitsDC(QString fileName);

    void setSurvey(QString s){survey=s;}
    void setSpecies(QString s){species=s;}
    void setTransition(QString t){transition=t;}

private slots:
    void on_actionOperation_queue_triggered();
    void on_actionHi_Gal_triggered();
    void on_actionImportFitsImage_triggered();
    void on_actionVialactea_triggered();
    void on_actionDatacube_triggered();
    void on_actionCsv_triggered();

    void on_volumeRadioButton_toggled(bool checked);

    void on_importPushButton_clicked();

    void on_buttonFilter_clicked();

    void on_selectFilterComboBox_currentIndexChanged(int index);

    void on_runFilterPushButton_clicked();

private:
    vtkwindow_new * m_OldRenderingWindow;
    void createModel();
    void resetInterface();
    void hideAllFilterParameter();

    //QList <VisPoint> m_VisPointsObjectList;
    VisPoint *m_VisPointsObject;
    VSTableDesktop * m_Table;
    QStandardItem *m_ParentItem;
    OperationQueue *queue;
    bool higal;
    QString class_l;
    QString class_b;
    QString class_r;
    QString class_dl;
    QString class_db;
    vtkwindow_new *myCallingVtkWindow;
    QString fileName;
    QString survey,species,transition;
    int type;
    QList < QPair<QString, QString> > selectedSurvey;


protected:
    void  closeEvent(QCloseEvent*);


};

#endif // MAINWINDOW_H
