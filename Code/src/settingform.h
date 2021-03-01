#ifndef SETTINGFORM_H
#define SETTINGFORM_H

#include <QWidget>
#include "authwrapper.h"

namespace Ui {
class SettingForm;
}

class SettingForm : public QWidget
{
    Q_OBJECT

public:
    explicit SettingForm(QWidget *parent = 0);

    ~SettingForm();

private slots:
    void on_IdlPushButton_clicked();

    void on_TilePushButton_clicked();


    void on_pushButton_clicked();

    void on_checkBox_clicked(bool checked);

    void on_privateVLKB_radioButton_toggled(bool checked);
    void on_workdirButton_clicked();

    void on_checkBox_clicked();
    void on_OkPushButton_clicked();


    void on_neaniasVLKB_radioButton_toggled(bool checked);

    void on_loginButton_clicked();

private:
    Ui::SettingForm *ui;
    QString m_sSettingsFile;
    AuthWrapper *m_authWrapper;
};

#endif // SETTINGFORM_H
