#include "startupwindow.h"
#include "ui_startupwindow.h"
#include "recentfilesmanager.h"

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

    PoulateHistory();

}

StartupWindow::~StartupWindow()
{
    delete ui;
}


void StartupWindow::PoulateHistory()
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


