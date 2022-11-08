#include "fitsimagestatisiticinfo.h"
#include "ui_fitsimagestatisiticinfo.h"

FitsImageStatisiticInfo::FitsImageStatisiticInfo(vtkSmartPointer<vtkFitsReader2> readerCube,
                                                 vtkSmartPointer<vtkFitsReader2> readerSlice,
                                                 QWidget *parent)
    : QWidget(parent),
      ui(new Ui::FitsImageStatisiticInfo),
      readerCube(readerCube),
      readerSlice(readerSlice)
{
    ui->setupUi(this);
    ui->textCubeMin->setText(QString::number(readerCube->GetValueRange()[0], 'f', 4));
    ui->textCubeMax->setText(QString::number(readerCube->GetValueRange()[1], 'f', 4));
    ui->textCubeMean->setText(QString::number(readerCube->GetMean(), 'f', 4));
    ui->textCubeRMS->setText(QString::number(readerCube->GetRMS(), 'f', 4));

    updateSliceStats();
}

FitsImageStatisiticInfo::~FitsImageStatisiticInfo()
{
    delete ui;
}

void FitsImageStatisiticInfo::updateSliceStats()
{
    if (!readerSlice)
        return;

    ui->textSliceMin->setText(QString::number(readerSlice->GetValueRange()[0], 'f', 4));
    ui->textSliceMax->setText(QString::number(readerSlice->GetValueRange()[1], 'f', 4));
    ui->textSliceMean->setText(QString::number(readerSlice->GetMean(), 'f', 4));
    ui->textSliceRMS->setText(QString::number(readerSlice->GetRMS(), 'f', 4));
}
