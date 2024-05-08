// visivomenu.h
#ifndef VISIVOMENU_H
#define VISIVOMENU_H

#include <QMenuBar>

class VisIVOMenu : public QMenuBar
{
    Q_OBJECT

public:
    explicit VisIVOMenu(QWidget *parent = nullptr);
private:
    QMenu *fileMenu;
    QMenu *fileAddCompactMenu;
    QMenu *cameraMenu;
    QMenu *momentMenu;
    QMenu *viewMenu;
    QMenu *wcsMenu;
    QMenu *actionMenu;
    QMenu *windowMenu;
    QMenu *toolsMenu;

private slots:
    //file
    void actionAddFitsFileTriggered();
    void actionLocalCompactSourcesTriggered();
    void actionJsonCompactSourcesTriggered();
    void actionDS9CompactSourcesTriggered();
    void actionRemoteCompactSourcesTriggered();
    void action3DCompactSourcesTriggered();
    void actionSaveSessionTriggered();
    void exitApplication();

    //action
    void actionFrontTriggered();
    void actionBackTriggered();
    void actionTopTriggered();
    void actionRightTriggered();
    void actionBottomTriggered();
    void actionLeftTriggered();
    //moment
    void actionCalculate_order_0Triggered();
    void actionCalculate_order_1Triggered();
    void actionCalculate_order_2Triggered();
    void actionCalculate_order_6Triggered();
    void actionCalculate_order_8Triggered();
    void actionCalculate_order_10Triggered();
    //view
    void actionSlice_Lookup_TableTriggered();
    void actionShowSliceTriggered();
    void actionShowMomentMapTriggered();
    //action
    void actionExtract_spectrumTriggered();
    void actionPVTriggered();
    void actionFilterTriggered();
    //window
    void actionInfoWindowTriggered();
    void actionSelectWindowTriggered();
    void actionExtractWindowTriggered();
    void actionFilterWindowTriggered();
    //tools
    void actionSouceFindersTriggered();
    void actionProfileTriggered();


};

#endif // VISIVOMENU_H
