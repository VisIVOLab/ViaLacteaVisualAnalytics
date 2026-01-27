#include "AboutDialog.h"
#include "ui_AboutDialog.h"

using namespace Qt::StringLiterals;

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent), ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->labelLogo->setPixmap(QPixmap(u":/icons/VisIVO.svg"_s));
    ui->label->setText(u"%1 v%2"_s.arg(qApp->applicationName(), qApp->applicationVersion()));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
