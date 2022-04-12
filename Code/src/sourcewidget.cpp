#include "sourcewidget.h"
#include "ui_sourcewidget.h"

#include "source.h"

SourceWidget::SourceWidget(QWidget *parent) : QWidget(parent), ui(new Ui::SourceWidget)
{
    ui->setupUi(this);
}

SourceWidget::~SourceWidget()
{
    delete ui;
}

void SourceWidget::showSourceInfo(const Source *source)
{
    ui->labelName->setText(source->getName());
    ui->labelIndex->setText(QString::number(source->getIndex()));
    ui->labelIauName->setText(source->getIau_name());
    ui->labelClassId->setText(QString::number(source->getClassid()));
    ui->labelLabel->setText(source->getLabel());
    ui->labelNumIslands->setText(QString::number(source->getNislands()));
    ui->labelX->setText(QString::number(source->getX0(), 'f', 6));
    ui->labelY->setText(QString::number(source->getY0(), 'f', 6));
    ui->labelRa->setText(QString::number(source->getRa(), 'f', 6));
    ui->labelDec->setText(QString::number(source->getDec(), 'f', 6));
    ui->labelMorpthLabel->setText(source->getMorph_label());
    ui->labelSourceLabel->setText(source->getSourceness_label());
    ui->labelSourceScore->setText(QString::number(source->getSourceness_score(), 'f', 6));
}
