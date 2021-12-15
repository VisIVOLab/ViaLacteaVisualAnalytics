#include "aboutform.h"
#include "ui_aboutform.h"

#include <QDesktopServices>

AboutForm::AboutForm(QWidget *parent)
    : QWidget(parent, Qt::Window), ui(new Ui::AboutForm), neaniasUrl("https://www.neanias.eu")
{
    ui->setupUi(this);
    ui->labelVersion->setText("v " + qApp->applicationVersion());

    QPixmap logo(":/icons/NEANIAS.jpg");
    ui->neaniasLogo->setIcon(logo);
    ui->neaniasLogo->setIconSize(logo.rect().size());

    setAttribute(Qt::WA_DeleteOnClose);
}

AboutForm::~AboutForm()
{
    delete ui;
}

void AboutForm::on_neaniasLogo_clicked()
{
    QDesktopServices::openUrl(neaniasUrl);
}
