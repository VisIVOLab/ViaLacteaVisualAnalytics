#include "settingform.h"
#include "ui_settingform.h"

#include "caesarwindow.h"
#include "singleton.h"
#include "vialactea.h"
#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

SettingForm::SettingForm(QWidget *parent) : QWidget(parent, Qt::Window), ui(new Ui::SettingForm)
{
    ui->setupUi(this);
    ui->groupBox_4->hide();
    ui->vlkbLogoutButton->hide();
    ui->caesarLogoutButton->hide();
    ui->caesarEndpoint->setText(CaesarWindow::baseUrl);

    setAttribute(Qt::WA_DeleteOnClose);

    // this->setWindowFlags(Qt::WindowStaysOnTopHint);

    m_vlkbAuth = &NeaniasVlkbAuth::Instance();
    connect(m_vlkbAuth, &AuthWrapper::authenticated, this, &SettingForm::vlkb_loggedin);
    connect(m_vlkbAuth, &AuthWrapper::logged_out, this, &SettingForm::vlkb_loggedout);

    m_caesarAuth = &NeaniasCaesarAuth::Instance();
    connect(m_caesarAuth, &AuthWrapper::authenticated, this, &SettingForm::caesar_loggedin);
    connect(m_caesarAuth, &AuthWrapper::logged_out, this, &SettingForm::caesar_loggedout);

    m_sSettingsFile = QDir::homePath()
                              .append(QDir::separator())
                              .append("VisIVODesktopTemp")
                              .append("/setting.ini");

    readSettingsFromFile();
}

SettingForm::~SettingForm()
{
    delete ui;
}

void SettingForm::readSettingsFromFile()
{
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);

    m_termsAccepted = settings.value("termsaccepted", false).toBool();

    QString tilePath = settings.value("tilepath", "").toString();
    ui->TileLineEdit->setText(tilePath);

    QString glyphmax = settings.value("glyphmax", "2147483647").toString();
    ui->glyphLineEdit->setText(glyphmax);

    QString vlkbtype = settings.value("vlkbtype", "public").toString();
    if (vlkbtype == "public") {
        ui->publicVLKB_radioButton->setChecked(true);
        ui->vlkbUrl_lineEdit->setText(ViaLactea::VLKB_URL_PUBLIC);
        ui->tapUrl_lineEdit->setText(ViaLactea::TAP_URL_PUBLIC);

        ui->username_LineEdit->hide();
        ui->password_LineEdit->hide();
        ui->userLabel->hide();
        ui->passLabel->hide();
        ui->vlkbAuthLabel->hide();
        ui->vlkbAuthStatusLabel->hide();
        ui->vlkbLoginButton->hide();
    } else if (vlkbtype == "private") {
        ui->privateVLKB_radioButton->setChecked(true);
        ui->vlkbAuthLabel->hide();
        ui->vlkbAuthStatusLabel->hide();
        ui->vlkbLoginButton->hide();
        ui->username_LineEdit->show();
        ui->password_LineEdit->show();
        ui->userLabel->show();
        ui->passLabel->show();

        ui->username_LineEdit->setText(settings.value("vlkbuser", "").toString());
        ui->password_LineEdit->setText(settings.value("vlkbpass", "").toString());
    } else if (vlkbtype == "neanias") {
        ui->neaniasVLKB_radioButton->setChecked(true);
        ui->username_LineEdit->hide();
        ui->password_LineEdit->hide();
        ui->userLabel->hide();
        ui->passLabel->hide();

        ui->vlkbAuthLabel->show();

        if (m_vlkbAuth->isAuthenticated()) {
            vlkb_loggedin();
        } else {
            vlkb_loggedout();
        }

        ui->vlkbAuthStatusLabel->show();
    }

    if (settings.value("online", false) == true) {
        ui->checkBox->setChecked(true);
    }

    ui->urlLineEdit->setText(
            settings.value("onlinetilepath", ViaLactea::ONLINE_TILE_PATH).toString());

    if (m_caesarAuth->isAuthenticated()) {
        caesar_loggedin();
    } else {
        caesar_loggedout();
    }
}

void SettingForm::vlkb_loggedin()
{
    ui->vlkbAuthStatusLabel->setText("Authenticated");
    ui->vlkbLoginButton->hide();
    ui->vlkbLogoutButton->show();
}

