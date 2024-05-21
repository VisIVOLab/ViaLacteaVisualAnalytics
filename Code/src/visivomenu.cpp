// visivomenu.cpp
#include "visivomenu.h"
#include <QAction>
#include <QMessageBox>
#include <QApplication>
#include "stdio.h"
#include "iostream"
#include "qdebug.h"

VisIVOMenu::VisIVOMenu(QWidget *parent) : QMenuBar(parent)
{
    fileMenu = addMenu("File");

    fileLoadMenu = fileMenu->addMenu("Load");
    actionLoadLocalFitsFile = fileLoadMenu->addAction("Load Local FITS");
    connect(actionLoadLocalFitsFile, &QAction::triggered, this, &VisIVOMenu::actionLoadLocalFitsTriggered);
    
    actionAddFitsFile = fileLoadMenu->addAction("Add new FITS file");
    connect(actionAddFitsFile, &QAction::triggered, this, &VisIVOMenu::actionAddFitsFileTriggered);

    fileAddCompactMenu = fileMenu->addMenu("Add compact sources");
    localCompactSources = fileAddCompactMenu->addAction("Local");
    connect(localCompactSources, &QAction::triggered, this, &VisIVOMenu::actionLocalCompactSourcesTriggered);
    jsonCompactSources = fileAddCompactMenu->addAction("From JSON catalogue");
    connect(localCompactSources, &QAction::triggered, this, &VisIVOMenu::actionJsonCompactSourcesTriggered);
    ds9CompactSources = fileAddCompactMenu->addAction("From DS9 Region");
    connect(ds9CompactSources, &QAction::triggered, this, &VisIVOMenu::actionDS9CompactSourcesTriggered);
    remoteCompactSources = fileAddCompactMenu->addAction("Remote");
    connect(remoteCompactSources, &QAction::triggered, this, &VisIVOMenu::actionRemoteCompactSourcesTriggered);
    tdCompactSources = fileAddCompactMenu->addAction("3D");
    connect(tdCompactSources, &QAction::triggered, this, &VisIVOMenu::action3DCompactSourcesTriggered);

    saveSessionFile = fileMenu->addAction("Save Session");
    connect(saveSessionFile, &QAction::triggered, this, &VisIVOMenu::actionSaveSessionTriggered);

    fileMenu->addSeparator();

    QAction *exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &VisIVOMenu::exitApplication);

    actionMenu= addMenu("Action");
    actionExtract_spectrum = actionMenu->addAction("Extract Spectrum");
    connect(actionExtract_spectrum, &QAction::triggered, this, &VisIVOMenu::actionExtract_spectrumTriggered);
    actionPV = actionMenu->addAction("Position-Velocity plot");
    connect(actionPV, &QAction::triggered, this, &VisIVOMenu::actionPVTriggered);
    actionFilter = actionMenu->addAction("Spatial Filter");
    connect(actionFilter, &QAction::triggered, this, &VisIVOMenu::actionFilterTriggered);

    cameraMenu = addMenu("Camera");
    actionFront = cameraMenu->addAction("Front");
    actionFront->setIcon(QIcon(":/icons/PIC_FRONT.bmp"));
    connect(actionFront, &QAction::triggered, this, &VisIVOMenu::actionFrontTriggered);
    actionBack = cameraMenu->addAction("Back");
    actionBack->setIcon(QIcon(":/icons/PIC_BACK.bmp"));
    connect(actionBack, &QAction::triggered, this, &VisIVOMenu::actionBackTriggered);
    actionTop = cameraMenu->addAction("Top");
    actionTop->setIcon(QIcon(":/icons/PIC_TOP.bmp"));
    connect(actionTop, &QAction::triggered, this, &VisIVOMenu::actionTopTriggered);
    actionRight = cameraMenu->addAction("Right");
    actionRight->setIcon(QIcon(":/icons/PIC_RIGHT.bmp"));
    connect(actionRight, &QAction::triggered, this, &VisIVOMenu::actionRightTriggered);
    actionBottom = cameraMenu->addAction("Bottom");
    actionBottom->setIcon(QIcon(":/icons/PIC_BOTTOM.bmp"));
    connect(actionBottom, &QAction::triggered, this, &VisIVOMenu::actionBottomTriggered);
    actionLeft = cameraMenu->addAction("Left");
    actionLeft->setIcon(QIcon(":/icons/PIC_LEFT.bmp"));
    connect(actionLeft, &QAction::triggered, this, &VisIVOMenu::actionLeftTriggered);

    cameraMenu = addMenu("Moment");
    actionCalculate_order_0 = cameraMenu->addAction("0 - the integrated value of the spectrum");
    connect(actionCalculate_order_0, &QAction::triggered, this, &VisIVOMenu::actionCalculate_order_0Triggered);
    actionCalculate_order_1 = cameraMenu->addAction("1 - the intensity weighted coordinate");
    connect(actionCalculate_order_1, &QAction::triggered, this, &VisIVOMenu::actionCalculate_order_1Triggered);
    actionCalculate_order_2 = cameraMenu->addAction("2 - the intensity weighted dispersion of the coordinate");
    connect(actionCalculate_order_2, &QAction::triggered, this, &VisIVOMenu::actionCalculate_order_2Triggered);
    actionCalculate_order_6 = cameraMenu->addAction("6 - root mean square of the spectrum (noise map)");
    connect(actionCalculate_order_6, &QAction::triggered, this, &VisIVOMenu::actionCalculate_order_6Triggered);
    actionCalculate_order_8 = cameraMenu->addAction("8 - maximum value of the spectrum (peak map)");
    connect(actionCalculate_order_8, &QAction::triggered, this, &VisIVOMenu::actionCalculate_order_8Triggered);
    actionCalculate_order_10 = cameraMenu->addAction("10 - the minimum value of the spectrum");
    connect(actionCalculate_order_10, &QAction::triggered, this, &VisIVOMenu::actionCalculate_order_10Triggered);

    toolsMenu= addMenu("Tools");
    actionSourceFinders = toolsMenu->addAction("Source Finders");
    connect(actionSourceFinders, &QAction::triggered, this, &VisIVOMenu::actionSouceFindersTriggered);
    actionProfileFinders = toolsMenu->addAction("Profile");
    connect(actionProfileFinders, &QAction::triggered, this, &VisIVOMenu::actionProfileTriggered);

    viewMenu = addMenu("View");
    actionSlice_Lookup_Table = viewMenu->addAction("Edit Lookup Table");
    connect(actionSlice_Lookup_Table, &QAction::triggered, this, &VisIVOMenu::actionSlice_Lookup_TableTriggered);
    viewMenu->addSeparator();
    // Setup context menu to toggle slice/moment renderers

    /*
    QAction *actionShowSlice = viewMenu->addAction("Show Slice");
    connect(actionShowSlice, &QAction::triggered, this, &VisIVOMenu::actionShowSliceTriggered);
    QAction *actionShowMomentMap = viewMenu->addAction("Show Moment Map");
    connect(actionShowMomentMap, &QAction::triggered, this, &VisIVOMenu::actionShowMomentMapTriggered);
   */

    QActionGroup *grp = new QActionGroup(this);
    actionShowSlice = new QAction("Show Slice",grp);
    actionShowSlice->setCheckable(true);
    actionShowSlice->setChecked(true);
    connect(actionShowSlice, &QAction::triggered, this, &VisIVOMenu::actionShowSliceTriggered);
    actionShowMomentMap = new QAction("Show Moment Map",grp);
    actionShowMomentMap->setCheckable(true);
    actionShowMomentMap->setChecked(false);
    connect(actionShowMomentMap, &QAction::triggered, this, &VisIVOMenu::actionShowMomentMapTriggered);
    grp->addAction(actionShowSlice);
    grp->addAction(actionShowMomentMap);
    viewMenu->addActions(grp->actions());

    wcsMenu = addMenu("WCS");
    
    wcsGroup = new QActionGroup(this);
    auto wcsItem = new QAction("Galactic", wcsGroup);
    wcsItem->setCheckable(true);
    wcsItem->setChecked(true);
    wcsGroup->addAction(wcsItem);
    connect(wcsItem, &QAction::triggered, this, &VisIVOMenu::actionChangeWCSGalactic);

    wcsItem = new QAction("FK5", wcsGroup);
    wcsItem->setCheckable(true);
    wcsGroup->addAction(wcsItem);
    connect(wcsItem, &QAction::triggered, this, &VisIVOMenu::actionChangeWCSFk5);


    wcsItem = new QAction("FK4", wcsGroup);
    wcsItem->setCheckable(true);
    wcsGroup->addAction(wcsItem);
    connect(wcsItem, &QAction::triggered, this, &VisIVOMenu::actionChangeWCSFk4);

    //   connect(wcsItem, &QAction::triggered, this, [=]() { changeLegendWCS(WCS_B1950); });

    wcsItem = new QAction("Ecliptic", wcsGroup);
    wcsItem->setCheckable(true);
    wcsGroup->addAction(wcsItem);
    connect(wcsItem, &QAction::triggered, this, &VisIVOMenu::actionChangeWCSEcliptic);

    //   connect(wcsItem, &QAction::triggered, this, [=]() { changeLegendWCS(WCS_ECLIPTIC); });
    wcsMenu->addActions(wcsGroup->actions());

    windowMenu= addMenu("Window");
    actionInfoWindow = windowMenu->addAction("Info");
    connect(actionInfoWindow, &QAction::triggered, this, &VisIVOMenu::actionInfoWindowTriggered);
    actionSelectWindow = windowMenu->addAction("Select");
    connect(actionSelectWindow, &QAction::triggered, this, &VisIVOMenu::actionSelectWindowTriggered);
    actionExtractWindow = windowMenu->addAction("Extract");
    connect(actionExtractWindow, &QAction::triggered, this, &VisIVOMenu::actionExtractWindowTriggered);
    actionFilterWindow = windowMenu->addAction("Filter");
    connect(actionFilterWindow, &QAction::triggered, this, &VisIVOMenu::actionFilterWindowTriggered);

}


