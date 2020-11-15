#include "higalselectedsources.h"
#include "ui_higalselectedsources.h"
#include "qdebug.h"
#include <QListWidget>
//#include "ViaLactea.h"
#include "singleton.h"
#include "plotwindow.h"
//#include "viewselectedsourcedataset.h"
#include "selectedsourcefieldsselect.h"
#include <QWidgetAction>
#include "vlkbquery.h"
#include "astroutils.h"
#include "vtkCleanPolyData.h"
#include "vtkPolyData.h"
HigalSelectedSources::HigalSelectedSources(vtkwindow_new *v, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HigalSelectedSources)
{
    ui->setupUi(this);

    vtkwin=v;

    plotMenu = new QMenu("Plot", this);


    QAction*   newWindowAction = new QAction("New plot window", this);
    plotMenu->addAction(newWindowAction);
    connect(newWindowAction, SIGNAL(triggered()), this, SLOT(plotNewWindow()));

    existingWindowMenu = new QMenu("Existing plot", this);
    //existingWindowMenu->setEnabled(false);
    plotMenu->addMenu(existingWindowMenu);

    ui->plotButton->setMenu(plotMenu);

    itemSelected=false;
    itemChanged=true;

    qDebug()<<"HigalSelectedSources costruttore";

}

void HigalSelectedSources::setConnect(QListWidget * list)
{

    connect( list, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*) ), this, SLOT( sourceChangedEvent(QListWidgetItem*,QListWidgetItem*) ) );
    connect( list, SIGNAL( itemSelectionChanged()) , this, SLOT( itemSelectionChanged() ) );
    connect( list, SIGNAL( itemPressed(QListWidgetItem*)) , this, SLOT( itemPressed(QListWidgetItem*) ) );

}

void HigalSelectedSources::itemPressed(QListWidgetItem *cur)
{

    if( itemChanged)
    {

        qDebug()<<ui->tabWidget->tabText(ui->tabWidget->currentIndex());

        VSTableDesktop *table=vtkwin->getEllipseList().value(cur->text())->getTable();
        std::cout<<table->getName()<<std::endl;
        qDebug()<<"cur: "<<cur->text();
        int row = vtkwin->getEllipseList().value(cur->text())->getRowInTable();
        qDebug()<<"row: "<<row;


        double semiMajorAxisLength,semiMinorAxisLength,angle,ra,dec;

        int id=table->getColId("fwhma");
        if(id == -1)
            id=table->getColId("fwhma"+ui->tabWidget->tabText(ui->tabWidget->currentIndex()).toStdString());
        if(id == -1)
            qDebug()<<"non esiste";
        else
            semiMajorAxisLength= ::atof(table->getTableData()[id][row].c_str());

        //double semiMajorAxisLength= ::atof(table->getTableData()[table->getColId("fwhma")][row].c_str());

        id=table->getColId("fwhmb");
        if(id == -1)
            id=table->getColId("fwhmb"+ui->tabWidget->tabText(ui->tabWidget->currentIndex()).toStdString());
        if(id == -1)
            qDebug()<<"non esiste";
        else
            semiMinorAxisLength= ::atof(table->getTableData()[id][row].c_str());

        id=table->getColId("pa");
        if(id == -1)
            id=table->getColId("pa"+ui->tabWidget->tabText(ui->tabWidget->currentIndex()).toStdString());
        if(id == -1)
            qDebug()<<"non esiste";
        else
            angle= ::atof(table->getTableData()[id][row].c_str());

        //double angle= ::atof(table->getTableData()[table->getColId("pa")][row].c_str());

        id=table->getColId("glon");
        if(id == -1)
            id=table->getColId("glon"+ui->tabWidget->tabText(ui->tabWidget->currentIndex()).toStdString());
        if(id == -1)
            qDebug()<<"non esiste";
        else
            ra= ::atof(table->getTableData()[id][row].c_str());

        // double ra= ::atof(table->getTableData()[table->getColId("glon")][row].c_str());

        id=table->getColId("glat");
        if(id == -1)
            id=table->getColId("glat"+ui->tabWidget->tabText(ui->tabWidget->currentIndex()).toStdString());
        if(id == -1)
            qDebug()<<"non esiste";
        else
            dec= ::atof(table->getTableData()[id][row].c_str());


        //  double dec= ::atof(table->getTableData()[table->getColId("glat")][row].c_str());

        double *coord= new double[3];

        AstroUtils().sky2xy(vtkwin->filenameWithPath,ra,dec,coord);


        vtkEllipse *el= new vtkEllipse(semiMajorAxisLength/2,semiMinorAxisLength/2,angle, coord[0], coord[1], coord[2], 0,0, cur->text(), NULL);
        drawSingleEllipse(el);
        itemChanged=false;
    }


}

void HigalSelectedSources::itemSelectionChanged()
{

    itemSelected=true;
    if(ellipseActor)
    {
        vtkwin->removeSingleEllipse(ellipseActor);
    }


}

