#include "simplacedialog.h"
#include "ui_simplacedialog.h"

SimPlaceDialog::SimPlaceDialog(QWidget *parent) : QDialog(parent), ui(new Ui::SimPlaceDialog)
{
    ui->setupUi(this);
}

SimPlaceDialog::~SimPlaceDialog()
{
    delete ui;
}

void SimPlaceDialog::on_buttonBox_rejected()
{
    this->close();
}

void SimPlaceDialog::on_buttonBox_accepted()
{
    double lon = ui->spinLon->value();
    double lat = ui->spinLat->value();
    double distance = ui->spinDistance->value();
    emit dialogSubmitted(lon, lat, distance);
    this->close();
}
