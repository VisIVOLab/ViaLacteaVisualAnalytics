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
    ui->groupBox_4->hide();

    // this->setWindowFlags(Qt::WindowStaysOnTopHint);

    m_authWrapper = &Singleton<AuthWrapper>::Instance();
    connect(m_authWrapper, &AuthWrapper::authenticated, [&](){
        ui->authStatusLabel->setText("Authenticated");
        ui->loginButton->hide();
        ui->logoutButton->show();
    });
    connect(m_authWrapper, &AuthWrapper::logged_out, [&](){
        ui->authStatusLabel->setText("Not authenticated");
        ui->loginButton->show();
        ui->logoutButton->hide();
    });

    m_sSettingsFile = QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp").append("/setting.ini");
}

SettingForm::~SettingForm()
{
    delete ui;
}

void SettingForm::readSettingsFromFile()
{
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);

    QString tilePath = settings.value("tilepath", "").toString();
    ui->TileLineEdit->setText(tilePath);

    QString idlPath = settings.value("idlpath", "").toString();
    ui->IdlLineEdit->setText(idlPath);

    QString glyphmax = settings.value("glyphmax", "2147483647").toString();
    ui->glyphLineEdit->setText(glyphmax);

    QString vlkbtype = settings.value("vlkbtype", "public").toString();
    if(vlkbtype == "public")
    {
        ui->publicVLKB_radioButton->setChecked(true);
        ui->vlkbUrl_lineEdit->setText(ViaLactea::VLKB_URL_PUBLIC);
        ui->tapUrl_lineEdit->setText(ViaLactea::TAP_URL_PUBLIC);

        ui->username_LineEdit->hide();
        ui->password_LineEdit->hide();
        ui->userLabel->hide();
        ui->passLabel->hide();
        ui->authLabel->hide();
        ui->authStatusLabel->hide();
        ui->loginButton->hide();
    }
    else if(vlkbtype == "private")
    {
        ui->privateVLKB_radioButton->setChecked(true);
        ui->authLabel->hide();
        ui->authStatusLabel->hide();
        ui->loginButton->hide();
        ui->username_LineEdit->show();
        ui->password_LineEdit->show();
        ui->userLabel->show();
        ui->passLabel->show();

        ui->username_LineEdit->setText(settings.value("vlkbuser", "").toString());
        ui->password_LineEdit->setText(settings.value("vlkbpass", "").toString());
    }
    else if (vlkbtype == "neanias")
    {
        ui->neaniasVLKB_radioButton->setChecked(true);
        ui->username_LineEdit->hide();
        ui->password_LineEdit->hide();
        ui->userLabel->hide();
        ui->passLabel->hide();

        ui->authLabel->show();

        if(m_authWrapper->isAuthenticated()){
            ui->loginButton->hide();
            ui->logoutButton->show();
            ui->authStatusLabel->setText("Authenticated");
        } else {
            ui->loginButton->show();
            ui->logoutButton->hide();
            ui->authStatusLabel->setText("Not authenticated");
        }

        ui->authStatusLabel->show();
    }

    if (settings.value("online",false) == true)
    {
        ui->checkBox->setChecked(true);

    }

    ui->urlLineEdit->setText(settings.value("onlinetilepath", ViaLactea::ONLINE_TILE_PATH).toString());
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
    else if (ui->publicVLKB_radioButton->isChecked())
        settings.setValue("vlkbtype", "public");
    else
        settings.setValue("vlkbtype", "neanias");


    settings.setValue("vlkbuser", ui->username_LineEdit->text());
    settings.setValue("vlkbpass", ui->password_LineEdit->text());
  //  settings.setValue("workdir",  ui->lineEdit_2->text());

    settings.setValue("online", ui->checkBox->isChecked());
    settings.setValue("onlinetilepath", ui->urlLineEdit->text());

    settings.sync();

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
        ui->vlkbUrl_lineEdit->setText(ViaLactea::VLKB_URL_PRIVATE);
        ui->tapUrl_lineEdit->setText(ViaLactea::TAP_URL_PRIVATE);
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

void SettingForm::on_neaniasVLKB_radioButton_toggled(bool checked)
{
    if (checked) {
        ui->authLabel->show();
        ui->authStatusLabel->show();
        ui->vlkbUrl_lineEdit->setText(ViaLactea::VLKB_URL_NEANIAS);
        ui->tapUrl_lineEdit->setText(ViaLactea::TAP_URL_NEANIAS);

        if (!m_authWrapper->isAuthenticated()) {
            ui->authStatusLabel->setText("Not authenticated");
            ui->loginButton->show();
            ui->logoutButton->hide();
        }
        else {
            ui->authStatusLabel->setText("Authenticated");
            ui->loginButton->hide();
            ui->logoutButton->show();
        }

    } else {
        ui->authLabel->hide();
        ui->authStatusLabel->hide();
        ui->loginButton->hide();
        ui->logoutButton->hide();
    }
}

void SettingForm::on_loginButton_clicked()
{
    if(!m_authWrapper->isAuthenticated())
        m_authWrapper->grant();
}

void SettingForm::on_publicVLKB_radioButton_toggled(bool checked)
{
    if (checked) {
        ui->vlkbUrl_lineEdit->setText(ViaLactea::VLKB_URL_PUBLIC);
        ui->tapUrl_lineEdit->setText(ViaLactea::TAP_URL_PUBLIC);
    }
}

void SettingForm::on_logoutButton_clicked()
{
    if (m_authWrapper->isAuthenticated())
        m_authWrapper->logout();
}
