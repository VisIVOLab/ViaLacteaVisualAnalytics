#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "AboutDialog.h"
#include "AstroUtils.h"
#include "AuthWrapper.h"
#include "FitsHeaderWidget.h"
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
                     &MainWindow::showSettingsDialog);

    // Setup Menu About
    QObject::connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);

    // Setup Panoramic View
    this->loadPanoramicView();
    auto *webobj = new WebViewProcess(ui->view);
    auto *webChannel = new QWebChannel(ui->view);
    webChannel->registerObject(u"webobj"_s, webobj);
    ui->view->page()->setWebChannel(webChannel);
    QObject::connect(webobj, &WebViewProcess::processJavascript, this,
                     &MainWindow::skyRegionSelected);

    // Setup Loading
    ui->btnLoadData->setIcon(QIcon(u":/icons/FILE_OPEN.png"_s));
    QObject::connect(ui->btnLoadData, &QPushButton::clicked, this, &MainWindow::openLocalData);

    // Setup selection modes
    auto *groupSelection = new QButtonGroup(this);
    groupSelection->addButton(ui->radioSelNone, static_cast<int>(ViewSelectionMode::None));
    groupSelection->addButton(ui->radioSelPoint, static_cast<int>(ViewSelectionMode::Point));
    groupSelection->addButton(ui->radioSelRect, static_cast<int>(ViewSelectionMode::Rectangle));
    QObject::connect(groupSelection, &QButtonGroup::idToggled, this,
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
    auto res = QMessageBox::question(this, u"Exit"_s, u"Do you want to exit?"_s,
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (res == QMessageBox::No) {
        event->ignore();
        return;
    }

    event->accept();
}

void MainWindow::setApplicationTheme()
{
    QGuiApplication::styleHints()->setColorScheme(this->settings->getColorScheme());
}

void MainWindow::openLocalData()
{
    QFileDialog dialog(this, u"Open local FITS file"_s, QString(), u"FITS files (*.fits *.fit)"_s);
    dialog.setOptions(QFileDialog::DontUseNativeDialog | QFileDialog::ReadOnly);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setMinimumSize(800, 500);

    auto *headerWidget = new FitsHeaderWidget(&dialog);
    QObject::connect(&dialog, &QFileDialog::currentChanged, headerWidget,
                     &FitsHeaderWidget::showHeader);

    auto *layout = qobject_cast<QGridLayout *>(dialog.layout());
    layout->addWidget(headerWidget, 1, layout->columnCount());

    if (!dialog.exec()) {
        return;
    }

    const QString filepath = dialog.selectedFiles().first();
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
        qCCritical(logApp) << "Could not open file" << filepath << "Unknown file format!";
        return;
    }

    win->show();
    win->raise();
    win->activateWindow();
}

void MainWindow::showSettingsDialog()
{
    SettingsDialog dialog(this->settings, this->auth, this);
    dialog.exec();
}

void MainWindow::showAboutDialog()
{
    AboutDialog about(this);
    about.exec();
}

void MainWindow::loadPanoramicView()
{
    ui->view->load(QUrl::fromUserInput(this->settings->getPanoramicView()));
}

void MainWindow::changeViewSelectionMode(int id)
{
    switch (static_cast<ViewSelectionMode>(id)) {
    case ViewSelectionMode::None:
        ui->view->page()->runJavaScript(WebViewProcess::ActivatePointSelection.arg(false));
        ui->view->page()->runJavaScript(WebViewProcess::ActivateRectangularSelection.arg(false));
        break;

    case ViewSelectionMode::Point:
        ui->view->page()->runJavaScript(WebViewProcess::ActivatePointSelection.arg(true));
        break;

    case ViewSelectionMode::Rectangle:
        ui->view->page()->runJavaScript(WebViewProcess::ActivateRectangularSelection.arg(true));
        break;
    }
}

void MainWindow::skyRegionSelected(const QString &point, const QString &area)
{
    ui->radioSelNone->setChecked(true);

    if (point.isEmpty()) {
        // User clicked outside the map, do nothing
        return;
    }

    // Get coords
    const QStringList coordsStr = point.split(',');
    const double coords[2] = { coordsStr[0].simplified().toDouble(),
                               coordsStr[1].simplified().toDouble() };
    ui->lineLon->setText(QString::number(coords[0]));
    ui->lineLat->setText(QString::number(coords[1]));

    if (area.isEmpty()) {
        // Point
        ui->lineRadius->setText(QString::number(0.1));
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
