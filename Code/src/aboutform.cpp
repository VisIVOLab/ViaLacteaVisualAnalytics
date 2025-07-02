#include "aboutform.h"
#include "ui_aboutform.h"

#include <QDesktopServices>

AboutForm::AboutForm(QWidget *parent)
    : QWidget(parent, Qt::Window),
      ui(new Ui::AboutForm),
      neaniasUrl("https://www.neanias.eu"),
      cirasaUrl("https://www.oact.inaf.it/project/cirasa"),
      ecogalUrl("http://www.ecogal.eu"),
      cnUrl("https://www.supercomputing-icsc.it")
{
    ui->setupUi(this);
    ui->labelVersion->setText("v " + qApp->applicationVersion());
    setAttribute(Qt::WA_DeleteOnClose);
}

AboutForm::~AboutForm()
{
    delete ui;
}

void AboutForm::on_btnNeanias_clicked()
{
    QDesktopServices::openUrl(neaniasUrl);
}

void AboutForm::on_btnCirasa_clicked()
{
    QDesktopServices::openUrl(cirasaUrl);
}

void AboutForm::on_btnEcogal_clicked()
{
    QDesktopServices::openUrl(ecogalUrl);
}

void AboutForm::on_btnCN_clicked()
{
    QDesktopServices::openUrl(cnUrl);
}
