#include "visivofilterdesktop.h"
#include <QDebug>
#include "singleton.h"
#include "mainwindow.h"
extern "C" {
#include "visivo.h"
}

VisIVOFilterDesktop::VisIVOFilterDesktop()
{
}

void VisIVOFilterDesktop::runFilter(int index)
{

    switch (index) {
    //add identifier
    case 0:
        addIdentifier();
        break;
        //Append
    case 1:
        break;
        //Cartesian 2 Polar
    case 2:
        break;
        //cut
    case 3:
        break;
        //Decimator
    case 4:
        break;
        //Extract List
    case 5:
        break;
        //Extraction
    case 6:
        break;
        //Grid 2 Point
    case 7:
        break;
        //Include
    case 8:
        break;
        //Math Operation
    case 9:
        break;
        //Merge
    case 10:
        break;
        //Module
    case 11:
        break;
        //Multi Resolution
    case 12:
        break;
        //Point distribute
    case 13:
        break;
        //Point Property
    case 14:
        break;
        //Randomizer
    case 15:
        break;
        //Scalar distribution
    case 16:
        break;
        //Select Columns
    case 17:
        break;
        //Select Rows
    case 18:
        break;
        //Sigma Contours
    case 19:
        break;
        //Split tables
    case 20:
        break;
        //Write VO Table
    case 21:
        break;
    default:
        break;
    }


}

VisIVOFilterDesktop::~VisIVOFilterDesktop()
{

}






void VisIVOFilterDesktop::addIdentifier()
{
    MainWindow *w = &Singleton<MainWindow>::Instance();

    QModelIndex  selectedItemIndex  = w->ui->treeView->selectionModel()->selectedIndexes()[0];


    char *filepath = new char[w->selectedFile.toStdString().length() + 1];
    std::strcpy(filepath,w->selectedFile.toStdString().c_str());

    char *colname = new char[w->ui->addIdNewColName->text().toStdString().length() + 1];
    std::strcpy(colname,w->ui->addIdNewColName->text().toStdString().c_str());

    char *startnumber = new char[w->ui->addIdStartNumber->text().toStdString().length() + 1];
    std::strcpy(startnumber,w->ui->addIdStartNumber->text().toStdString().c_str());

    VisIVOFilter env;
    int init= VF_Init(&env);
    VF_SetAtt(&env, VF_SET_OPERATION,"addId");
    VF_SetAtt(&env, VF_SET_ADDIDSTART,startnumber);
    VF_SetAtt(&env, VF_SET_OUTCOL,colname);
    VF_SetAtt(&env, VF_SET_FILEVBT,filepath);
    VF_Filter(&env);


    w->m_VisIVOTreeModel->getTable(selectedItemIndex)->readHeader();
    w->m_VisIVOTreeModel->getTable(selectedItemIndex)->readStatistics();

    w->itemSelected();
    w->ui->tabWidget->setCurrentWidget(w->ui->tabObjectTree);


}
