#include "loadingwidget.h"
#include "ui_loadingwidget.h"
#include <QDebug>

LoadingWidget::LoadingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoadingWidget)
{
    ui->setupUi(this);
    //this->setWindowFlags(this->windowFlags() |  Qt::WindowStaysOnTopHint);

}

LoadingWidget::~LoadingWidget()
{
    delete ui;
}

void LoadingWidget::init()
{
    ui->progressBar->setMaximum(0);
    ui->progressBar->setValue(0);
}

void LoadingWidget::setFileName(QString name)
{
    ui->titleLabel->setText(name);
}

void LoadingWidget::loadingEnded()
{
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(100);

    ui->dismissPushButton->setEnabled(true);
}

void LoadingWidget::on_dismissPushButton_clicked()
{
    close();
}
