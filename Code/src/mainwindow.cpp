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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QTreeView>
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <QIcon>
#include <QFont>
#include <QFileSystemModel>
#include "vtkwindow_new.h"
#include "visivoimporterdesktop.h"
#include "vispoint.h"
#include "singleton.h"
#include "vtkfitsreader.h"
#include "vtkSmartPointer.h"
#include "vialactea.h"
#include "sed.h"
//#include "vosamp.h"
//#include "visivofilterdesktop.h"
#include "customprocess.h"

/*
extern "C" {
#include "visivo.h"
}

*/
#include "vtkwindow_new.h"

/*
 *  type
 * 0 = ASCII
 * 1 = CSV
 * 2 = VOTABLE
 * 3 = BINARY
 *
 */


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{


    //m_RenderingWindow = NULL;
    // QWidget::setWindowIcon(QIcon( ":/icons/VisIVODesktop.icns" ));
    ui->setupUi(this);
    createModel();
    ui->treeView->setModel( m_VisIVOTreeModel );
    m_VisPointsObject = new VisPoint();
    QObject::connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(close()));
    QObject::connect(ui->actionImportAscii, SIGNAL(triggered()), this, SLOT(importAscii()));
    QObject::connect(ui->actionImportVoTable, SIGNAL(triggered()), this, SLOT(importVoTable()));
    QObject::connect(ui->actionImportBinary, SIGNAL(triggered()), this, SLOT(importBinary()));
    QObject::connect(ui->actionVTK_ImageData, SIGNAL(triggered()), this, SLOT(importVTI()));
    // QObject::connect(ui->actionVTK_PolyData, SIGNAL(TreeModeltriggered()), this, SLOT(importVTP()));
    QDir::home().mkdir("VisIVODesktopTemp");
    QDir tmp_download(QDir::homePath()+"/VisIVODesktopTemp/tmp_download");
    qDebug()<<QDir::homePath()+"/VisIVODesktopTemp/tmp_download";
    tmp_download.removeRecursively();
    QDir::home().mkdir("VisIVODesktopTemp/tmp_download");

    workingPath = QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp");
    importer= QString("/opt/VisIVOServer-vtk6/VisIVOImporter");
    filter = QString("/opt/VisIVOServer-vtk6/VisIVOFilters");
    util = QString("/opt/VisIVOServer/VisIVOUtils");


    queue = &Singleton<OperationQueue>::Instance();


    ui->importerGroupBox->hide();
    ui->filterGroupBox->hide();
    hideAllFilterParameter();

    // ui->addIdentifierParameterGroupBox->hide();
    // ui->appendParameterGroupBox->hide();

    ui->volumeGroupBox->hide();
    on_actionVialactea_triggered();

}

MainWindow::~MainWindow()
{
    delete ui;

}
void MainWindow::createModel()
{
    QStringList headerLabels;
    headerLabels << "Item";
    headerLabels << "Name";
    m_VisIVOTreeModel = new TreeModel(headerLabels,"");
    ui->tabWidget->setCurrentWidget(ui->tabObjectTree);
}
void MainWindow::resetInterface()
{
    ui->buttonUndo->setEnabled(false);
    ui->buttonResetCameraObject->setEnabled(false);
    ui->buttonPlot->setEnabled(false);
    ui->buttonMergeDO->setEnabled(false);
    ui->buttonMathOp->setEnabled(false);
    ui->buttonDefineVector->setEnabled(false);
    ui->buttonCreateGrid->setEnabled(false);
    ui->buttonCreateDO->setEnabled(false);
}

void MainWindow::importAscii()
{
    genericImport(0);
}

void MainWindow::genericImport(int t)
{
    int errorCode;

    fileName = QFileDialog::getOpenFileName(this,tr("Import a file"), "", tr("(*.*)"));
    if (fileName.compare("")!=0)
    {
        type=t;
        ui->tabWidget->setCurrentWidget(ui->tabOpParameters);
        ui->importerGroupBox->show();
        ui->filterGroupBox->hide();
        ui->addIdentifierParameterGroupBox->hide();
    }

}


void MainWindow::importAsciiFilaments(QString fileName, vtkwindow_new *v)
{
    // It Checks for an empty filename
    if (fileName.isEmpty()) return;
    VisIVOImporterDesktop *VI= new VisIVOImporterDesktop(fileName,v);

}

void MainWindow::importAsciiBubbles(QString fileName, vtkwindow_new *v)
{
    if (fileName.isEmpty()) return;
    VisIVOImporterDesktop *VI= new VisIVOImporterDesktop(fileName,v,false,true);

}

void MainWindow::importAscii3dSelection(QString fileName, vtkwindow_new *v)
{
    // It Checks for an empty filename
    if (fileName.isEmpty()) return;
    VisIVOImporterDesktop *VI= new VisIVOImporterDesktop(fileName,v,false);
}


