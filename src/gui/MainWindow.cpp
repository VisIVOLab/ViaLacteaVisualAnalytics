#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "AboutDialog.h"
#include "AuthWrapper.h"
#include "app/BackendClient.h"
#include "DatasetOpenService.h"
#include "DatasetWindowFactory.h"
#include "RemoteFileBrowserDialog.h"
#include "Settings.h"
#include "SettingsDialog.h"
#include "WebViewProcess.h"
#include "vtkWindowCube.h"
#include "vtkWindowImage.h"

#include <QButtonGroup>
#include <QCloseEvent>
#include <QDebug>
#include <QDir>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QWebChannel>
#include <qstylehints.h>

using namespace Qt::StringLiterals;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qInfo().noquote() << qApp->applicationName() << qApp->applicationVersion();

    const QString workDir = QDir::home().absoluteFilePath(qApp->applicationName());
    qInfo() << "Working directory:" << workDir;

    // Create core objects
    this->settings = new Settings(workDir, this);
    this->auth = new AuthWrapper(this);
    this->datasetOpenService = std::make_unique<DatasetOpenService>();
    this->datasetWindowFactory = std::make_unique<DatasetWindowFactory>();
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
    bool ok = false;
    const QString source = QInputDialog::getItem(
            this, u"Open dataset"_s, u"Source"_s, { u"Local file"_s, u"Remote backend"_s }, 0,
            false, &ok);
    if (!ok || source.isEmpty()) {
        return;
    }

    if (source == u"Remote backend"_s) {
        this->openRemoteData();
        return;
    }

    const QString filepath = QFileDialog::getOpenFileName(
            this, u"Open local FITS file"_s, QString(), u"FITS files (*.fits *.fit)"_s);
    if (filepath.isEmpty()) {
        return;
    }

    const DatasetOpenInfo dataset = this->datasetOpenService->inspect(DatasetOpenRequest { filepath });
    if (!dataset.isValid()) {
        QMessageBox::critical(this, u"Could not open file"_s, dataset.errorMessage);
        return;
    }

    QWidget *win = this->datasetWindowFactory->createWindow(dataset, this);
    if (!win) {
        QMessageBox::critical(this, u"Could not open file"_s, u"Unknown file format."_s);
        return;
    }

    win->show();
    win->raise();
    win->activateWindow();
}

void MainWindow::openRemoteData()
{
    BackendClient client;
    const auto health = client.health();
    if (!health.ok) {
        QMessageBox::critical(this, u"Backend unavailable"_s,
                              health.error.isEmpty() ? u"Could not reach backend."_s : health.error);
        return;
    }

    RemoteFileBrowserDialog dialog(client.baseUrl(), this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString remotePath = dialog.selectedFilePath();
    if (remotePath.isEmpty()) {
        return;
    }

    const auto opened = client.openDataset(remotePath);
    if (!opened.valid) {
        QMessageBox::critical(this, u"Could not open remote dataset"_s,
                              opened.error.isEmpty() ? u"Backend rejected dataset open."_s
                                                     : opened.error);
        return;
    }

    if (opened.kind != u"cube"_s) {
        if (opened.kind == u"image"_s) {
            qDebug().noquote()
                    << QStringLiteral("[fits] active_axes=%1 -> opening as 2D image | %2")
                               .arg(opened.activeAxes)
                               .arg(opened.degenerateAxesSummary.isEmpty()
                                            ? QStringLiteral("no collapsed axes")
                                            : opened.degenerateAxesSummary);
            auto *win = new vtkWindowImage(remotePath, client.baseUrl(), opened.datasetId,
                                           opened.ctype, opened.cunit, opened.crval, opened.crpix,
                                           opened.cdelt, opened.degenerateAxesSummary, this);
            win->show();
            win->raise();
            win->activateWindow();
            return;
        }

        QMessageBox::information(this, u"Remote dataset"_s, u"Unsupported remote dataset kind."_s);
        return;
    }

    qDebug().noquote()
            << QStringLiteral("[fits] active_axes=%1 -> opening as cube | %2")
                       .arg(opened.activeAxes)
                       .arg(opened.degenerateAxesSummary.isEmpty()
                                    ? QStringLiteral("no collapsed axes")
                                    : opened.degenerateAxesSummary);
    auto *win = new vtkWindowCube(remotePath, client.baseUrl(), opened.datasetId, opened.width,
                                  opened.height, opened.depth, opened.spacing, opened.origin,
                                  opened.ctype, opened.cunit, opened.crval, opened.crpix,
                                  opened.cdelt, opened.degenerateAxesSummary, this);
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