//file menu actions
void VisIVOMenu::actionLoadLocalFitsTriggered()
{
    emit loadLocalFitsFileRequested();
}
void VisIVOMenu::actionAddFitsFileTriggered()
{

}
void VisIVOMenu::actionLocalCompactSourcesTriggered()
{

}
void VisIVOMenu::actionJsonCompactSourcesTriggered()
{

}
void VisIVOMenu::actionDS9CompactSourcesTriggered()
{

}
void VisIVOMenu::actionRemoteCompactSourcesTriggered()
{

}
void VisIVOMenu::action3DCompactSourcesTriggered()
{

}
void VisIVOMenu::actionSaveSessionTriggered()
{
    emit saveSessionTriggered();
}

void VisIVOMenu::exitApplication()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Exit", "Do you really want to exit?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
        qApp->quit();
}

//Camera menu actions
void VisIVOMenu::actionFrontTriggered()
{
    emit cameraFrontTriggered();
}

void VisIVOMenu::actionBackTriggered()
{
    emit cameraBackTriggered();
}

void VisIVOMenu::actionTopTriggered()
{
    emit cameraTopTriggered();
}

void VisIVOMenu::actionRightTriggered()
{
    emit cameraRightTriggered();
}

void VisIVOMenu::actionBottomTriggered()
{
    emit cameraBottomTriggered();
}

