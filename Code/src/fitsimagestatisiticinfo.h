#ifndef FITSIMAGESTATISITICINFO_H
#define FITSIMAGESTATISITICINFO_H

#include "vtkfitsreader2.h"

#include <QWidget>

#include <vtkSmartPointer.h>

namespace Ui {
class FitsImageStatisiticInfo;
}

class FitsImageStatisiticInfo : public QWidget
{
    Q_OBJECT

public:
    explicit FitsImageStatisiticInfo(vtkSmartPointer<vtkFitsReader2> readerCube,
                                     vtkSmartPointer<vtkFitsReader2> readerSlice,
                                     QWidget *parent = 0);
    ~FitsImageStatisiticInfo();

    void updateSliceStats();

private:
    Ui::FitsImageStatisiticInfo *ui;
    vtkSmartPointer<vtkFitsReader2> readerCube;
    vtkSmartPointer<vtkFitsReader2> readerSlice;
};

#endif // FITSIMAGESTATISITICINFO_H