void MainWindow::importAscii(QString fileName, QString wavelen, bool higal, bool bm, vtkwindow_new *v)
{
    // It Checks for an empty filename
    if (fileName.isEmpty()) return;
    VisIVOImporterDesktop *VI;
    if(higal)
    {
        VI = new VisIVOImporterDesktop(fileName,m_VisIVOTreeModel,bm,v,wavelen);
    }
    else
    {
        VI = new VisIVOImporterDesktop("ascii",fileName,m_VisIVOTreeModel);
        VI->setBm(bm);
        VI->setVtkWin(v);
        //        ui->tabWidget->setCurrentWidget(ui->tabViewSettings);
        //        ui->tabWidget->setCurrentWidget(ui->tabObjectTree);
        // ui->tabWidget->setCurrentWidget(ui->tabOpParameters);
        VI->doImport(wavelen,true);

    }
}


void MainWindow::importCSV(QString fileName)
{

    // It Checks for an empty filename
    if (fileName.isEmpty()) return;
    VisIVOImporterDesktop *VI;
    VI = new VisIVOImporterDesktop("csv",fileName,m_VisIVOTreeModel);
    VI->doImport("",true);


}

void MainWindow::importVOTable(QString fileName)
{

    // It Checks for an empty filename
    if (fileName.isEmpty()) return;
    VisIVOImporterDesktop *VI;
    VI = new VisIVOImporterDesktop("votable",fileName,m_VisIVOTreeModel);
    VI->doImport("",true);


}


void MainWindow::importVoTable() {

    genericImport(2);

    /*
    QString fileName = QFileDialog::getOpenFileName(this,tr("Import a file"), "", tr("(*.*)"));
    // It Checks for an empty filename
    if (fileName.isEmpty()) return;
    VisIVOImporterDesktop *VI = new VisIVOImporterDesktop("votable",fileName,m_VisIVOTreeModel);
    VI->doImport();
    ui->tabWidget->setCurrentWidget(ui->tabViewSettings);
    ui->tabWidget->setCurrentWidget(ui->tabObjectTree);
    //ui->treeView->update();
    */
}

void MainWindow::importBinary() {

    genericImport(3);


    /*Browse and choose the filename*/
    /*
    QString fileName = QFileDialog::getOpenFileName(this,tr("Import a file"), "", tr("(*.*)"));
    // It Checks for an empty filename
    if (fileName.isEmpty()) return;


    VisIVOImporterDesktop *VI = new VisIVOImporterDesktop("binary",fileName,m_VisIVOTreeModel);
    VI->doImport();

    ui->tabWidget->setCurrentWidget(ui->tabViewSettings);
    ui->tabWidget->setCurrentWidget(ui->tabObjectTree);
    //ui->treeView->update();
    */
}

void MainWindow::importBinaryTable(QString fileName)
{

    // It Checks for an empty filename
    if (fileName.isEmpty()) return;
    VisIVOImporterDesktop *VI;
    VI = new VisIVOImporterDesktop("binary",fileName,m_VisIVOTreeModel);
    VI->doImport("",true);


}

void MainWindow::importVTI() {
    /*Browse and choose the filename*/
    QString fileName = QFileDialog::getOpenFileName(this,tr("Import a file"), "", tr("(*.*)"));
    // It Checks for an empty filename
    if (fileName.isEmpty()) return;
    VisIVOImporterDesktop *VI = new VisIVOImporterDesktop("VTI",fileName,m_VisIVOTreeModel);
    VI->doImport();

    ui->tabWidget->setCurrentWidget(ui->tabViewSettings);
    ui->tabWidget->setCurrentWidget(ui->tabObjectTree);
    //ui->treeView->update();
    // Check if the importing is OK
    if (VI->getStatus()) return;
    return;
}

void MainWindow::importVTP() {
    /*Browse and choose the filename*/
    QString fileName = QFileDialog::getOpenFileName(this,tr("Import a file"), "", tr("(*.*)"));
    // It Checks for an empty filename
    if (fileName.isEmpty()) return;
    VisIVOImporterDesktop *VI = new VisIVOImporterDesktop("VTP",fileName,m_VisIVOTreeModel);
    VI->doImport();

    ui->tabWidget->setCurrentWidget(ui->tabViewSettings);
    ui->tabWidget->setCurrentWidget(ui->tabObjectTree);
    //ui->treeView->update();
    // Check if the importing is OK
    if (VI->getStatus()) return;
    return;
}

void MainWindow::on_actionDatacube_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Import a file"), "", tr("(*.*)"));
    // It Checks for an empty filename
    if (fileName.isEmpty()) return;
    importFitsImage(fileName);

    //IMPORT DATACUBE
}


void MainWindow::on_actionImportFitsImage_triggered()
{

    QString fileName = QFileDialog::getOpenFileName(this,tr("Import a file"), "", tr("(*.*)"));
    // It Checks for an empty filename
    if (fileName.isEmpty()) return;

    importFitsImage(fileName);
}