void VisIVOMenu::actionLeftTriggered()
{
    emit cameraLeftTriggered();
}

//moment
void VisIVOMenu::actionCalculate_order_0Triggered()
{
    emit calculate_order_0Triggered();
}

void VisIVOMenu::actionCalculate_order_1Triggered()
{
    emit calculate_order_1Triggered();
}

void VisIVOMenu::actionCalculate_order_2Triggered()
{
    emit calculate_order_2Triggered();
}

void VisIVOMenu::actionCalculate_order_6Triggered()
{
    emit calculate_order_6Triggered();
}

void VisIVOMenu::actionCalculate_order_8Triggered()
{
    emit calculate_order_8Triggered();
}

void VisIVOMenu::actionCalculate_order_10Triggered()
{
    emit calculate_order_10Triggered();
}

void VisIVOMenu::actionSlice_Lookup_TableTriggered()
{
    emit sliceLookupTableTriggered();
}

void VisIVOMenu::actionShowSliceTriggered()
{
    emit showSliceTriggered();
}

void VisIVOMenu::actionShowMomentMapTriggered()
{
    emit showMomentMapTriggered();
}

void VisIVOMenu::actionExtract_spectrumTriggered()
{
    emit extractSpectrumTriggered();
}

void VisIVOMenu::actionPVTriggered()
{
    emit pvTriggered();
}

void VisIVOMenu::actionFilterTriggered()
{
    emit spatialFilterTriggered();
}

//window
void VisIVOMenu::actionInfoWindowTriggered()
{

}
void VisIVOMenu::actionSelectWindowTriggered()
{

}
void VisIVOMenu::actionExtractWindowTriggered()
{

}
void VisIVOMenu::actionFilterWindowTriggered()
{

}
//tools
void VisIVOMenu::actionSouceFindersTriggered()
{

}
void VisIVOMenu::actionProfileTriggered()
{

}

//wcs
void VisIVOMenu::actionChangeWCSGalactic()
{
    emit changeWCSGalacticTriggered();
}
void VisIVOMenu::actionChangeWCSFk5()
{
    emit changeWCSFk5Triggered();
}
void VisIVOMenu::actionChangeWCSFk4()
{
    emit changeWCSFk4Triggered();
}
void VisIVOMenu::actionChangeWCSEcliptic()
{
    emit changeWCSEclipticTriggered();
}

