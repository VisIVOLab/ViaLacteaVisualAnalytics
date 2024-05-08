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
    QMenu *cameraMenu;
    QMenu *momentMenu;
    QMenu *viewMenu;
    QMenu *wcsMenu;
    QMenu *actionMenu;

private slots:
    void exitApplication();
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

};

#endif // VISIVOMENU_H