void MainWindow::importFitsImage(QString filename, QList<QMap<QString, QString> > elementsOnDb, QString l,QString b,QString r, QString db,QString dl, bool layer )
{

    VisIVOImporterDesktop *VI = new VisIVOImporterDesktop("FITSIMG",filename,m_VisIVOTreeModel);
    VI->doImport();

    class_l=l;
    class_b=b;
    class_r=r;
    class_db=db;
    class_dl=dl;

    int rows = m_VisIVOTreeModel->rowCount()-1;

    QModelIndex first = m_VisIVOTreeModel->index(rows, 0, QModelIndex());
    ui->treeView->setCurrentIndex(first);
    itemSelected(elementsOnDb,layer);

}

void MainWindow::switchTab(int index)
{
    switch (index){
    case 0:break;  // tabObjectTree
    case 1:// tabViewSettings
    {
        MainWindow::viewSetting();
    }
    case 2:break;  // tabOpParameters
    default:
        return;
    }
}

TreeModel* MainWindow::getTreeModel()
{
    return m_VisIVOTreeModel;
}

void MainWindow::viewSetting() {

    qDebug()<<"***pre.setCurrentWidget.viewSetting().MainWindow";
    ui->tabWidget->setCurrentWidget(ui->tabViewSettings);
    qDebug()<<"***post.setCurrentWidget.viewSetting().MainWindow";

    // Retrieve the first index among the selected indexes
    QModelIndexList list = ui->treeView->selectionModel()->selectedIndexes();

    // Don't use an empty list (with no selected items)
    // Hide the tabViewSettings widget
    if (list.isEmpty()) {
        //qDebug() << "MainWindow::viewSetting: No selected items";
        ui->tabViewSettings->hide();
        return;
    }

    QModelIndex  selectedItemIndex  = list[0];

    TreeItem::TreeItemType type = m_VisIVOTreeModel->getType(selectedItemIndex);

    // a TreeModel Object can return a table both for PointTable and VisualObject objects
    m_Table= m_VisIVOTreeModel->getTable(selectedItemIndex);

    // Don't use an invalid table
    if (!m_Table->checkTable())
    {
        qDebug() << "MainWindow::viewSetting: Error in importing the table";
        ui->tabViewSettings->hide();
        return;
    }

    ui->tabViewSettings->show();
    unsigned int numberOfFields;
    numberOfFields = m_Table->getNumberOfColumns();

    QString name = QString(m_Table->getName().c_str());
    QStringList fields;

    ui->XcomboBox->clear();
    ui->YcomboBox->clear();
    ui->ZcomboBox->clear();
    ui->XVectorsComboBox->clear();
    ui->YVectorsComboBox->clear();
    ui->ZVectorsComboBox->clear();

    switch(type){
    case TreeItem::Null: break;
    case TreeItem::VTI: break;
    case TreeItem::VTP: break;
    case TreeItem::VisualObject: {
        m_VisPointsObject = m_VisIVOTreeModel->getVisualObject(selectedItemIndex);
        ui->XLogCheckBox->setChecked(m_VisPointsObject->isEnabledLogX());
        ui->YLogCheckBox->setChecked(m_VisPointsObject->isEnabledLogY());
        ui->ZLogCheckBox->setChecked(m_VisPointsObject->isEnabledLogZ());

        ui->ShowPointsCheckBox->setChecked(m_VisPointsObject->isEnabledShowPoints());
        ui->ScaleCheckBox->setChecked(m_VisPointsObject->isEnabledScale());
        ui->ShowAxesCheckBox->setChecked(m_VisPointsObject->isEnabledShowAxes());
        ui->ShowBoxCheckBox->setChecked(m_VisPointsObject->isEnabledShowBox());
        ui->ShowVectorsCheckBox->setChecked(m_VisPointsObject->isEnabledShowVectors());

        ui->namelineEdit->setText(m_VisPointsObject->getName());
        for (unsigned int i =0; i < numberOfFields;i++)
        {
            QString field =  QString(m_Table->getColName(i).c_str());
            fields << field;
            ui->XcomboBox->addItem(field);
            ui->YcomboBox->addItem(field);
            ui->ZcomboBox->addItem(field);

            ui->XVectorsComboBox->addItem(field);
            ui->YVectorsComboBox->addItem(field);
            ui->ZVectorsComboBox->addItem(field);
        }


        ui->XcomboBox->setCurrentIndex(fields.indexOf(m_VisPointsObject->getX()));
        ui->YcomboBox->setCurrentIndex(fields.indexOf(m_VisPointsObject->getY()));
        ui->ZcomboBox->setCurrentIndex(fields.indexOf(m_VisPointsObject->getZ()));

        ui->XVectorsComboBox->setCurrentIndex(fields.indexOf(m_VisPointsObject->getVectorX()));
        ui->YVectorsComboBox->setCurrentIndex(fields.indexOf(m_VisPointsObject->getVectorY()));
        ui->ZVectorsComboBox->setCurrentIndex(fields.indexOf(m_VisPointsObject->getVectorZ()));
        // hide some controls...
        ui->XLogCheckBox->hide();
        ui->YLogCheckBox->hide();
        ui->ZLogCheckBox->hide();

        ui->XVectorsComboBox->hide();
        ui->YVectorsComboBox->hide();
        ui->ZVectorsComboBox->hide();

        ui->XVectorsLabel->hide();
        ui->YVectorsLabel->hide();
        ui->ZVectorsLabel->hide();

        ui->ShowPointsCheckBox->hide();

        ui->ScaleCheckBox->hide();
        ui->ShowAxesCheckBox->hide();
        ui->ShowBoxCheckBox->hide();

        ui->ShowVectorsCheckBox->hide();
        // end hide some controls...
        // end control setting case TreeItem::VisualObject

    }
    case TreeItem::VolumeTable: break;
    case TreeItem::PointTable:
    {
        m_VisPointsObject = new VisPoint(m_Table);

        // hide some controls...
        ui->XLogCheckBox->hide();
        ui->YLogCheckBox->hide();
        ui->ZLogCheckBox->hide();

        ui->XVectorsComboBox->hide();
        ui->YVectorsComboBox->hide();
        ui->ZVectorsComboBox->hide();

        ui->XVectorsLabel->hide();
        ui->YVectorsLabel->hide();
        ui->ZVectorsLabel->hide();

        ui->ShowPointsCheckBox->hide();

        ui->ScaleCheckBox->hide();
        ui->ShowAxesCheckBox->hide();
        ui->ShowBoxCheckBox->hide();

        ui->ShowVectorsCheckBox->hide();
        // end hide some controls...

        // control setting case TreeItem::PointTable
        ui->namelineEdit->setText(m_VisPointsObject->getName());

        for (unsigned int i =0; i < numberOfFields;i++)
        {
            QString field =  QString(m_Table->getColName(i).c_str());
            fields << field;
            ui->XcomboBox->addItem(field);
            ui->YcomboBox->addItem(field);
            ui->ZcomboBox->addItem(field);

            ui->XVectorsComboBox->addItem(field);
            ui->YVectorsComboBox->addItem(field);
            ui->ZVectorsComboBox->addItem(field);
        }
        // Set some default values for the following ComboBox:
        ui->XcomboBox->setCurrentIndex(0);
        ui->YcomboBox->setCurrentIndex(1);
        ui->ZcomboBox->setCurrentIndex(2);

        ui->XVectorsComboBox->setCurrentIndex(0);
        ui->YVectorsComboBox->setCurrentIndex(1);
        ui->ZVectorsComboBox->setCurrentIndex(2);

        ui->XLogCheckBox->setChecked(m_VisPointsObject->isEnabledLogX());
        ui->YLogCheckBox->setChecked(m_VisPointsObject->isEnabledLogY());
        ui->ZLogCheckBox->setChecked(m_VisPointsObject->isEnabledLogZ());

        ui->ShowPointsCheckBox->setChecked(m_VisPointsObject->isEnabledShowPoints());
        ui->ScaleCheckBox->setChecked(m_VisPointsObject->isEnabledScale());
        ui->ShowAxesCheckBox->setChecked(m_VisPointsObject->isEnabledShowAxes());
        ui->ShowBoxCheckBox->setChecked(m_VisPointsObject->isEnabledShowBox());
        ui->ShowVectorsCheckBox->setChecked(m_VisPointsObject->isEnabledShowVectors());
        // end control setting case TreeItem::PointTable
        return;
    }
    default:
        return;
    }

}

