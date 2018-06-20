#ifndef VIALACTEA_H
#define VIALACTEA_H

#include <QMainWindow>
#include <QMap>
#include <QPair>

namespace Ui {
class ViaLactea;
}

class ViaLactea : public QMainWindow
{
    Q_OBJECT

public:
    explicit ViaLactea(QWidget *parent = 0);
    void reload();

    ~ViaLactea();

private slots:
    void on_PLW_checkBox_clicked();
    void on_queryPushButton_clicked();
    void on_noneRadioButton_clicked(bool checked);
    void on_saveToDiskCheckBox_clicked(bool checked);
    void on_selectFsPushButton_clicked();
    void on_webView_statusBarMessage(const QString &text);
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

private:
    Ui::ViaLactea *ui;
    QString selectedBand;
    QString m_sSettingsFile;
    QString tilePath;
    QMap <int, QPair<QString, QString> > mapSurvey;

protected:
    void  closeEvent(QCloseEvent*);

};

#endif // VIALACTEA_H
