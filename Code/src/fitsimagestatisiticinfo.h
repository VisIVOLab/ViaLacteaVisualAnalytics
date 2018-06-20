#ifndef FITSIMAGESTATISITICINFO_H
#define FITSIMAGESTATISITICINFO_H

#include <QWidget>
#include "vtkwindow_new.h"

namespace Ui {
class FitsImageStatisiticInfo;
}

class FitsImageStatisiticInfo : public QWidget
{
    Q_OBJECT

public:
    explicit FitsImageStatisiticInfo(vtkwindow_new *v, QWidget *parent = 0);
    ~FitsImageStatisiticInfo();
    void setGalaptic(double l, double b);
    void setEcliptic(double lat, double lon);
    void setFk5(double ra, double dec);
    void setImage(double x, double y);
    void setFilename();




private:
    Ui::FitsImageStatisiticInfo *ui;
    vtkwindow_new *vtkwin;
};

#endif // FITSIMAGESTATISITICINFO_H
