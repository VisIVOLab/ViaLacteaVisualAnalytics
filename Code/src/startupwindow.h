#ifndef STARTUPWINDOW_H
#define STARTUPWINDOW_H

#include <QWidget>
#include <QSettings>
#include <QPointer>

namespace Ui {
class StartupWindow;
}

class RecentFilesManager;
class SettingForm;

class StartupWindow : public QWidget
{
    Q_OBJECT

public:
    explicit StartupWindow(QWidget *parent = nullptr);
    ~StartupWindow();

private slots:
    void on_localOpenPushButton_clicked(bool fromHistory);
    void openLocalDC(const QString &fn);
    void on_clearPushButton_clicked();

    void on_settingsPushButton_clicked();

    void on_openPushButton_clicked();

private:
    Ui::StartupWindow *ui;
    RecentFilesManager *historyModel;
    QString settingsFile;
    QSettings settings;
    QPointer<SettingForm> settingForm;

};

#endif // STARTUPWINDOW_H