void MainWindow::viewSettingOk()
{
    // Retrieve the first index among the selected indexes
    QModelIndexList list = ui->treeView->selectionModel()->selectedIndexes();

    // Don't use an empty list (from an empty model)
    if (list.isEmpty()) return;

    QModelIndex  selectedItemIndex  = list[0];

    TreeItem::TreeItemType type = m_VisIVOTreeModel->getType(selectedItemIndex);

    // a TreeModel Object can return a table both for PointTable and VisualObject objects
    m_Table= m_VisIVOTreeModel->getTable(selectedItemIndex);

    // Don't use an invalid table
    if (!m_Table->checkTable())
    {
        qDebug() << "MainWindow::viewSettingOk: Error in importing the table";
        return;
    }


    unsigned int numberOfFields;
    numberOfFields = m_Table->getNumberOfColumns();
    if (numberOfFields < 3 )
    {
        qDebug() << "numberOfFields < 3";
        return;
    }
    QString name = QString(m_Table->getName().c_str());
    QStringList fields;

    // A brand new m_VisPointsObject is needed if type==PoinTable
    if (type==TreeItem::PointTable)
    {
        m_VisPointsObject = new VisPoint(m_Table);
    }
    if (type==TreeItem::PointTable || type==TreeItem::VisualObject)
    {


        m_VisPointsObject->setLogX(ui->XLogCheckBox->isChecked());
        m_VisPointsObject->setLogY(ui->YLogCheckBox->isChecked());
        m_VisPointsObject->setLogZ(ui->ZLogCheckBox->isChecked());

        m_VisPointsObject->setShowPoints(ui->ShowPointsCheckBox->isChecked());
        m_VisPointsObject->setScale(ui->ScaleCheckBox->isChecked());
        m_VisPointsObject->setShowAxes(ui->ShowAxesCheckBox->isChecked());
        m_VisPointsObject->setShowBox(ui->ShowBoxCheckBox->isChecked());
        m_VisPointsObject->setShowVectors(ui->ShowVectorsCheckBox->isChecked());

        m_VisPointsObject->setX(ui->XcomboBox->currentText());
        m_VisPointsObject->setY(ui->YcomboBox->currentText());
        m_VisPointsObject->setZ(ui->ZcomboBox->currentText());

        m_VisPointsObject->setVectorX(ui->XVectorsComboBox->currentText());
        m_VisPointsObject->setVectorY(ui->YVectorsComboBox->currentText());
        m_VisPointsObject->setVectorZ(ui->ZVectorsComboBox->currentText());

        // bisogna espandere prima un elemento perchè lo slot expand(QModelIndex) fallisce
        // mentre lo slot expandall() cancella tutta la vista dell'albero
        // indagare please....
        ui->treeView->expand(selectedItemIndex);
        QModelIndex  parentIndex  = selectedItemIndex;

        // If the selected item is a VisualObject you will create another Visual Object with the same parent
        if (type==TreeItem::VisualObject) parentIndex = m_VisIVOTreeModel->parent(parentIndex);

        int rows = m_VisIVOTreeModel->rowCount(parentIndex);
        if (m_VisIVOTreeModel->insertRows(rows,1,parentIndex))
        {
            m_VisIVOTreeModel->setVisualObject(m_VisIVOTreeModel->index(rows,0,parentIndex),m_VisPointsObject);
            m_VisIVOTreeModel->setData(m_VisIVOTreeModel->index(rows,0,parentIndex),QIcon( ":/icons/VBT_CLOUD.bmp" ));
            m_VisIVOTreeModel->setData(m_VisIVOTreeModel->index(rows,1,parentIndex),"VisualObject");
            // Set tabObjectTree as Current tab
            //ui->tabWidget->setCurrentWidget(ui->tabObjectTree);
        }
        if (m_VisPointsObject->isOriginSpecified())
        {
            ui->tabWidget->setCurrentWidget(ui->tabObjectTree);
            m_OldRenderingWindow = new vtkwindow_new(this, m_VisPointsObject);
            /*
            // We will open a single Rendering Window
            if (m_RenderingWindow == NULL)
            {
                m_OldRenderingWindow = new vtkwindow(this, m_VisPointsObject);
            }
            else
            {
                m_RenderingWindow->update(m_VisIVOTreeModel);
            }
            */
        }
    }
    /*
QModelIndexList lista = m_VisIVOTreeModel->getActiveViewList();
int numberOfVisualObject = lista.count();
int b=numberOfVisualObject;
*/
}

