#include "startupwindow.h"
#include "ui_startupwindow.h"

#include "astroutils.h"
#include "recentfilesmanager.h"
#include "vtkfitsreader.h"
#include "vtkwindow_new.h"
#include "vtkwindowcube.h"
#include "fitsheadermodifierdialog.h"
#include "settingform.h"
#include "vialactea.h"
#include "fitsheaderviewer.h"

#include <QFileDialog>
#include <QMessageBox>

StartupWindow::StartupWindow(QWidget *parent) : QWidget(parent), ui(new Ui::StartupWindow),
    settingsFile(QDir::homePath().append("/VisIVODesktopTemp/setting.ini")),
    settings(settingsFile, QSettings::IniFormat)
{
    ui->setupUi(this);

    QPalette pal = QPalette();
    pal.setColor(QPalette::Window, QColor::fromRgb(234, 233, 229));
    ui->leftContainer->setAutoFillBackground(true);
    ui->leftContainer->setPalette(pal);

    QPalette pal2 = QPalette();
    pal2.setColor(QPalette::Window, Qt::white);

    ui->buttonArea->setAutoFillBackground(true);
    ui->buttonArea->setPalette(pal2);

    // Setup History Manager
    this->historyModel = new RecentFilesManager(this);
    ui->historyArea->setModel(this->historyModel);
}

StartupWindow::~StartupWindow()
{
    delete ui;
}

void StartupWindow::on_localOpenPushButton_clicked(bool fromHistory=false)
{
    QString fn;
    if (!fromHistory)
        fn = QFileDialog::getOpenFileName(this, tr("Import an image file"), QString(),
                                          tr("FITS images (*.fit *.fits)"));
    else{
        QModelIndexList selectedIndexes = ui->historyArea->selectionModel()->selectedIndexes();
        if (!selectedIndexes.isEmpty()) {
            QModelIndex selectedIndex = selectedIndexes.at(0);
            fn = selectedIndex.data(Qt::DisplayRole).toString();
        }
    }

    if (fn.isEmpty()) {
        return;
    }

    if (AstroUtils::isFitsImage(fn.toStdString())) {
        //    openLocalImage(fn);
    } else {
        openLocalDC(fn);
    }
}

void StartupWindow::on_clearPushButton_clicked()
{
    this->historyModel->clearHistory();
}

void StartupWindow::openLocalDC(const QString &fn)
{
    // Check if the fits is a simcube
    auto fitsReader_dc = vtkSmartPointer<vtkFitsReader>::New();
    fitsReader_dc->SetFileName(fn.toStdString());
    if (fitsReader_dc->ctypeXY) {
        // Check if the keywords in the header are present
        std::list<std::string> missing;
        try {
            if (!AstroUtils::checkSimCubeHeader(fn.toStdString(), missing)) {
                auto res = QMessageBox::warning(this, "Warning",
                                                "The header does not contain all the necessary "
                                                "keywords!\nDo you want to add them?",
                                                QMessageBox::Yes | QMessageBox::No);

                if (res == QMessageBox::Yes) {
                    QStringList qMissing;
                    std::for_each(missing.cbegin(), missing.cend(), [&](const std::string &key) {
                        qMissing << QString::fromStdString(key);
                    });
                    auto dialog = new FitsHeaderModifierDialog(fn, qMissing);
                    dialog->show();
                    return;
                }
            }
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error", QString::fromUtf8(e.what()));
            return;
        }
        new vtkwindow_new(this, fitsReader_dc, 1, nullptr);
        this->historyModel->addRecentFile(fn);
        return;
    }

    long maxSize = settings.value("downscaleSize", 1024).toInt() * 1024; // MB->KB
    long size = QFileInfo(fn).size() / 1024; // B -> KB
    int ScaleFactor = 1; // AstroUtils::calculateResizeFactor(size, maxSize);

    vtkWindowCube *win = new vtkWindowCube(nullptr, fn, ScaleFactor);
    win->show();
    win->activateWindow();
    win->raise();

    // Aggiungi alcuni file recenti di esempio al gestore dei file recenti
    this->historyModel->addRecentFile(fn);
}


void StartupWindow::on_settingsPushButton_clicked()
{
    if (!settingForm) {
        settingForm = new SettingForm(this);
    }
    settingForm->show();
    settingForm->activateWindow();
    settingForm->raise();
}


void StartupWindow::on_openPushButton_clicked()
{
    on_localOpenPushButton_clicked(true);
}


void StartupWindow::on_vlkbPushButton_clicked()
{
    ViaLactea *vialactealWin = new ViaLactea();
    vialactealWin->show();
}


void StartupWindow::on_infoPushButton_clicked()
{
    QString fn;
    QModelIndexList selectedIndexes = ui->historyArea->selectionModel()->selectedIndexes();
    if (!selectedIndexes.isEmpty()) {
        QModelIndex selectedIndex = selectedIndexes.at(0);
        fn = selectedIndex.data(Qt::DisplayRole).toString();
    }

    if (fn.isEmpty()) {
        return;
    }
    FitsHeaderViewer *fitsHeaderViewer = new FitsHeaderViewer(fn);
    fitsHeaderViewer->show();
}