void VisIVOMenu::configureStartupMenu()
{
    actionAddFitsFile->setVisible(false);
    fileAddCompactMenu->setVisible(false);
    fileMenu->removeAction(fileAddCompactMenu->menuAction());

    localCompactSources->setVisible(false);
    jsonCompactSources->setVisible(false);
    ds9CompactSources->setVisible(false);
    remoteCompactSources->setVisible(false);
    tdCompactSources->setVisible(false);
    
    saveSessionFile->setVisible(false);
    actionExtract_spectrum->setVisible(false);
    actionPV->setVisible(false);
    actionFilter->setVisible(false);
    actionFront->setVisible(false);
    actionBack->setVisible(false);
    actionTop->setVisible(false);
    actionRight->setVisible(false);
    actionBottom->setVisible(false);
    actionLeft->setVisible(false);
    actionCalculate_order_0->setVisible(false);
    actionCalculate_order_1->setVisible(false);
    actionCalculate_order_2->setVisible(false);
    actionCalculate_order_6->setVisible(false);
    actionCalculate_order_8->setVisible(false);
    actionCalculate_order_10->setVisible(false);
    actionSourceFinders->setVisible(false);
    actionProfileFinders->setVisible(false);
    actionSlice_Lookup_Table->setVisible(false);
    actionShowSlice->setVisible(false);
    actionShowMomentMap->setVisible(false);
    wcsMenu->setVisible(false);
    foreach (QAction *action, wcsGroup->actions())
        wcsMenu->removeAction(action);
    actionInfoWindow->setVisible(false);
    actionSelectWindow->setVisible(false);
    actionExtractWindow->setVisible(false);
    actionFilterWindow->setVisible(false);
}

void VisIVOMenu::configureCubeWindowMenu()
{
    actionAddFitsFile->setVisible(false);
    fileAddCompactMenu->setVisible(false);
    fileMenu->removeAction(fileAddCompactMenu->menuAction());
    saveSessionFile->setVisible(false);
    actionExtract_spectrum->setVisible(true);
    actionPV->setVisible(true);
    actionFilter->setVisible(true);
    actionFront->setVisible(true);
    actionBack->setVisible(true);
    actionTop->setVisible(true);
    actionRight->setVisible(true);
    actionBottom->setVisible(true);
    actionLeft->setVisible(true);
    actionCalculate_order_0->setVisible(true);
    actionCalculate_order_1->setVisible(true);
    actionCalculate_order_2->setVisible(true);
    actionCalculate_order_6->setVisible(true);
    actionCalculate_order_8->setVisible(true);
    actionCalculate_order_10->setVisible(true);
    actionSourceFinders->setVisible(false);
    actionProfileFinders->setVisible(false);
    actionSlice_Lookup_Table->setVisible(true);
    actionShowSlice->setVisible(true);
    actionShowMomentMap->setVisible(true);
    wcsMenu->setVisible(true);
    foreach (QAction *action, wcsGroup->actions())
        wcsMenu->addAction(action);

    actionInfoWindow->setVisible(false);
    actionSelectWindow->setVisible(false);
    actionExtractWindow->setVisible(false);
    actionFilterWindow->setVisible(false);

}
void VisIVOMenu::configureImageWindowMenu()
{
    actionAddFitsFile->setVisible(true);
    fileAddCompactMenu->setVisible(true);
    fileMenu->addAction(fileAddCompactMenu->menuAction());
    saveSessionFile->setVisible(true);
    actionExtract_spectrum->setVisible(false);
    actionPV->setVisible(false);
    actionFilter->setVisible(false);
    actionFront->setVisible(false);
    actionBack->setVisible(false);
    actionTop->setVisible(false);
    actionRight->setVisible(false);
    actionBottom->setVisible(false);
    actionLeft->setVisible(false);
    actionCalculate_order_0->setVisible(false);
    actionCalculate_order_1->setVisible(false);
    actionCalculate_order_2->setVisible(false);
    actionCalculate_order_6->setVisible(false);
    actionCalculate_order_8->setVisible(false);
    actionCalculate_order_10->setVisible(false);
    actionSourceFinders->setVisible(true);
    actionProfileFinders->setVisible(true);
    actionSlice_Lookup_Table->setVisible(false);
    actionShowSlice->setVisible(false);
    actionShowMomentMap->setVisible(false);
    wcsMenu->setVisible(true);
    foreach (QAction *action, wcsGroup->actions())
        wcsMenu->addAction(action);

    actionInfoWindow->setVisible(true);
    actionSelectWindow->setVisible(true);
    actionExtractWindow->setVisible(true);
    actionFilterWindow->setVisible(true);
}
