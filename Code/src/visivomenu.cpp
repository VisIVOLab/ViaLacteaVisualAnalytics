// visivomenu.cpp
#include "visivomenu.h"
#include <QAction>
#include <QMessageBox>
#include <QApplication>

VisIVOMenu::VisIVOMenu(QWidget *parent) : QMenuBar(parent)
{
    fileMenu = addMenu("File");

    QAction *actionAddFitsFile = fileMenu->addAction("Add new FITS file");
    connect(actionAddFitsFile, &QAction::triggered, this, &VisIVOMenu::actionAddFitsFileTriggered);

    fileAddCompactMenu = fileMenu->addMenu("Add compact sources");
    QAction *localCompactSources = fileAddCompactMenu->addAction("Local");
    connect(localCompactSources, &QAction::triggered, this, &VisIVOMenu::actionLocalCompactSourcesTriggered);
    QAction *jsonCompactSources = fileAddCompactMenu->addAction("From JSON catalogue");
    connect(localCompactSources, &QAction::triggered, this, &VisIVOMenu::actionJsonCompactSourcesTriggered);
    QAction *ds9CompactSources = fileAddCompactMenu->addAction("From DS9 Region");
    connect(ds9CompactSources, &QAction::triggered, this, &VisIVOMenu::actionDS9CompactSourcesTriggered);
    QAction *remoteCompactSources = fileAddCompactMenu->addAction("Remote");
    connect(remoteCompactSources, &QAction::triggered, this, &VisIVOMenu::actionRemoteCompactSourcesTriggered);
    QAction *tdCompactSources = fileAddCompactMenu->addAction("3D");
    connect(tdCompactSources, &QAction::triggered, this, &VisIVOMenu::action3DCompactSourcesTriggered);

    QAction *saveSessionFile = fileMenu->addAction("Save Session");
    connect(saveSessionFile, &QAction::triggered, this, &VisIVOMenu::actionSaveSessionTriggered);

    fileMenu->addSeparator();

    QAction *exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &VisIVOMenu::exitApplication);

    actionMenu= addMenu("Action");
    QAction *actionExtract_spectrum = actionMenu->addAction("Extract Spectrum");
    connect(actionExtract_spectrum, &QAction::triggered, this, &VisIVOMenu::actionExtract_spectrumTriggered);
    QAction *actionPV = actionMenu->addAction("Position-Velocity plot");
    connect(actionPV, &QAction::triggered, this, &VisIVOMenu::actionPVTriggered);
    QAction *actionFilter = actionMenu->addAction("Spatial Filter");
    connect(actionFilter, &QAction::triggered, this, &VisIVOMenu::actionFilterTriggered);

    cameraMenu = addMenu("Camera");
    QAction *actionFront = cameraMenu->addAction("Front");
    connect(actionFront, &QAction::triggered, this, &VisIVOMenu::actionFrontTriggered);
    QAction *actionBack = cameraMenu->addAction("Back");
    connect(actionBack, &QAction::triggered, this, &VisIVOMenu::actionBackTriggered);
    QAction *actionTop = cameraMenu->addAction("Top");
    connect(actionTop, &QAction::triggered, this, &VisIVOMenu::actionTopTriggered);
    QAction *actionRight = cameraMenu->addAction("Right");
    connect(actionRight, &QAction::triggered, this, &VisIVOMenu::actionRightTriggered);
    QAction *actionBottom = cameraMenu->addAction("Front");
    connect(actionBottom, &QAction::triggered, this, &VisIVOMenu::actionBottomTriggered);
    QAction *actionLeft = cameraMenu->addAction("Left");
    connect(actionLeft, &QAction::triggered, this, &VisIVOMenu::actionLeftTriggered);

    cameraMenu = addMenu("Moment");
    QAction *actionCalculate_order_0 = cameraMenu->addAction("0 - the integrated value of the spectrum");
    connect(actionCalculate_order_0, &QAction::triggered, this, &VisIVOMenu::actionCalculate_order_0Triggered);
    QAction *actionCalculate_order_1 = cameraMenu->addAction("1 - the intensity weighted coordinate");
    connect(actionCalculate_order_1, &QAction::triggered, this, &VisIVOMenu::actionCalculate_order_1Triggered);
    QAction *actionCalculate_order_2 = cameraMenu->addAction("2 - the intensity weighted dispersion of the coordinate");
    connect(actionCalculate_order_2, &QAction::triggered, this, &VisIVOMenu::actionCalculate_order_2Triggered);
    QAction *actionCalculate_order_6 = cameraMenu->addAction("6 - root mean square of the spectrum (noise map)");
    connect(actionCalculate_order_6, &QAction::triggered, this, &VisIVOMenu::actionCalculate_order_6Triggered);
    QAction *actionCalculate_order_8 = cameraMenu->addAction("8 - maximum value of the spectrum (peak map)");
    connect(actionCalculate_order_8, &QAction::triggered, this, &VisIVOMenu::actionCalculate_order_8Triggered);
    QAction *actionCalculate_order_10 = cameraMenu->addAction("10 - the minimum value of the spectrum");
    connect(actionCalculate_order_10, &QAction::triggered, this, &VisIVOMenu::actionCalculate_order_10Triggered);

    toolsMenu= addMenu("Tools");
    QAction *actionSourceFinders = toolsMenu->addAction("Source Finders");
    connect(actionSourceFinders, &QAction::triggered, this, &VisIVOMenu::actionSouceFindersTriggered);
    QAction *actionProfileFinders = toolsMenu->addAction("Profile");
    connect(actionProfileFinders, &QAction::triggered, this, &VisIVOMenu::actionProfileTriggered);

    viewMenu = addMenu("View");
    QAction *actionSlice_Lookup_Table = viewMenu->addAction("Edit Lookup Table");
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
    QAction *actionShowSlice = new QAction("Show Slice",grp);
    actionShowSlice->setCheckable(true);
    actionShowSlice->setChecked(true);
    QAction *actionShowMomentMap = new QAction("Show Moment Map",grp);
    actionShowMomentMap->setCheckable(true);
    actionShowMomentMap->setChecked(false);
    grp->addAction(actionShowSlice);
    grp->addAction(actionShowMomentMap);
    viewMenu->addActions(grp->actions());

    wcsMenu = addMenu("WCS");
    auto wcsGroup = new QActionGroup(this);
    auto wcsItem = new QAction("Galactic", wcsGroup);
    wcsItem->setCheckable(true);
    wcsItem->setChecked(true);
    wcsGroup->addAction(wcsItem);
    //   connect(wcsItem, &QAction::triggered, this, [=]() { changeLegendWCS(WCS_GALACTIC); });

    wcsItem = new QAction("FK5", wcsGroup);
    wcsItem->setCheckable(true);
    wcsGroup->addAction(wcsItem);
    //   connect(wcsItem, &QAction::triggered, this, [=]() { changeLegendWCS(WCS_J2000); });

    wcsItem = new QAction("FK4", wcsGroup);
    wcsItem->setCheckable(true);
    wcsGroup->addAction(wcsItem);
    //   connect(wcsItem, &QAction::triggered, this, [=]() { changeLegendWCS(WCS_B1950); });

    wcsItem = new QAction("Ecliptic", wcsGroup);
    wcsItem->setCheckable(true);
    wcsGroup->addAction(wcsItem);
    //   connect(wcsItem, &QAction::triggered, this, [=]() { changeLegendWCS(WCS_ECLIPTIC); });
    wcsMenu->addActions(wcsGroup->actions());

    windowMenu= addMenu("Window");
    QAction *actionInfoWindow = windowMenu->addAction("Info");
    connect(actionInfoWindow, &QAction::triggered, this, &VisIVOMenu::actionInfoWindowTriggered);
    QAction *actionSelectWindow = windowMenu->addAction("Select");
    connect(actionSelectWindow, &QAction::triggered, this, &VisIVOMenu::actionSelectWindowTriggered);
    QAction *actionExtractWindow = windowMenu->addAction("Extract");
    connect(actionExtractWindow, &QAction::triggered, this, &VisIVOMenu::actionExtractWindowTriggered);
    QAction *actionFilterWindow = windowMenu->addAction("Filter");
    connect(actionFilterWindow, &QAction::triggered, this, &VisIVOMenu::actionFilterWindowTriggered);

}


//file menu actions
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

}

void VisIVOMenu::actionBackTriggered()
{

}

void VisIVOMenu::actionTopTriggered()
{

}

void VisIVOMenu::actionRightTriggered()
{

}

void VisIVOMenu::actionBottomTriggered()
{

}

void VisIVOMenu::actionLeftTriggered()
{

}

//moment
void VisIVOMenu::actionCalculate_order_0Triggered()
{

}

void VisIVOMenu::actionCalculate_order_1Triggered()
{

}

void VisIVOMenu::actionCalculate_order_2Triggered()
{

}

void VisIVOMenu::actionCalculate_order_6Triggered()
{

}

void VisIVOMenu::actionCalculate_order_8Triggered()
{

}

void VisIVOMenu::actionCalculate_order_10Triggered()
{

}

void VisIVOMenu::actionSlice_Lookup_TableTriggered()
{

}

void VisIVOMenu::actionShowSliceTriggered()
{

}

void VisIVOMenu::actionShowMomentMapTriggered()
{

}

void VisIVOMenu::actionExtract_spectrumTriggered()
{

}

void VisIVOMenu::actionPVTriggered()
{

}

void VisIVOMenu::actionFilterTriggered()
{

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
