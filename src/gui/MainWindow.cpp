#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "AboutDialog.h"
#include "AstroUtils.h"
#include "AuthWrapper.h"
#include "Logging.h"
#include "Settings.h"
#include "SettingsDialog.h"
#include "WebViewProcess.h"
#include "vtkWindowCube.h"
#include "vtkWindowImage.h"

#include <QButtonGroup>
#include <QCloseEvent>
#include <QDebug>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QMessageBox>
#include <QStyleHints>
#include <QWebChannel>

using namespace Qt::StringLiterals;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qCInfo(logApp).noquote() << qApp->applicationName() << qApp->applicationVersion();
    qCInfo(logApp) << "Session directory:" << qApp->property("sessionFolder").toString();

    // Create core objects
    this->settings = new Settings(QDir::home().absoluteFilePath(qApp->applicationName()), this);
    this->auth = new AuthWrapper(this);
    QObject::connect(this->settings, &Settings::updated, this, &MainWindow::setApplicationTheme);
    QObject::connect(this->settings, &Settings::updated, this, &MainWindow::loadPanoramicView);

    // Setup Menu File
    QObject::connect(ui->actionOpenFile, &QAction::triggered, this, &MainWindow::openLocalData);
    QObject::connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);

    // Setup Menu Edit
    QObject::connect(ui->actionSettings, &QAction::triggered, this,
                     &MainWindow::openSettingsDialog);

    // Setup Menu About
    QObject::connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::openAboutDialog);

    // Setup Panoramic View
    this->loadPanoramicView();
    auto webobj = new WebViewProcess(ui->view);
    auto webChannel = new QWebChannel(ui->view);
    webChannel->registerObject(u"webobj"_s, webobj);
    ui->view->page()->setWebChannel(webChannel);
    QObject::connect(webobj, &WebViewProcess::processJavascript, this,
                     &MainWindow::skyRegionSelected);

    // Setup Loading
    ui->btnLoadData->setIcon(QIcon(u":/icons/FILE_OPEN.png"_s));
    QObject::connect(ui->btnLoadData, &QPushButton::clicked, this, &MainWindow::openLocalData);

    // Setup selection modes
    auto groupSelection = new QButtonGroup(this);
    groupSelection->addButton(ui->radioSelNone);
    groupSelection->addButton(ui->radioSelPoint);
    groupSelection->addButton(ui->radioSelRect);
    QObject::connect(groupSelection, &QButtonGroup::buttonToggled, this,
                     &MainWindow::changeViewSelectionMode);

    // Setup Line Edits
    ui->lineLon->setValidator(new QDoubleValidator(ui->lineLon));
    ui->lineLat->setValidator(new QDoubleValidator(ui->lineLat));
    ui->lineRadius->setValidator(new QDoubleValidator(ui->lineRadius));
    ui->lineDLon->setValidator(new QDoubleValidator(ui->lineDLon));
    ui->lineDLat->setValidator(new QDoubleValidator(ui->lineDLat));

    // Load theme
    this->setApplicationTheme();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    auto res = QMessageBox::question(this, u"Exit"_s, u"Do you  want to exit?"_s,
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (res == QMessageBox::No) {
        event->ignore();
    }
}

void MainWindow::setApplicationTheme()
{
    QGuiApplication::styleHints()->setColorScheme(this->settings->getColorScheme());
}

void MainWindow::openLocalData()
{
    const QString filepath = QFileDialog::getOpenFileName(
            this, u"Open local FITS file"_s, QString(), u"FITS files (*.fits *.fit)"_s);
    if (filepath.isEmpty()) {
        return;
    }

    const AstroUtils astro(filepath.toStdString());
    QWidget *win;
    if (astro.isImage()) {
        // Image
        win = new vtkWindowImage(filepath, this);
    } else if (astro.isCube()) {
        // Cube
        win = new vtkWindowCube(filepath, this);
    } else {
        // Unknown
        QMessageBox::critical(this, u"Could not open file"_s, u"Unknown file format."_s);
        return;
    }

    win->show();
    win->raise();
    win->activateWindow();
}

void MainWindow::openSettingsDialog()
{
    SettingsDialog dialog(this->settings, this->auth, this);
    dialog.exec();
}

void MainWindow::openAboutDialog()
{
    AboutDialog about(this);
    about.exec();
}

void MainWindow::loadPanoramicView()
{
    ui->view->load(QUrl::fromUserInput(this->settings->getPanoramicView()));
}

void MainWindow::changeViewSelectionMode()
{
    if (ui->radioSelNone->isChecked()) {
        ui->view->page()->runJavaScript(u"activatePointSelection(false)"_s);
        ui->view->page()->runJavaScript(u"activateRectangularSelection(false)"_s);
        return;
    }

    if (ui->radioSelPoint->isChecked()) {
        ui->view->page()->runJavaScript(u"activatePointSelection(true)"_s);
        return;
    }

    // ui->radioSelRect->isChecked
    ui->view->page()->runJavaScript(u"activateRectangularSelection(true)"_s);
}

void MainWindow::skyRegionSelected(const QString &point, const QString &area)
{
    ui->radioSelNone->setChecked(true);

    // Get coords
    const QStringList coordsStr = point.split(',');
    const double coords[2] = { coordsStr[0].simplified().toDouble(),
                               coordsStr[1].simplified().toDouble() };
    ui->lineLon->setText(QString::number(coords[0]));
    ui->lineLat->setText(QString::number(coords[1]));

    // Get Area
    if (area.isEmpty()) {
        // Point
        if (ui->lineRadius->text().isEmpty()) {
            ui->lineRadius->setText(QString::number(0.1));
        }
        ui->lineDLon->clear();
        ui->lineDLat->clear();
    } else {
        // Rect
        const QStringList areaStr = area.split(',');
        const double rect[2] = { areaStr[0].simplified().toDouble(),
                                 areaStr[1].simplified().toDouble() };
        ui->lineRadius->clear();
        ui->lineDLon->setText(QString::number(rect[0]));
        ui->lineDLat->setText(QString::number(rect[1]));
    }
}
