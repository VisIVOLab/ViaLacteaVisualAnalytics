#include "loadingwidget.h"
#include "ui_loadingwidget.h"
#include <QDebug>

LoadingWidget::LoadingWidget(QWidget *parent) : QWidget(parent), ui(new Ui::LoadingWidget)
{
    ui->setupUi(this);
    reply = 0;
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

void LoadingWidget::updateProgressBar(qint64 current, qint64 max)
{
    if (max > 0) {
        ui->progressBar->setMaximum(max);
        ui->progressBar->setValue(current);
    }
}

void LoadingWidget::setText(QString name)
{
    ui->titleLabel->setText(name);
}

void LoadingWidget::setLoadingProcess(QNetworkReply *reply)
{
    this->reply = reply;
}

void LoadingWidget::setButtonStatus(bool enabled)
{
    ui->dismissPushButton->setEnabled(enabled);
}

void LoadingWidget::loadingEnded()
{
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(100);
    ui->dismissPushButton->setEnabled(true);
}

void LoadingWidget::on_dismissPushButton_clicked()
{
    if (reply) {
        reply->abort();
        reply = 0;
    }
}
