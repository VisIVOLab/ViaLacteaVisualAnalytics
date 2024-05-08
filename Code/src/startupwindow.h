#ifndef STARTUPWINDOW_H
#define STARTUPWINDOW_H

#include <QPointer>
#include <QSettings>
#include <QWidget>

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
    void openLocalImage(const QString &fn);
    void showFitsHeader(const QString &fn);
    void on_clearPushButton_clicked();
    void on_settingsPushButton_clicked();
    void on_openPushButton_clicked();
    void on_vlkbPushButton_clicked();
    void on_historyArea_activated(const QModelIndex &index);
    void on_historyArea_clicked(const QModelIndex &index);

private:
    Ui::StartupWindow *ui;
    RecentFilesManager *historyModel;
    QString settingsFile;
    QSettings settings;
    QPointer<SettingForm> settingForm;
};

#endif // STARTUPWINDOW_H