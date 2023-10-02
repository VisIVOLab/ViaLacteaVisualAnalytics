#ifndef VTKWINDOWPV_H
#define VTKWINDOWPV_H

#include <QMainWindow>

#include "vtkfitsreader2.h"

#include <vtkNew.h>

class vtkCoordinate;
class vtkImageSlice;
class vtkLookupTable;

namespace Ui {
class vtkWindowPV;
}

class vtkWindowPV : public QMainWindow
{
    Q_OBJECT

public:
    explicit vtkWindowPV(const QString &filepath, const QString &cubepath, float x1, float y1,
                         float x2, float y2, QWidget *parent = nullptr);
    ~vtkWindowPV();

private slots:
    void changeLutScale();
    void changeOpacity(int opacity);
    void changeLut(const QString &lutName);
    void saveAsImage();

private:
    Ui::vtkWindowPV *ui;
    QString filepath;
    QString cubepath;
    float x1;
    float x2;
    float m;
    float q;
    vtkNew<vtkCoordinate> coordinate;
    vtkNew<vtkFitsReader2> reader;
    vtkNew<vtkLookupTable> lut;
    vtkNew<vtkImageSlice> image;

    void coordsCallback();
};

#endif // VTKWINDOWPV_H
