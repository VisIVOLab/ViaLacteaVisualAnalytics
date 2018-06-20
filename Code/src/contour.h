#ifndef CONTOUR_H
#define CONTOUR_H

#include <QWidget>
#include <vtkActor.h>
#include <vtkMarchingSquares.h>
#include <vtkSmartPointer.h>
#include <QTreeWidgetItem>
#include <QAction>
#include "vtkfitsreader.h"
#include "vtkActor2D.h"
#include "vtkStripper.h"
#include "vtkwindow_new.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkScalarBarActor.h"



namespace Ui {
class contour;
}

class contour : public QWidget
{
    Q_OBJECT

public:
    explicit contour(QWidget *parent = 0);
    ~contour();
    void setFitsReader(vtkFitsReader *fits,vtkwindow_new *win);



    vtkActor *contour_actor;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkRenderWindow> renderWindow;
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;
    bool createwin_flag=false;
    void createwin();
    Ui::contour *ui;
    vtkSmartPointer<vtkActor> isolines;
    vtkSmartPointer<vtkActor2D> getIsolabels(){return isolabels;};
    vtkSmartPointer<vtkActor2D> getIdlabel(){return idlabel;};
    vtkSmartPointer<vtkScalarBarActor> getScalarBar(){return  scalarBar;};

public slots:
 void on_okButton_clicked();

private:

    vtkFitsReader *fitsReader;
    vtkwindow_new *vtkWin;
    vtkMarchingSquares *contours;
    vtkSmartPointer<vtkActor2D> isolabels;
    vtkSmartPointer<vtkActor2D> idlabel;
    vtkSmartPointer<vtkStripper> contourStripper;
    vtkSmartPointer<vtkScalarBarActor> scalarBar;



private slots:

    void createContour();
    void addContours2();
    void deleteContour();
    void addContours();
    void toggledLabel(bool t);
    void toggledId(bool t);

    //void on_pushButton_clicked();
};

#endif // CONTOUR_H
