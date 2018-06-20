#include "settingform.h"
#include "ui_settingform.h"
#include <QSettings>
#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include "vialactea.h"
#include "singleton.h"

SettingForm::SettingForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingForm)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::WindowStaysOnTopHint);

    m_sSettingsFile = QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp").append("/setting.ini");

    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    QString tilePath = settings.value("tilepath", "").toString();
    ui->TileLineEdit->setText(tilePath);

    QString idlPath = settings.value("idlpath", "").toString();
    ui->IdlLineEdit->setText(idlPath);

    QString glyphmax = settings.value("glyphmax", "").toString();
    ui->glyphLineEdit->setText(glyphmax);
    ui->groupBox_4->hide();


    if(settings.value("vlkbtype", "public")=="public")
    {
        ui->publicVLKB_radioButton->setChecked(true);

        ui->username_LineEdit->hide();
        ui->password_LineEdit->hide();
        ui->userLabel->hide();
        ui->passLabel->hide();
    }
    else if(settings.value("vlkbtype", "public")=="private")
    {
        ui->privateVLKB_radioButton->setChecked(true);
        ui->username_LineEdit->show();
        ui->password_LineEdit->show();
        ui->userLabel->show();
        ui->passLabel->show();

        ui->username_LineEdit->setText(settings.value("vlkbuser", "").toString());
        ui->password_LineEdit->setText(settings.value("vlkbpass", "").toString());
    }

    if (settings.value("online",false) == true)
    {
        ui->checkBox->setChecked(true);

    }


    ui->urlLineEdit->setText(settings.value("onlinetilepath", "http://visivo.oact.inaf.it/vialacteatiles/openlayers.html").toString());


}

SettingForm::~SettingForm()
{
    delete ui;
}

void SettingForm::on_IdlPushButton_clicked()
{
    QString fn = QFileDialog::getOpenFileName(this, "IDL bin", QString(), "idl");

    if (!fn.isEmpty() )
    {
        ui->IdlLineEdit->setText(fn);
    }
}

void SettingForm::on_TilePushButton_clicked()
{
    QString fn = QFileDialog::getOpenFileName(this, "Html file", QString(), "openlayers.html");

    if (!fn.isEmpty() )
    {
        ui->TileLineEdit->setText(fn);
    }
}

void SettingForm::on_OkPushButton_clicked()
{
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);

    settings.setValue("tilepath",  ui->TileLineEdit->text());
    settings.setValue("idlpath", ui->IdlLineEdit->text());
    settings.setValue("glyphmax", ui->glyphLineEdit->text());
    if(ui->privateVLKB_radioButton->isChecked())
        settings.setValue("vlkbtype", "private");
    else
        settings.setValue("vlkbtype", "public");


    settings.setValue("vlkbuser", ui->username_LineEdit->text());
    settings.setValue("vlkbpass", ui->password_LineEdit->text());
  //  settings.setValue("workdir",  ui->lineEdit_2->text());

    settings.setValue("online", ui->checkBox->isChecked());
    settings.setValue("onlinetilepath", ui->urlLineEdit->text());


    this->close();
    ViaLactea *vialactealWin = &Singleton<ViaLactea>::Instance();
    vialactealWin->reload();
}

void SettingForm::on_pushButton_clicked()
{
    this->close();
}

void SettingForm::on_checkBox_clicked(bool checked)
{
    qDebug()<<"checked: "<<checked;
}

void SettingForm::on_privateVLKB_radioButton_toggled(bool checked)
{
    if (checked)
    {

        ui->username_LineEdit->show();
        ui->password_LineEdit->show();
        ui->userLabel->show();
        ui->passLabel->show();


    }
    else
    {

        ui->username_LineEdit->hide();
        ui->password_LineEdit->hide();
        ui->userLabel->hide();
        ui->passLabel->hide();
    }
}

void SettingForm::on_workdirButton_clicked()
{
    QString fn =QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                  "~",
                                                  QFileDialog::ShowDirsOnly
                                                  | QFileDialog::DontResolveSymlinks);

    //QFileDialog::getOpenFileName(this, "Html file", QString(), "openlayers.html");

    if (!fn.isEmpty() )
    {
        ui->lineEdit_2->setText(fn);
    }
}

void SettingForm::on_checkBox_clicked()
{

}
