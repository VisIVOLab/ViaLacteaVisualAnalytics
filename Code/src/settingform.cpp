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

    QString pythonpath = settings.value("python.path").toString();
    if (pythonpath.isEmpty()) {
        pythonpath = "python3";
    }
    ui->textPython->setText(pythonpath);

    bool useBundlePython = settings.value("python.bundle", true).toBool();
    ui->checkBundlePython->setChecked(useBundlePython);

    QString tilePath = settings.value("tilepath", "").toString();
    ui->TileLineEdit->setText(tilePath);

    QString glyphmax = settings.value("glyphmax", "2147483647").toString();
    ui->glyphLineEdit->setText(glyphmax);

    int maxSize = settings.value("downscaleSize", 1024).toInt();
    ui->textDownscaleSize->setText(QString::number(maxSize));

    QString vlkbtype = settings.value("vlkbtype", "ia2").toString();
    QString vlkburl = settings.value("vlkburl", ViaLactea::VLKB_URL_IA2).toString();
    QString vlkbtableurl = settings.value("vlkbtableurl", ViaLactea::TAP_URL_IA2).toString();
    ui->vlkbUrl_lineEdit->setText(vlkburl);
    ui->tapUrl_lineEdit->setText(vlkbtableurl);

    bool searchOnImport = settings.value("vlkb.search", false).toBool();
    ui->checkSearchOnImport->setChecked(searchOnImport);

    if (m_ia2VlkbAuth->isAuthenticated()) {
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
    settings.setValue("tilepath", ui->TileLineEdit->text());
    settings.setValue("python.path", ui->textPython->text());
    settings.setValue("python.bundle", ui->checkBundlePython->isChecked());
    settings.setValue("glyphmax", ui->glyphLineEdit->text());
    settings.setValue("downscaleSize", ui->textDownscaleSize->text().toInt());
    settings.setValue("vlkbtype", "ia2");
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

void SettingForm::on_workdirButton_clicked()
{
    QString fn = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "~",
                                                   QFileDialog::ShowDirsOnly
                                                           | QFileDialog::DontResolveSymlinks);
    if (!fn.isEmpty()) {
        ui->lineEdit_2->setText(fn);
    }
}

void SettingForm::on_vlkbLoginButton_clicked()
{
    if (m_ia2VlkbAuth->isAuthenticated())
        return;

    m_ia2VlkbAuth->grant();
}

void SettingForm::on_vlkbLogoutButton_clicked()
{
    if (m_ia2VlkbAuth->isAuthenticated())
        m_ia2VlkbAuth->logout();
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

void SettingForm::on_checkBundlePython_toggled(bool checked)
{
    ui->textPython->setEnabled(!checked);
    ui->btnPython->setEnabled(!checked);
}

void SettingForm::on_btnPython_clicked()
{
    QString fn = QFileDialog::getOpenFileName(this, "Python3 Binary", QString(), "python3");
    if (!fn.isEmpty()) {
        ui->textPython->setText(fn);
    }
}
