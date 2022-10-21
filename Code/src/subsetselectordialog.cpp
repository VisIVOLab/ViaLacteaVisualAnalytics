#include "subsetselectordialog.h"
#include "ui_subsetselectordialog.h"

#include <QDebug>
#include <QIntValidator>

SubsetSelectorDialog::SubsetSelectorDialog(QWidget *parent, const CubeSubset &subset)
    : QDialog(parent), ui(new Ui::SubsetSelectorDialog), subset(subset)
{
    ui->setupUi(this);

    ui->checkSubset->setChecked(subset.readSubset);
    ui->textX0->setText(QString::number(subset.subset[0]));
    ui->textX1->setText(QString::number(subset.subset[1]));
    ui->textY0->setText(QString::number(subset.subset[2]));
    ui->textY1->setText(QString::number(subset.subset[3]));
    ui->textZ0->setText(QString::number(subset.subset[4]));
    ui->textZ1->setText(QString::number(subset.subset[5]));
    ui->textStepX->setText(QString::number(subset.readSteps[0]));
    ui->textStepY->setText(QString::number(subset.readSteps[1]));
    ui->textStepZ->setText(QString::number(subset.readSteps[2]));

    QIntValidator v(this);
    ui->textX0->setValidator(&v);
    ui->textX1->setValidator(&v);
    ui->textY0->setValidator(&v);
    ui->textY1->setValidator(&v);
    ui->textZ0->setValidator(&v);
    ui->textZ1->setValidator(&v);
    ui->textStepX->setValidator(&v);
    ui->textStepY->setValidator(&v);
    ui->textStepZ->setValidator(&v);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SubsetSelectorDialog::btnOkClicked);
}

SubsetSelectorDialog::~SubsetSelectorDialog()
{
    qDebug() << Q_FUNC_INFO;
    delete ui;
}

void SubsetSelectorDialog::btnOkClicked()
{
    subset.readSubset = ui->checkSubset->isChecked();
    subset.subset[0] = ui->textX0->text().toInt();
    subset.subset[1] = ui->textX1->text().toInt();
    subset.subset[2] = ui->textY0->text().toInt();
    subset.subset[3] = ui->textY1->text().toInt();
    subset.subset[4] = ui->textZ0->text().toInt();
    subset.subset[5] = ui->textZ1->text().toInt();
    subset.readSteps[0] = ui->textStepX->text().toInt();
    subset.readSteps[1] = ui->textStepY->text().toInt();
    subset.readSteps[2] = ui->textStepZ->text().toInt();

    emit subsetSelected(subset);
}
