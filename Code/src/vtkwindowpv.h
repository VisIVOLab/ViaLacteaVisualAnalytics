#ifndef VTKWINDOWPV_H
#define VTKWINDOWPV_H

#include <QMainWindow>

#include "vtkfitsreader2.h"

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageMapToColors.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkRenderer.h>

namespace Ui {
class vtkWindowPV;
}

class vtkWindowPV : public QMainWindow
{
    Q_OBJECT

public:
    explicit vtkWindowPV(const QString &filepath, QWidget *parent = nullptr);
    ~vtkWindowPV();

private slots:
    void changeLutScale();
    void changeOpacity(int opacity);
    void changeLut(const QString &lutName);
    void saveAsImage();

private:
    Ui::vtkWindowPV *ui;
    QString filepath;
    vtkNew<vtkFitsReader2> reader;
    vtkNew<vtkLookupTable> lut;
    vtkNew<vtkImageSlice> image;
};

#endif // VTKWINDOWPV_H