void MainWindow::itemSelected(QList<QMap<QString, QString> > elementsOnDb,bool layer)
{
    qDebug()<<"itemSelected";
    // Retrieve the first index among the selected indexes
    QModelIndexList list = ui->treeView->selectionModel()->selectedIndexes();

    // Don't use an empty list (with no selected items)
    // Hide the tabViewSettings widget
    if (list.isEmpty()) {
        //qDebug() << "MainWindow::viewSetting: No selected items";
        ui->tabViewSettings->hide();
        return;
    }

    QModelIndex  selectedItemIndex  = list[0];
    TreeItem::TreeItemType type = m_VisIVOTreeModel->getType(selectedItemIndex);

    if(type == TreeItem::FITSIMAGE )
    {
        qDebug()<<"mainwindow::itemSelected layer: "<<layer;
        if(layer)
        {
            qDebug()<<"*******++ "<<survey<<" "<<species<<" "<<transition;

            myCallingVtkWindow->addLayerImage(m_VisIVOTreeModel->getFITSIMG(selectedItemIndex),survey,species,transition);
        }
        else
        {
            vtkSmartPointer<vtkFitsReader> fitsreader= m_VisIVOTreeModel->getFITSIMG(selectedItemIndex);
            fitsreader->setSurvey(survey);
            fitsreader->setSpecies(species);
            fitsreader->setTransition(transition);

            qDebug()<<"*******++ "<<survey<<" "<<species<<" "<<transition;

            m_OldRenderingWindow = new vtkwindow_new(this, m_VisIVOTreeModel->getFITSIMG(selectedItemIndex));

            if (elementsOnDb.size()>0)
            {
                m_OldRenderingWindow->setDbElements(elementsOnDb);
                m_OldRenderingWindow->setCallingL(class_l);
                m_OldRenderingWindow->setCallingB(class_b);
                m_OldRenderingWindow->setCallingR(class_r);
                m_OldRenderingWindow->setCallingDl(class_dl);
                m_OldRenderingWindow->setCallingDb(class_db);
            }
            if (selectedSurvey.size()>1)
            {

                m_OldRenderingWindow->setCallingL(class_l);
                m_OldRenderingWindow->setCallingB(class_b);
                m_OldRenderingWindow->setCallingR(class_r);
                m_OldRenderingWindow->setCallingDl(class_dl);
                m_OldRenderingWindow->setCallingDb(class_db);
                m_OldRenderingWindow->downloadStartingLayers(selectedSurvey);
            }

        }

    }
    //IF IS NOT FITS IMAGE
    else
    {
        m_Table= m_VisIVOTreeModel->getTable(selectedItemIndex);


        // Don't use an invalid table
        if (!m_Table->checkTable())
        {
            ui->tabViewSettings->hide();
            return;
        }

        unsigned int numberOfElements;
        unsigned int numberOfFields;
        numberOfFields = m_Table->getNumberOfColumns();
        numberOfElements = m_Table->getNumberOfRows();

        QString name = QString(m_Table->getName().c_str());

        QStringList fields;
        ui->fieldTable->clear();
        ui->fieldTable->setRowCount(0);
        ui->fieldTable->setColumnCount(0);

        ui->elementLabel->clear();
        ui->fieldLabel->clear();
        ui->nameLabel->clear();
        ui->typeLabel->clear();

        ui->valueNameLabel->clear();
        ui->valueTypeLabel->clear();
        ui->valueFieldLabel->clear();
        ui->valueElementLabel->clear();

        ui->buttonUndo->setEnabled(false);
        ui->buttonResetCameraAll->setEnabled(false);
        ui->buttonResetCameraObject->setEnabled(false);
        ui->buttonCreateDO->setEnabled(false);
        ui->buttonFilter->setEnabled(false);
        ui->buttonCreateGrid->setEnabled(false);
        ui->buttonDefineVector->setEnabled(false);
        ui->buttonMathOp->setEnabled(false);
        ui->buttonPlot->setEnabled(false);
        ui->buttonMergeDO->setEnabled(false);


        switch(type){
        case TreeItem::Null: break;
        case TreeItem::VTI: break;
        case TreeItem::VTP: break;
        case TreeItem::VisualObject:
        {

            ui->typeLabel->setText("Type");
            ui->nameLabel_2->setText("Name");

            ui->valueNameLabel->setText(name);
            ui->valueTypeLabel->setText("Visual Object");
        }
        case TreeItem::VolumeTable: break;
        case TreeItem::PointTable:
        {

            ui->buttonCreateDO->setEnabled(true);
            ui->buttonFilter->setEnabled(true);

            ui->fieldTable->setColumnCount(3);
            QStringList headers;
            headers << "Field Name" << "Min" << "Max";

            ui->fieldTable->setHorizontalHeaderLabels(headers);

            for (unsigned int i =0; i < numberOfFields;i++)
            {

                QString field =  QString(m_Table->getColName(i).c_str());
                fields << field;

                ui->fieldTable->insertRow(i);

                QTableWidgetItem *nameItem= new QTableWidgetItem(field);
                nameItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                ui->fieldTable->setItem(i,0, nameItem);

                float *range = new float[2];
                m_Table->getRange(i,range);
                QTableWidgetItem *minItem= new QTableWidgetItem(QString::number(range[0]));
                minItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

                ui->fieldTable->setItem(i,1, minItem);

                QTableWidgetItem *maxItem= new QTableWidgetItem(QString::number(range[1]));
                maxItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                ui->fieldTable->setItem(i,2, maxItem);
            }


            ui->typeLabel->setText("Type");
            ui->nameLabel_2->setText("Name");
            ui->elementLabel->setText("Elements");
            ui->fieldLabel->setText("Fields");

            ui->valueNameLabel->setText(name);
            ui->valueTypeLabel->setText("Point Table");
            ui->valueFieldLabel->setText(QString::number(numberOfFields));
            ui->valueElementLabel->setText(QString::number(numberOfElements));

        }
        default:return;
        }

    }


}

