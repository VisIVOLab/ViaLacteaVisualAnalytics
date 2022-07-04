#include "simcollapsedialog.h"
#include "ui_simcollapsedialog.h"

SimCollapseDialog::SimCollapseDialog(double *angles, QWidget *parent)
    : QDialog(parent), ui(new Ui::SimCollapseDialog)
{
    ui->setupUi(this);
    ui->textX->setText(QString::number(angles[0], 'g', ui->spinScale->decimals()));
    ui->textY->setText(QString::number(angles[1], 'g', ui->spinScale->decimals()));
    ui->textZ->setText(QString::number(angles[2], 'g', ui->spinScale->decimals()));
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
    double scale = ui->spinScale->value();
    double lon = ui->spinLon->value();
    double lat = ui->spinLat->value();
    double distance = ui->spinDistance->value();
    double beam = ui->spinBeam->value();
    emit dialogSubmitted(scale, lon, lat, distance, beam);
    this->close();
}
