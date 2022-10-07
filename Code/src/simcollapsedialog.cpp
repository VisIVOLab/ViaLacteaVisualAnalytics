#include "simcollapsedialog.h"
#include "ui_simcollapsedialog.h"

#include <QDoubleValidator>

SimCollapseDialog::SimCollapseDialog(double *angles, QWidget *parent)
    : QDialog(parent), ui(new Ui::SimCollapseDialog)
{
    ui->setupUi(this);
    ui->textX->setText(QString::number(angles[0], 'g', 4));
    ui->textY->setText(QString::number(angles[1], 'g', 4));
    ui->textZ->setText(QString::number(angles[2], 'g', 4));

    auto v = new QDoubleValidator(this);
    ui->textScale->setValidator(v);
    ui->textLon->setValidator(v);
    ui->textLat->setValidator(v);
    ui->textDistance->setValidator(v);
    ui->textSigma->setValidator(v);
}

SimCollapseDialog::~SimCollapseDialog()
{
    delete ui;
}

void SimCollapseDialog::on_buttonBox_rejected()
{
    this->close();
}

void SimCollapseDialog::on_buttonBox_accepted()
{
    double scale = ui->textScale->text().toDouble();
    double lon = ui->textLon->text().toDouble();
    double lat = ui->textLat->text().toDouble();
    double distance = ui->textDistance->text().toDouble();
    double sigma = ui->textSigma->text().toDouble();
    emit dialogSubmitted(scale, lon, lat, distance, sigma);
    this->close();
}
