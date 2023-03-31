#ifndef FITSIMAGESTATISITICINFO_H
#define FITSIMAGESTATISITICINFO_H

#include <QWidget>

#include <vtkSmartPointer.h>

namespace Ui {
class FitsImageStatisiticInfo;
}

class vtkFitsReader;
class vtkFitsReader2;

class FitsImageStatisiticInfo : public QWidget
{
    Q_OBJECT

public:
    explicit FitsImageStatisiticInfo(vtkSmartPointer<vtkFitsReader2> readerCube,
                                     QWidget *parent = 0);
    ~FitsImageStatisiticInfo();

    void showSliceStats(vtkSmartPointer<vtkFitsReader2> readerSlice);
    void showMomentStats(vtkSmartPointer<vtkFitsReader> moment);

private:
    Ui::FitsImageStatisiticInfo *ui;
};

#endif // FITSIMAGESTATISITICINFO_H