void MainWindow::on_actionOperation_queue_triggered()
{
    queue->show();
}

void MainWindow::on_actionHi_Gal_triggered()
{

    //HiGal *higalWin = &Singleton<HiGal>::Instance();
    //higalWin->show();

}

void MainWindow::on_actionVialactea_triggered()
{
    ViaLactea *vialactealWin = &Singleton<ViaLactea>::Instance();
 //   vialactealWin->showMaximized();

    vialactealWin->show();

}

void MainWindow::on_actionCsv_triggered()
{

    genericImport(1);


    /*Browse and choose the filename*/
    /*
    QString fileName = QFileDialog::getOpenFileName(this,tr("Import a file"), "", tr("(*.*)"));
    // It Checks for an empty filename
    if (fileName.isEmpty()) return;
    VisIVOImporterDesktop *VI = new VisIVOImporterDesktop("csv",fileName,m_VisIVOTreeModel);
    VI->doImport();
    ui->tabWidget->setCurrentWidget(ui->tabViewSettings);
    ui->tabWidget->setCurrentWidget(ui->tabObjectTree);
    //ui->treeView->update();
    */
}

void MainWindow::on_actionTEST_DC3D_triggered()
{
    
    QString fileName = QFileDialog::getOpenFileName(this,tr("Import a file"), "", tr("FITS images(*.fit *.fits)"));
    
    if (!fileName.isEmpty() )
    {

        vtkSmartPointer<vtkFitsReader> fitsReader = vtkSmartPointer<vtkFitsReader>::New();
        fitsReader->is3D=true;
        fitsReader->SetFileName(fileName.toStdString());
        fitsReader->GetOutput();

        //TEST
        //new vtkwindow(this,fitsReader,1);
        vtkwindow_new *nn=new vtkwindow_new(this,fitsReader,1);
        //  nn->showNormal();
        //END TEST

    }
    
}

