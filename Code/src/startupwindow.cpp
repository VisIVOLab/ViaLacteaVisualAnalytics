#include "startupwindow.h"
#include "ui_startupwindow.h"
#include "recentfilesmanager.h"
#include <QFileDialog>
#include "astroutils.h"
#include "vtkfitsreader.h"
#include <QMessageBox>
#include "vtkwindow_new.h"
#include "vtkWindowCube.h"

StartupWindow::StartupWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StartupWindow)
{
    ui->setupUi(this);

    QPalette pal = QPalette();
    pal.setColor(QPalette::Window, QColor::fromRgb(234,233,229));
    ui->leftContainer->setAutoFillBackground(true);
    ui->leftContainer->setPalette(pal);

    QPalette pal2 = QPalette();
    pal2.setColor(QPalette::Window, Qt::white);

    ui->logoArea->setAutoFillBackground(true);
    ui->logoArea->setPalette(pal2);

    //    ui->historyArea->setAutoFillBackground(true);
    //   ui->historyArea->setPalette(pal2);

    ui->buttonArea->setAutoFillBackground(true);
    ui->buttonArea->setPalette(pal2);

    PopulateHistory();

}

StartupWindow::~StartupWindow()
{
    delete ui;
}


void StartupWindow::PopulateHistory()
{
    RecentFilesManager *recentFilesManager = new RecentFilesManager(this);
    QStringList recentFiles = recentFilesManager->recentFiles();

    // Pulisci la lista widget
    ui->historyArea->clear();

    // Aggiungi i file recenti alla lista widget
    foreach(const QString &file, recentFiles) {
        QListWidgetItem *item = new QListWidgetItem(file);
        ui->historyArea->addItem(item);
    }
}



void StartupWindow::on_localOpenPushButton_clicked()
{
    QString fn = QFileDialog::getOpenFileName(this, tr("Import an image file"), QString(),
                                              tr("FITS images (*.fit *.fits)"));
    if (fn.isEmpty()) {
        return;
    }

    if (AstroUtils::isFitsImage(fn.toStdString())) {
        //    openLocalImage(fn);
    } else {
        openLocalDC(fn);
    }
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
                    //  auto dialog = new FitsHeaderModifierDialog(fn, qMissing);
                    //  dialog->show();
                    return;
                }
            }
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error", QString::fromUtf8(e.what()));
            return;
        }

        new vtkwindow_new(this, fitsReader_dc, 1, nullptr);
        return;
    }


    //long maxSize = settings.value("downscaleSize", 1024).toInt() * 1024; // MB->KB
    long size = QFileInfo(fn).size() / 1024; // B -> KB
    int ScaleFactor = 1;// AstroUtils::calculateResizeFactor(size, maxSize);

    vtkWindowCube *win = new vtkWindowCube(nullptr, fn, ScaleFactor);
    win->show();
    win->activateWindow();
    win->raise();

    // Creazione del gestore dei file recenti
    RecentFilesManager *recentFilesManager = new RecentFilesManager(this);

    // Aggiungi alcuni file recenti di esempio al gestore dei file recenti
    recentFilesManager->addRecentFile(fn);
    PopulateHistory();
}

