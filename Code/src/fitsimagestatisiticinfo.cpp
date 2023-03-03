#include "fitsimagestatisiticinfo.h"
#include "ui_fitsimagestatisiticinfo.h"

#include "vtkfitsreader.h"
#include "vtkfitsreader2.h"

FitsImageStatisiticInfo::FitsImageStatisiticInfo(vtkSmartPointer<vtkFitsReader2> readerCube,
                                                 QWidget *parent)
    : QWidget(parent), ui(new Ui::FitsImageStatisiticInfo)
{
    ui->setupUi(this);
    ui->textCubeMin->setText(QString::number(readerCube->GetValueRange()[0], 'f', 4));
    ui->textCubeMax->setText(QString::number(readerCube->GetValueRange()[1], 'f', 4));
    ui->textCubeMean->setText(QString::number(readerCube->GetMean(), 'f', 4));
    ui->textCubeRMS->setText(QString::number(readerCube->GetRMS(), 'f', 4));
}

FitsImageStatisiticInfo::~FitsImageStatisiticInfo()
{
    delete ui;
}

void FitsImageStatisiticInfo::showSliceStats(vtkSmartPointer<vtkFitsReader2> readerSlice)
{
    if (!readerSlice)
        return;

    ui->boxSliceMom->setTitle("Slice");
    ui->textMin->setText(QString::number(readerSlice->GetValueRange()[0], 'f', 4));
    ui->textMax->setText(QString::number(readerSlice->GetValueRange()[1], 'f', 4));
    ui->textMean->setText(QString::number(readerSlice->GetMean(), 'f', 4));
    ui->textRMS->setText(QString::number(readerSlice->GetRMS(), 'f', 4));
}

void FitsImageStatisiticInfo::showMomentStats(vtkSmartPointer<vtkFitsReader> moment)
{
    if (!moment)
        return;

    ui->boxSliceMom->setTitle("Moment");
    ui->textMin->setText(QString::number(moment->GetMin(), 'f', 4));
    ui->textMax->setText(QString::number(moment->GetMax(), 'f', 4));
    ui->textMean->setText(QString::number(moment->GetMedia(), 'f', 4));
    ui->textRMS->setText(QString::number(moment->GetRMS(), 'f', 4));
}
