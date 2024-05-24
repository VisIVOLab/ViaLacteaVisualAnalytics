#include "startupwindow.h"
#include "ui_startupwindow.h"

#include "astroutils.h"
#include "fitsheadermodifierdialog.h"
#include "fitsheaderviewer.h"
#include "recentfilesmanager.h"
#include "settingform.h"
#include "vialactea.h"
#include "vtkfitsreader.h"
#include "vtkwindow_new.h"
#include "vtkwindowcube.h"

#include <QFileDialog>
#include <QMessageBox>
#include "visivomenu.h"
#include "vialacteainitialquery.h"

#include "sessionmanagermodel.h"


StartupWindow::StartupWindow(QWidget *parent)
: QWidget(parent),
ui(new Ui::StartupWindow),
settingsFile(QDir::homePath().append("/VisIVODesktopTemp/setting.ini")),
settings(settingsFile, QSettings::IniFormat),
sessionModel(nullptr)  // Initialize to nullptr
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);
    ui->headerViewer->hide();
    
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
    
    visivoMenu = new VisIVOMenu();
    visivoMenu->configureStartupMenu();
    
    this->layout()->setMenuBar(visivoMenu);
    connect(visivoMenu, &VisIVOMenu::loadLocalFitsFileRequested, this, [this](){
        on_localOpenPushButton_clicked(false); });
    
    setupSessionManager();
    
}

StartupWindow::~StartupWindow()
{
    delete ui;
}

void StartupWindow::setupSessionManager()
{
    sessionModel = new SessionManagerModel(this);
    ui->sessionManagerTreeView->setModel(sessionModel);
    ui->sessionManagerTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->sessionManagerTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->sessionManagerTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void StartupWindow::on_localOpenPushButton_clicked(bool fromHistory = false)
{
    QString fn;
    if (!fromHistory)
        fn = QFileDialog::getOpenFileName(this, tr("Import an image file"), QString(),
                                          tr("FITS images (*.fit *.fits)"));
    else {
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
        openLocalImage(fn);
    } else {
        openLocalDC(fn);
    }
}

void StartupWindow::on_clearPushButton_clicked()
{
    ui->headerViewer->hide();
    this->historyModel->clearHistory();
}
void StartupWindow::openLocalImage(const QString &fn)
{
    auto fits = vtkSmartPointer<vtkFitsReader>::New();
    fits->SetFileName(fn.toStdString());
    
    bool doSearch = settings.value("vlkb.search", false).toBool();
    
    if (doSearch) {
        double coords[2], rectSize[2];
        AstroUtils::GetCenterCoords(fn.toStdString(), coords);
        AstroUtils::GetRectSize(fn.toStdString(), rectSize);
        
        VialacteaInitialQuery *vq = new VialacteaInitialQuery;
        connect(vq, &VialacteaInitialQuery::searchDone, this,
                [vq, fn, fits, this](QList<QMap<QString, QString>> results) {
            auto win = new vtkwindow_new(this, fits);
            win->setDbElements(results);
            sessionModel->addSessionItem(QFileInfo(fn).baseName(), win);  // Add to session manager
            vq->deleteLater();
        });
        
        vq->searchRequest(coords[0], coords[1], rectSize[0], rectSize[1]);
    }
    else
    {
        auto win = new vtkwindow_new(this, fits);
        sessionModel->addSessionItem(QFileInfo(fn).baseName(), win);  // Add to session manager

    }
    this->historyModel->addRecentFile(fn);
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
                    auto dialog = new FitsHeaderModifierDialog(fn, qMissing, this);
                    dialog->show();
                    return;
                }
            }
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error", QString::fromUtf8(e.what()));
            return;
        }
        auto win = new vtkwindow_new(this, fitsReader_dc, 1);
        sessionModel->addSessionItem(QFileInfo(fn).baseName(), win);  // Add to session manager

        this->historyModel->addRecentFile(fn);
        return;
    }
    
    long maxSize = settings.value("downscaleSize", 1024).toInt() * 1024; // MB->KB
    long size = QFileInfo(fn).size() / 1024; // B -> KB
    int ScaleFactor = AstroUtils::calculateResizeFactor(size, maxSize);
    
    vtkWindowCube *win = new vtkWindowCube(this, fn, ScaleFactor);
    sessionModel->addSessionItem(QFileInfo(fn).baseName(), win);  // Add to session manager

    win->show();
    win->activateWindow();
    win->raise();
    
    // Aggiungi alcuni file recenti di esempio al gestore dei file recenti
    this->historyModel->addRecentFile(fn);
}

void StartupWindow::showFitsHeader(const QString &fn)
{
    if (!fn.isEmpty()) {
        ui->headerViewer->showHeader(fn);
        ui->headerViewer->show();
    }
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
    if (!vialactealWin) {
        vialactealWin = new ViaLactea(this);
    }
    vialactealWin->show();
    vialactealWin->activateWindow();
    vialactealWin->raise();
    
    
}

void StartupWindow::on_historyArea_activated(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    
    this->showFitsHeader(index.data().toString());
}

void StartupWindow::on_historyArea_clicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    
    this->showFitsHeader(index.data().toString());
}


void StartupWindow::changeEvent(QEvent *e)
{
    /*
     if(e->type() == QEvent::ActivationChange && this->isActiveWindow()) {
     visivoMenu->configureStartupMenu();
     qDebug()<<"connetto";
     connect(visivoMenu, &VisIVOMenu::loadLocalFitsFileRequested, this, [this](){
     on_localOpenPushButton_clicked(false); });
     
     }
     */
}

void StartupWindow::closeEvent(QCloseEvent *event)
{
    
    auto res = QMessageBox::question(this, "Confirm exit",
                                     "Do you want to exit?.\nClosing this "
                                     "window will terminate ongoing processes.",
                                     QMessageBox::Yes | QMessageBox::No);
    
    if (res == QMessageBox::No) {
        event->ignore();
        return;
    }
    
    QApplication::quit();
}