void MainWindow::importFitsDC(QString fileName)
{
    vtkSmartPointer<vtkFitsReader> fitsReader = vtkSmartPointer<vtkFitsReader>::New();
    fitsReader->is3D=true;
    fitsReader->SetFileName(fileName.toStdString());
    // fitsReader->GetOutput();

    //TEST
    //new vtkwindow(this,fitsReader,1);
    new vtkwindow_new(this,fitsReader,1,myCallingVtkWindow);
}

void   MainWindow::closeEvent(QCloseEvent*)
{
    qDebug()<<"close";

    // vosamp *samp = &Singleton<vosamp>::Instance();
    // samp->unregister();

    qApp->quit();
}

void MainWindow::on_volumeRadioButton_toggled(bool checked)
{
    if (checked)
    {
        ui->volumeGroupBox->show();
    }
    else
        ui->volumeGroupBox->hide();

}

void MainWindow::on_importPushButton_clicked()
{
    //Removed VisIVO Integrations
    /*
    qDebug()<<"filename: "<<fileName;
    qDebug()<<"type: "<<type;
    int errorCode;

    switch (type) {
    case 0:
        importAscii(fileName);
        break;
    case 1:
        importCSV(fileName);
        break;
    case 2:
        importVOTable(fileName);
        break;
    case 3:
        importBinaryTable(fileName);
        break;
    default:
        break;
    }


    VisIVOImporter envVI1;
    errorCode=VI_SetAtt(&envVI1,VI_SET_FFORMAT,"ascii");
    errorCode=VI_SetAtt(&envVI1,VI_SET_FILEPATH, fileName.toLocal8Bit().data());

    QFileInfo infoFile = QFileInfo(fileName);


    QString outputPathString=QDir::homePath()+"/VisIVODesktopTemp/tmp_download/"+infoFile.fileName();
    char *outputPath= outputPathString.toLocal8Bit().data();
    errorCode=VI_SetAtt(&envVI1,VI_SET_OUTFILEVBT,outputPath);

    VI_Import(&envVI1);
*/

}

void MainWindow::on_buttonFilter_clicked()
{

    ui->importerGroupBox->hide();
    ui->filterGroupBox->show();
    ui->addIdentifierParameterGroupBox->show();

    ui->tabWidget->setCurrentWidget(ui->tabOpParameters);

    QModelIndex  selectedItemIndex  = ui->treeView->selectionModel()->selectedIndexes()[0];


    selectedFile=QString::fromStdString( m_VisIVOTreeModel->getTable(selectedItemIndex)->getLocator() );

}