void HigalSelectedSources::sourceChangedEvent(QListWidgetItem* cur,QListWidgetItem* pre)
{
    if (pre)
        itemChanged= true;

    /*
    if(itemSelected)
    {
        qDebug()<<"\t*";

        if(ellipseActor)
        {
            vtkwin->removeSingleEllipse(ellipseActor);
        }

        VSTable *table=vtkwin->getEllipseList().value(cur->text())->getTable();
        int row = vtkwin->getEllipseList().value(cur->text())->getRowInTable();
        double semiMajorAxisLength= ::atof(table->getTableData()[table->getColId("fwhma")][row].c_str());
        double semiMinorAxisLength= ::atof( table->getTableData()[table->getColId("fwhmb")][row].c_str());
        double angle= ::atof(table->getTableData()[table->getColId("pa")][row].c_str());
        double ra= ::atof(table->getTableData()[table->getColId("glon")][row].c_str());
        double dec= ::atof(table->getTableData()[table->getColId("glat")][row].c_str());

        double *coord= new double[3];

        AstroUtils().sky2xy(vtkwin->filenameWithPath,ra,dec,coord);


        vtkEllipse *el= new vtkEllipse(semiMajorAxisLength/2,semiMinorAxisLength/2,angle, coord[0], coord[1], coord[2], 0,0, cur->text(), NULL);
        drawSingleEllipse(el);
        */
}




void HigalSelectedSources::drawSingleEllipse(vtkEllipse * ellipse )
{

    vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();

    cleanFilter->SetInputData(ellipse->getPolyData());


    cleanFilter->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cleanFilter->GetOutputPort());

    ellipseActor = vtkSmartPointer<vtkLODActor>::New();
    ellipseActor->SetMapper(mapper);

    ellipseActor->GetProperty()->SetColor(0, 0, 0);
    vtkwin->drawSingleEllipse(ellipseActor);

}

HigalSelectedSources::~HigalSelectedSources()
{
    delete ui;
}


void HigalSelectedSources::plotNewWindow()
{

    QList<QListWidgetItem*> selectedItems = qobject_cast<QListWidget *>(ui->tabWidget->currentWidget())->selectedItems();
    if (selectedItems.isEmpty()){
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("Cannot create plot if no compact objects are selected.\n\rPlease select at least one compact object from the list and try again."));
        return;
    }
    PlotWindow *plotwin= new PlotWindow(vtkwin,selectedItems,plotWindowList.size());
    plotWindowList.append(plotwin);

    plotwin ->show();
    updateExistingWindowMenu();

}

void HigalSelectedSources::on_datasetButton_clicked()
{

    QList<QListWidgetItem*> selectedItems = qobject_cast<QListWidget *>(ui->tabWidget->currentWidget())->selectedItems();
    if (selectedItems.isEmpty()){
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("Cannot show dataset if no compact objects are selected.\n\rPlease select at least one compact object from the list and try again."));
        return;
    }

    selectedSourceFieldsSelect *selectFields = new selectedSourceFieldsSelect(vtkwin,selectedItems);
    selectFields->show();

}

void HigalSelectedSources::updateExistingWindowMenu()
{

    existingWindowMenu->clear();

    for(int i=0;i< plotWindowList.count();i++)
    {

        if(plotWindowList[i]->isVisible())
        {
            QAction*   newWindowAction = new QAction(plotWindowList[i]->windowTitle(), this);
            existingWindowMenu->addAction(newWindowAction);
        }


    }
}

void HigalSelectedSources::on_selectAllButton_clicked()
{
    QListWidget *list=qobject_cast<QListWidget *>(ui->tabWidget->currentWidget());

    for(int i=0;i< list->count();i++)
    {
        list->item(i)->setSelected(true);
    }
}

void HigalSelectedSources::on_deselectAllButton_clicked()
{
    QListWidget *list=qobject_cast<QListWidget *>(ui->tabWidget->currentWidget());
    for(int i=0;i< list->count();i++)
    {
        list->item(i)->setSelected(false);
    }
}

void HigalSelectedSources::on_sedButton_clicked()
{

    QString wave=QString::number(vtkwin->file_wavelength.value(ui->tabWidget->tabText(ui->tabWidget->currentIndex())));
    if( wave.compare("0") ==0 )
        wave=ui->tabWidget->tabText(ui->tabWidget->currentIndex());

    qDebug()<<"SELECTED: "<<ui->tabWidget->tabText(ui->tabWidget->currentIndex())<<" -> "<<wave;

    QList<QListWidgetItem*> selectedItems =qobject_cast<QListWidget *>(ui->tabWidget->currentWidget())->selectedItems();
    if (selectedItems.isEmpty()){
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("Cannot retrieve SED data if no compact object is selected.\n\rPlease select at least one compact object from the list and try again."));
        return;
    }
    for(int i=0;i<selectedItems.size();i++)
    {

        //QString query="SELECT * FROM vlkb_compactsources.bandmerged_sed_view where designation"+wave;
        //   QString query="SELECT * FROM vlkb_compactsources.sed_view_final where designation"+wave;

        // QString query="SELECT * FROM vlkb_compactsources.sed_view_final where numidtree";
        // query+="='"+QString::number(vtkwin->getFtEllipseList().value(selectedItems.at(i)->text())->getNumidtree())+"'";

       // QString query="SELECT 1 as flux22, 0.2 as err_flux22, * FROM vlkb_compactsources.sed_view_final where designationft=";
        QString query="SELECT  * FROM vlkb_compactsources.sed_view_final where designationft=";
        query+="'"+vtkwin->getFtEllipseList().value(selectedItems.at(i)->text())->getSourceName()+"'";


        new VLKBQuery(query,vtkwin);
        qDebug()<< query;
    }
    if(ellipseActor)
        vtkwin->removeSingleEllipse(ellipseActor);

    //this->close();
}

void HigalSelectedSources::closeEvent(QCloseEvent *event)
{
    itemSelectionChanged();
    event->accept();
    vtkwin->activateWindow();
}

void HigalSelectedSources::on_tabWidget_currentChanged(int index)
{
    qobject_cast<QListWidget *>(ui->tabWidget->currentWidget())->clearSelection();
}
