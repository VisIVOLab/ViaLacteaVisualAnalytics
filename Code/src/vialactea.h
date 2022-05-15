#ifndef VIALACTEA_H
#define VIALACTEA_H

#include "aboutform.h"
#include "settingform.h"
#include "vtkwindow_new.h"

#include <QMainWindow>
#include <QMap>
#include <QPair>
#include <QPointer>

#include "pqPipelineSource.h"


namespace Ui {
class ViaLactea;
}

class WebProcess : public QObject
{
    Q_OBJECT
    
public:
    explicit WebProcess(QObject *parent = nullptr);
    
    public slots:
    void jsCall(const QString &point, const QString &radius);
    
signals:
    void processJavascript(const QString &point, const QString &radius);
};

class ViaLactea : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit ViaLactea(QWidget *parent = 0);
    ~ViaLactea();
    void reload();
    bool isMasterWin(vtkwindow_new *win);
    void resetMasterWin();
    void setMasterWin(vtkwindow_new *win);
    
    // VLKB URLs
    static const QString ONLINE_TILE_PATH;
    static const QString VLKB_URL_IA2;
    static const QString TAP_URL_IA2;
    static const QString VLKB_URL_NEANIAS;
    static const QString TAP_URL_NEANIAS;
    
    private slots:
    void quitApp(); // Added page delete befor main app quits
    void on_queryPushButton_clicked();
    void on_noneRadioButton_clicked(bool checked);
    void on_saveToDiskCheckBox_clicked(bool checked);
    void on_selectFsPushButton_clicked();
    void on_webViewRegionSelected(const QString &point, const QString &area);
    void on_glonLineEdit_textChanged(const QString &arg1);
    void on_glatLineEdit_textChanged(const QString &arg1);
    void on_radiumLineEdit_textChanged(const QString &arg1);
    void queryButtonStatusOnOff();
    void on_openLocalImagePushButton_clicked();
    void on_actionSettings_triggered();
    void on_localDCPushButton_clicked();
    void on_actionExit_triggered();
    void on_actionAbout_triggered();
    void on_select3dPushButton_clicked();
    void on_actionLoad_SED_2_triggered();
    void on_pointRadioButton_clicked(bool checked);
    void on_rectRadioButton_clicked(bool checked);
    void on_dlLineEdit_textChanged(const QString &arg1);
    void on_dbLineEdit_textChanged(const QString &arg1);
    void updateVLKBSetting();
    void on_actionLoad_session_triggered();
    
    void on_loadTableButton_clicked();
    
private:
    Ui::ViaLactea *ui;
    QPointer<AboutForm> aboutForm;
    QPointer<SettingForm> settingForm;
    
    // for javascript communication procedures
    WebProcess *webobj;
    
    QString selectedBand;
    QString m_sSettingsFile;
    QString tilePath;
    QMap<int, QPair<QString, QString>> mapSurvey;
    vtkwindow_new *masterWin = nullptr;
    
    bool canImportToMasterWin(std::string importFn);
    void sessionScan(const QString &currentDir, const QDir &rootDir, QStringList &results);
    
protected:
    void closeEvent(QCloseEvent *);
    
protected slots:
    void onDataLoaded(pqPipelineSource *);
    
    
};

#endif // VIALACTEA_H