void MainWindow::on_selectFilterComboBox_currentIndexChanged(int index)
{
    hideAllFilterParameter();
    switch (index) {
    //add identifier
    case 0:
        ui->filterDescriptionTextBrowser->setText("This filter adds a new column to the input table. The column name is given in Column Name field, with a sequence of value starting from a Start Number value (Default value is 0).");
        ui->addIdentifierParameterGroupBox->show();
        break;
        //Append
    case 1:
        ui->filterDescriptionTextBrowser->setText("This filter creates a new table appending data from a list of selected tables. The selected Tables must contain the same numbers of columns.");

        // add vbt on left
        for(int i=0; i<m_VisIVOTreeModel->rowCount();i++)
        {
            QModelIndex itemIndex  = ui->treeView->model()->index(i,0);
            TreeItem::TreeItemType type = m_VisIVOTreeModel->getType(itemIndex);
            int colNum=m_VisIVOTreeModel->getTable(itemIndex)->getNumberOfColumns();
            qDebug()<<"type: "<<type <<" colNum: "<<colNum<<" oriColNum: "<< m_VisIVOTreeModel->getTable(ui->treeView->selectionModel()->selectedIndexes()[0])->getNumberOfColumns();
            if(type == TreeItem::PointTable && colNum== m_VisIVOTreeModel->getTable(ui->treeView->selectionModel()->selectedIndexes()[0])->getNumberOfColumns() )
            {
                qDebug()<<"stica :"<<i;
            }

        }

        ui->selectedVBT->setText(selectedFile);
        ui->appendParameterGroupBox->show();
        break;
        //Cartesian 2 Polar
    case 2:
        ui->filterDescriptionTextBrowser->setText("This filter creates creates three new columns with names given in Column name for fields as the result of the spherical polar transformation of three existing columns given in Columns for point coordinates fields. A new table is created if the New Table option is selected.");
        break;
        //cut
    case 3:
        ui->filterDescriptionTextBrowser->setText("This filter fixes to a given threshold all the column values included in an interval. Limits of the interval on all selected fields can combined with logic AND/OR operator.The 'unlimited' word can be used to indicate the infinite value.");
        break;
        //Decimator
    case 4:
        ui->filterDescriptionTextBrowser->setText("This filter creates a subtable as a regular subsample from the input table. Values are extracted in a regular sequence, skipping step element every time. The skip value is an integer number > 1 and represents the number of skipped values.");
        break;
        //Extract List
    case 5:
        ui->filterDescriptionTextBrowser->setText("This filter creates a new table from an input table with the elements (rows) listed in a given multi-list.");
        break;
        //Extraction
    case 6:
        ui->filterDescriptionTextBrowser->setText("This filter creates a new table from a sub-box or from a sphere in a 3D space. Three fields must be selected and the value must be given as follow: Sphere: sphere center coordinates Box: rectangular region center coordinates Corner: lower corner region coordinates. The Size field represent the sub volume dimension.");
        break;
        //Grid 2 Point
    case 7:
        ui->filterDescriptionTextBrowser->setText("This operation distributes a volume property to a point data set on the same computational domain using a field distribution (CIC/NGP/TSC algorithm) on a regular mesh. CIC is the default adopted algorithm. The Cell geometry is considered only to compute the cell volume value in this operation. This filter produces a new table or adds a new field to the input table.");
        break;
        //Include
    case 8:
        ui->filterDescriptionTextBrowser->setText("This operation assigns a different offset value to points inside and/or outside a sphere in the domain. This filter produces a new table or adds a new field to the input table. Points inside the sphere (given with center and radius) will have the value invalue, otherwise outvalue.");
        break;
        //Math Operation
    case 9:
        ui->filterDescriptionTextBrowser->setText("This filter creates a new field in a new table or add a new field as the result of a mathematical operation between the existing fields.");
        break;
        //Merge
    case 10:
        ui->filterDescriptionTextBrowser->setText("This filter creates a new table from two or more existing data tables. The user must select tables to be merged The filter produces a new table having the size of the smallest table or a new table having the size of the greatest table. In this case the PAD field value is used to pad the table rows of the smallest table.");
        break;
        //Module
    case 11:
        ui->filterDescriptionTextBrowser->setText("This operation computes the module of three fields (x, y, z) of the input data table sqrt(x^2+y^2+z^2).");
        break;
        //Multi Resolution
    case 12:
        ui->filterDescriptionTextBrowser->setText("This operation creates a new VBT as a random subsample from the input table, with different resolutions. Starting from a fixed position, that represent the center of inner sphere, concentric spheres are considered. Different level of randomization can be given, creating more detail table in the inner sphere and lower detail in the outer regions, or vice versa. The region that is external to the last sphere represents the background.");
        break;
        //Point distribute
    case 13:
        ui->filterDescriptionTextBrowser->setText("This filter creates a volume using a field distribution, CIC (default) or NGP or TSC algorithm, on a regular mesh. The filter produces (by default) a density field: the field is distributed and divided for the cell volume.");
        break;
        //Point Property
    case 14:
        ui->filterDescriptionTextBrowser->setText("This operation assigns a property to each data point on the table. The operation performs the following: 1) It creates a temporary volume using a field distribution (CIC algorithm) on a regular mesh. 2) It computes, with the same CIC algorithm, the property for each data point, considering the cells where the point is spread on the volume; 3) It saves the property in a new table or adds the field to the original input table.");
        break;
        //Randomizer
    case 15:
        ui->filterDescriptionTextBrowser->setText("This filter creates a new table as a random subsample from the input table. Percentage (from 0.0 to 95.0) of the input file obtained in the output file. Note: only the first decimal place is considered.");
        break;
        //Scalar distribution
    case 16:
        ui->filterDescriptionTextBrowser->setText("This filter computes average, min and max value of fields and creates an histogram of selected fields.");
        break;
        //Select Columns
    case 17:
        ui->filterDescriptionTextBrowser->setText("This filter creates e new table using one or more fields of a data table. 'Extract' produces the output table including only the selected fields, 'Remove' produces the output table excluding the selected fields.");
        break;
        //Select Rows
    case 18:
        ui->filterDescriptionTextBrowser->setText("This filter creates a new table setting limits on one or more fields of a the table. Limits on all selected can combined with logic AND/OR operator.The 'unlimited' word can be used to indicate the infinite value.");
        break;
        //Sigma Contours
    case 19:
        ui->filterDescriptionTextBrowser->setText("This filter creates a new table where one or more fields of a data table have values within (or outside) N sigma contours. The filter can be applied on fields that have a Gaussian distribution.");
        break;
        //Split tables
    case 20:
        ui->filterDescriptionTextBrowser->setText("This filter divides an existing table into two or more tables, using a field that will be used to divide the table. In case of Volume the split direction [1-3] must be specified.");
        break;
        //Write VO Table
    case 21:
        ui->filterDescriptionTextBrowser->setText("This filter produces a VOTable 1.2 with the selected fields.");
        break;
    default:
        break;
    }
}


void MainWindow::hideAllFilterParameter()
{

    ui->addIdentifierParameterGroupBox->hide();
    ui->appendParameterGroupBox->hide();

}

void MainWindow::on_runFilterPushButton_clicked()
{
    //VisIVOFilterDesktop::runFilter(ui->selectFilterComboBox->currentIndex());
}
