#ifndef SETTINGFORM_H
#define SETTINGFORM_H

#include "authwrapper.h"

#include <QWidget>

namespace Ui {
class SettingForm;
}

class SettingForm : public QWidget
{
    Q_OBJECT

public:
    explicit SettingForm(QWidget *parent = 0);
    ~SettingForm();
    void readSettingsFromFile();

private slots:
    void on_TilePushButton_clicked();
    void on_workdirButton_clicked();
    void on_pushButton_clicked();
    void on_OkPushButton_clicked();
    void on_vlkbLoginButton_clicked();
    void on_vlkbLogoutButton_clicked();
    void on_caesarLoginButton_clicked();
    void on_caesarLogoutButton_clicked();
    void vlkb_loggedin();
    void vlkb_loggedout();
    void caesar_loggedin();
    void caesar_loggedout();
    void on_btnRegister_clicked();
    void on_btnLocatePython_clicked();
    void on_btnBrowsePython_clicked();

private:
    Ui::SettingForm *ui;
    QString m_settingsFile;
    AuthWrapper *m_ia2VlkbAuth;
    AuthWrapper *m_caesarAuth;
};

#endif // SETTINGFORM_H
