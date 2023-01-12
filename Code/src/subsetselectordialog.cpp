#include "subsetselectordialog.h"
#include "ui_subsetselectordialog.h"

#include <QDebug>
#include <QIntValidator>

SubsetSelectorDialog::SubsetSelectorDialog(QWidget *parent, const CubeSubset &subset)
    : QDialog(parent), ui(new Ui::SubsetSelectorDialog), subset(subset)
{
    ui->setupUi(this);

    ui->checkSubset->setChecked(subset.ReadSubExtent);
    ui->textX0->setText(QString::number(subset.SubExtent[0]));
    ui->textX1->setText(QString::number(subset.SubExtent[1]));
    ui->textY0->setText(QString::number(subset.SubExtent[2]));
    ui->textY1->setText(QString::number(subset.SubExtent[3]));
    ui->textZ0->setText(QString::number(subset.SubExtent[4]));
    ui->textZ1->setText(QString::number(subset.SubExtent[5]));
    ui->checkAutoScale->setChecked(subset.AutoScale);
    ui->textCubeMaxSize->setText(QString::number(subset.CubeMaxSize));
    ui->textScaleFactor->setText(QString::number(subset.ScaleFactor));

    QIntValidator v(this);
    ui->textX0->setValidator(&v);
    ui->textX1->setValidator(&v);
    ui->textY0->setValidator(&v);
    ui->textY1->setValidator(&v);
    ui->textZ0->setValidator(&v);
    ui->textZ1->setValidator(&v);
    ui->textCubeMaxSize->setValidator(&v);
    ui->textScaleFactor->setValidator(&v);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SubsetSelectorDialog::btnOkClicked);
}

SubsetSelectorDialog::~SubsetSelectorDialog()
{
    delete ui;
}

void SubsetSelectorDialog::btnOkClicked()
{
    subset.ReadSubExtent = ui->checkSubset->isChecked();
    subset.SubExtent[0] = ui->textX0->text().toInt();
    subset.SubExtent[1] = ui->textX1->text().toInt();
    subset.SubExtent[2] = ui->textY0->text().toInt();
    subset.SubExtent[3] = ui->textY1->text().toInt();
    subset.SubExtent[4] = ui->textZ0->text().toInt();
    subset.SubExtent[5] = ui->textZ1->text().toInt();
    subset.AutoScale = ui->checkAutoScale->isChecked();
    subset.CubeMaxSize = ui->textCubeMaxSize->text().toInt();
    subset.ScaleFactor = ui->textScaleFactor->text().toInt();

    emit subsetSelected(subset);
}
