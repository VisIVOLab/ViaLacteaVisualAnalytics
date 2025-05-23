#ifndef VIALACTEA_H
#define VIALACTEA_H

#include <QDir>
#include <QMainWindow>
#include <QMap>
#include <QPair>
#include <QPointer>
#include <QSettings>

class AboutForm;
class SettingForm;
class SimpleConeSearchForm;
class HiPS2FITSForm;

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

    // VLKB URL
    static const QString VLKB_BASE_URL;

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
    void openLocalImage(const QString &fn);
    void on_actionSettings_triggered();
    void openLocalDC(const QString &fn);
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
    void on_actionConeSearch_triggered();
    void on_actionHiPS2FITS_triggered();
    void on_openLoadDataPushButton_clicked();
    void on_actionLoadArepo_triggered();

private:
    Ui::ViaLactea *ui;
    QPointer<AboutForm> aboutForm;
    QPointer<SettingForm> settingForm;
    QPointer<SimpleConeSearchForm> coneForm;
    QPointer<HiPS2FITSForm> hipsForm;

    // for javascript communication procedures
    WebProcess *webobj;

    QString selectedBand;
    QString settingsFile;
    QSettings settings;
    QString tilePath;
    QMap<int, QPair<QString, QString>> mapSurvey;

    void sessionScan(const QString &currentDir, const QDir &rootDir, QStringList &results);

protected:
    void closeEvent(QCloseEvent *);
};

#endif // VIALACTEA_H
