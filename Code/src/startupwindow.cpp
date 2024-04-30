#include "startupwindow.h"
#include "ui_startupwindow.h"

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

    ui->historyArea->setAutoFillBackground(true);
    ui->historyArea->setPalette(pal2);

    ui->buttonArea->setAutoFillBackground(true);
    ui->buttonArea->setPalette(pal2);

}

StartupWindow::~StartupWindow()
{
    delete ui;
}
