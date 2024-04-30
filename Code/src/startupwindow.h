#ifndef STARTUPWINDOW_H
#define STARTUPWINDOW_H

#include <QWidget>
#include <QSettings>

namespace Ui {
class StartupWindow;
}

class RecentFilesManager;

class StartupWindow : public QWidget
{
    Q_OBJECT

public:
    explicit StartupWindow(QWidget *parent = nullptr);
    ~StartupWindow();

private slots:
    void on_localOpenPushButton_clicked();
    void openLocalDC(const QString &fn);
    void on_clearPushButton_clicked();

private:
    Ui::StartupWindow *ui;
    RecentFilesManager *historyModel;
    QString settingsFile;
    QSettings settings;
};

#endif // STARTUPWINDOW_H