void SettingForm::vlkb_loggedout()
{
    ui->vlkbAuthStatusLabel->setText("Not authenticated");
    ui->vlkbLoginButton->show();
    ui->vlkbLogoutButton->hide();
}

void SettingForm::caesar_loggedin()
{
    ui->caesarAuthStatusLabel->setText("Authenticated");
    ui->caesarLoginButton->hide();
    ui->caesarLogoutButton->show();
}

void SettingForm::caesar_loggedout()
{
    ui->caesarAuthStatusLabel->setText("Not authenticated");
    ui->caesarLoginButton->show();
    ui->caesarLogoutButton->hide();
}

void SettingForm::on_TilePushButton_clicked()
{
    QString fn = QFileDialog::getOpenFileName(this, "Html file", QString(), "openlayers.html");

    if (!fn.isEmpty()) {
        ui->TileLineEdit->setText(fn);
    }
}

void SettingForm::on_OkPushButton_clicked()
{
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);

    settings.setValue("termsaccepted", m_termsAccepted);
    settings.setValue("tilepath", ui->TileLineEdit->text());
    settings.setValue("glyphmax", ui->glyphLineEdit->text());
    if (ui->privateVLKB_radioButton->isChecked())
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
    qDebug() << "checked: " << checked;
}

void SettingForm::on_privateVLKB_radioButton_toggled(bool checked)
{
    if (checked) {
        ui->username_LineEdit->show();
        ui->password_LineEdit->show();
        ui->userLabel->show();
        ui->passLabel->show();
        ui->vlkbUrl_lineEdit->setText(ViaLactea::VLKB_URL_PRIVATE);
        ui->tapUrl_lineEdit->setText(ViaLactea::TAP_URL_PRIVATE);
    } else {
        ui->username_LineEdit->hide();
        ui->password_LineEdit->hide();
        ui->userLabel->hide();
        ui->passLabel->hide();
    }
}

void SettingForm::on_workdirButton_clicked()
{
    QString fn = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "~",
                                                   QFileDialog::ShowDirsOnly
                                                           | QFileDialog::DontResolveSymlinks);

    // QFileDialog::getOpenFileName(this, "Html file", QString(), "openlayers.html");

    if (!fn.isEmpty()) {
        ui->lineEdit_2->setText(fn);
    }
}

void SettingForm::on_neaniasVLKB_radioButton_toggled(bool checked)
{
    if (checked) {
        ui->vlkbAuthLabel->show();
        ui->vlkbAuthStatusLabel->show();
        ui->vlkbUrl_lineEdit->setText(ViaLactea::VLKB_URL_NEANIAS);
        ui->tapUrl_lineEdit->setText(ViaLactea::TAP_URL_NEANIAS);

        if (!m_vlkbAuth->isAuthenticated()) {
            vlkb_loggedout();
        } else {
            vlkb_loggedin();
        }

    } else {
        ui->vlkbAuthLabel->hide();
        ui->vlkbAuthStatusLabel->hide();
        ui->vlkbLoginButton->hide();
        ui->vlkbLogoutButton->hide();
    }
}

void SettingForm::on_vlkbLoginButton_clicked()
{
    if (m_vlkbAuth->isAuthenticated())
        return;

    if (!m_termsAccepted) {
        auto text = QString("To continue you must accept the <a href=\"%1\">privacy policy</a> and "
                            "the <a href=\"%2\">terms of use</a>.<br>Do you accept both?")
                            .arg(m_privacyPolicyUrl)
                            .arg(m_termsOfUseUrl);

        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setInformativeText(text);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes) {
            m_termsAccepted = true;
        } else {
            return;
        }
    }

    m_vlkbAuth->grant();
}

void SettingForm::on_publicVLKB_radioButton_toggled(bool checked)
{
    if (checked) {
        ui->vlkbUrl_lineEdit->setText(ViaLactea::VLKB_URL_PUBLIC);
        ui->tapUrl_lineEdit->setText(ViaLactea::TAP_URL_PUBLIC);
    }
}

void SettingForm::on_vlkbLogoutButton_clicked()
{
    if (m_vlkbAuth->isAuthenticated())
        m_vlkbAuth->logout();
}

void SettingForm::on_caesarLoginButton_clicked()
{
    if (m_caesarAuth->isAuthenticated())
        return;
    m_caesarAuth->grant();
}

void SettingForm::on_caesarLogoutButton_clicked()
{
    if (m_caesarAuth->isAuthenticated())
        m_caesarAuth->logout();
}
