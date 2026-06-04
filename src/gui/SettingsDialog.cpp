#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"

#include "Settings.h"

#include <QFileDialog>
#include <QIntValidator>

#include <limits>

using namespace Qt::StringLiterals;

SettingsDialog::SettingsDialog(Settings *settings, QWidget *parent)
    : QDialog(parent), ui(new Ui::SettingsDialog), settings(settings)
{
    ui->setupUi(this);

    // Reload just in case the file has been modified by another program
    this->settings->reload();

    // UI
    ui->comboTheme->setCurrentIndex(static_cast<int>(this->settings->getColorScheme()));

    // Python
    ui->linePythonPath->setText(this->settings->getPythonLocation());
    ui->btnPython->setIcon(QIcon(u":/icons/FILE_OPEN.png"_s));
    QObject::connect(ui->btnPython, &QToolButton::clicked, this,
                     &SettingsDialog::selectPythonLocation);

    // Panoramic View
    ui->linePanoramicView->setText(this->settings->getPanoramicView());
    ui->btnPanoramicView->setIcon(QIcon(u":/icons/FILE_OPEN.png"_s));
    QObject::connect(ui->btnPanoramicView, &QToolButton::clicked, this,
                     &SettingsDialog::selectPanoramicView);

    // Visualization group
    ui->lineMaxGlyphs->setText(QString::number(this->settings->getMaxGlyphs()));
    ui->lineMaxGlyphs->setValidator(
            new QIntValidator(0, std::numeric_limits<int>::max(), ui->lineMaxGlyphs));

    // VLKB group
    ui->lineVLKBUrl->setText(this->settings->getVLKBUrl());
    this->setVLKBAuthStatus();
    ui->checkVLKBSearch->setChecked(this->settings->getSearchOnImportFlag());
    QObject::connect(ui->btnVLKBAuth, &QPushButton::clicked, this,
                     &SettingsDialog::VLKBAuthTriggered);

    // Restore defaults
    QObject::connect(ui->buttonBox, &QDialogButtonBox::clicked, this,
                     [this](QAbstractButton *button) {
                         if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ResetRole) {
                             this->restoreDefaults();
                         }
                     });

    // Update labels when user's authentication status changes
    // QObject::connect(this->auth, &AuthWrapper::granted, this, &SettingsDialog::updateAuthStatus);
    // QObject::connect(this->auth, &AuthWrapper::loggedOut, this, &SettingsDialog::updateAuthStatus);

    // Signal-to-signal connection to notify other classes they need to reload
    // their settings
    QObject::connect(this, &SettingsDialog::accepted, this->settings, &Settings::updated);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::accept()
{
    this->settings->setColorScheme(static_cast<Qt::ColorScheme>(ui->comboTheme->currentIndex()));
    this->settings->setPanoramicView(ui->linePanoramicView->text());
    this->settings->setMaxGlyphs(ui->lineMaxGlyphs->text().toInt());
    this->settings->setVLKBUrl(ui->lineVLKBUrl->text());
    this->settings->setSearchOnImportFlag(ui->checkVLKBSearch->isChecked());
    QDialog::accept();
}

void SettingsDialog::restoreDefaults()
{
    this->settings->resetDefaults();
    QDialog::accept();
}

void SettingsDialog::selectPythonLocation()
{
    const QString filepath =
            QFileDialog::getOpenFileName(this, u"Select Python interpreter"_s, QDir::homePath(),
                                         u"python3 executable (python3 python)"_s);
    if (!filepath.isEmpty()) {
        ui->linePythonPath->setText(filepath);
    }
}

void SettingsDialog::selectPanoramicView()
{
    const QString filepath = QFileDialog::getOpenFileName(
            this, u"Select Panoramic View"_s, QDir::homePath(), u"Panoramic View index (*.html)"_s);
    if (!filepath.isEmpty()) {
        ui->linePanoramicView->setText(QUrl::fromUserInput(filepath).toString());
    }
}

// void SettingsDialog::updateAuthStatus(AuthService service)
// {
//      if (service == AuthService::VLKB) {
//          this->setVLKBAuthStatus();
//      }
// }

void SettingsDialog::VLKBAuthTriggered()
{
    // if (this->auth->hasTokens(AuthService::VLKB)) {
    //     this->auth->logout(AuthService::VLKB);
    // } else {
    //     this->auth->grant(AuthService::VLKB);
    // }
}

void SettingsDialog::setVLKBAuthStatus()
{
    // if (this->auth->hasTokens(AuthService::VLKB)) {
    //     ui->labelVLKBAuth->setText(u"Authenticated"_s);
    //     ui->btnVLKBAuth->setText(u"Sign out"_s);
    // } else {
    //     ui->labelVLKBAuth->setText(u"Not authenticated"_s);
    //     ui->btnVLKBAuth->setText(u"Sign in"_s);
    // }
}
