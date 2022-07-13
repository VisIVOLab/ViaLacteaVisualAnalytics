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
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);
    ui->groupBox_4->hide();
    ui->vlkbLogoutButton->hide();
    ui->caesarLogoutButton->hide();
    ui->caesarEndpoint->setText(CaesarWindow::baseUrl);

    m_ia2VlkbAuth = &IA2VlkbAuth::Instance();
    connect(m_ia2VlkbAuth, &AuthWrapper::authenticated, this, &SettingForm::vlkb_loggedin);
    connect(m_ia2VlkbAuth, &AuthWrapper::logged_out, this, &SettingForm::vlkb_loggedout);

    m_neaniasVlkbAuth = &NeaniasVlkbAuth::Instance();
    connect(m_neaniasVlkbAuth, &AuthWrapper::authenticated, this, &SettingForm::vlkb_loggedin);
    connect(m_neaniasVlkbAuth, &AuthWrapper::logged_out, this, &SettingForm::vlkb_loggedout);

    m_caesarAuth = &NeaniasCaesarAuth::Instance();
    connect(m_caesarAuth, &AuthWrapper::authenticated, this, &SettingForm::caesar_loggedin);
    connect(m_caesarAuth, &AuthWrapper::logged_out, this, &SettingForm::caesar_loggedout);

    m_settingsFile = QDir::homePath()
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
    QSettings settings(m_settingsFile, QSettings::NativeFormat);

    m_termsAccepted = settings.value("termsaccepted", false).toBool();

    QString tilePath = settings.value("tilepath", "").toString();
    ui->TileLineEdit->setText(tilePath);

    QString glyphmax = settings.value("glyphmax", "2147483647").toString();
    ui->glyphLineEdit->setText(glyphmax);

    QString vlkbtype = settings.value("vlkbtype", "ia2").toString();
    AuthWrapper *vlkbAuth;
    if (vlkbtype == "ia2") {
        ui->ia2VLKB_radioButton->setChecked(true);
        ui->vlkbUrl_lineEdit->setText(ViaLactea::VLKB_URL_IA2);
        ui->tapUrl_lineEdit->setText(ViaLactea::TAP_URL_IA2);
        vlkbAuth = m_ia2VlkbAuth;
    } else /* (vlkbtype == "neanias") */ {
        ui->neaniasVLKB_radioButton->setChecked(true);
        ui->vlkbUrl_lineEdit->setText(ViaLactea::VLKB_URL_NEANIAS);
        ui->tapUrl_lineEdit->setText(ViaLactea::TAP_URL_NEANIAS);
        vlkbAuth = m_neaniasVlkbAuth;
    }

    bool searchOnImport = settings.value("vlkb.search", false).toBool();
    ui->checkSearchOnImport->setChecked(searchOnImport);

    if (vlkbAuth->isAuthenticated()) {
        vlkb_loggedin();
    } else {
        vlkb_loggedout();
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
    this->activateWindow();
    this->raise();
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
    this->activateWindow();
    this->raise();
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
    QSettings settings(m_settingsFile, QSettings::NativeFormat);

    settings.setValue("termsaccepted", m_termsAccepted);
    settings.setValue("tilepath", ui->TileLineEdit->text());
    settings.setValue("glyphmax", ui->glyphLineEdit->text());
    if (ui->ia2VLKB_radioButton->isChecked())
        settings.setValue("vlkbtype", "ia2");
    else
        settings.setValue("vlkbtype", "neanias");

    //  settings.setValue("workdir",  ui->lineEdit_2->text());

    settings.setValue("vlkb.search", ui->checkSearchOnImport->isChecked());
    settings.setValue("vlkburl", ui->vlkbUrl_lineEdit->text());
    settings.setValue("vlkbtableurl", ui->tapUrl_lineEdit->text());
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

void SettingForm::on_ia2VLKB_radioButton_toggled(bool checked)
{
    if (checked) {
        ui->vlkbUrl_lineEdit->setText(ViaLactea::VLKB_URL_IA2);
        ui->tapUrl_lineEdit->setText(ViaLactea::TAP_URL_IA2);

        if (!m_ia2VlkbAuth->isAuthenticated()) {
            vlkb_loggedout();
        } else {
            vlkb_loggedin();
        }
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
        ui->vlkbUrl_lineEdit->setText(ViaLactea::VLKB_URL_NEANIAS);
        ui->tapUrl_lineEdit->setText(ViaLactea::TAP_URL_NEANIAS);

        if (!m_neaniasVlkbAuth->isAuthenticated()) {
            vlkb_loggedout();
        } else {
            vlkb_loggedin();
        }
    }
}

void SettingForm::on_vlkbLoginButton_clicked()
{
    auto vlkbAuth = (ui->ia2VLKB_radioButton->isChecked() ? m_ia2VlkbAuth : m_neaniasVlkbAuth);

    if (vlkbAuth->isAuthenticated())
        return;

    if (vlkbAuth == m_neaniasVlkbAuth && !m_termsAccepted) {
        auto text = QString("To continue you must accept the <a href=\"%1\">privacy policy</a> and "
                            "the <a href=\"%2\">terms of use</a>.<br>Do you accept both?")
                            .arg(m_privacyPolicyUrl, m_termsOfUseUrl);

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

    vlkbAuth->grant();
}

void SettingForm::on_vlkbLogoutButton_clicked()
{
    auto vlkbAuth = (ui->ia2VLKB_radioButton->isChecked() ? m_ia2VlkbAuth : m_neaniasVlkbAuth);
    if (vlkbAuth->isAuthenticated())
        vlkbAuth->logout();
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
