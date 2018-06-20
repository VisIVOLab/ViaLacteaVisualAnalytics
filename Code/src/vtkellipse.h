#ifndef VTKELLIPSE_H
#define VTKELLIPSE_H

#include "vtkActor.h"
#include "vtkSmartPointer.h"
#include "vtkLODActor.h"
#include "vtkRect.h"
#include "qstring.h"
#include "vstabledesktop.h"
#include "vtkActor2D.h"
#include "vtkLODProp3D.h"
#include "vtkImageActor.h"
#include "vtkPolyData.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkRotationFilter.h"

class vtkEllipse
{
public:
    //vtkEllipse(float SemiMajorAxisLength,float SemiMinorAxisLength,float angle, float c_x, float c_y, float arcpixel,int band,int row, VSTable *table);
    vtkEllipse(float SemiMajorAxisLength,float SemiMinorAxisLength,float angle, float c_x, float c_y, float arcpixel,int band,int row,QString  name, VSTableDesktop *table, int numid=0, int numid_in=0);
    ~vtkEllipse();
    vtkSmartPointer<vtkImageActor> getActor();
    bool isInsideRect(vtkRectf *rect);
    QString getSourceName();
    VSTableDesktop* getTable();
    int getRowInTable(){return rowInTable;}

    vtkSmartPointer<vtkPolyData> getPolyData();
    vtkSmartPointer<vtkTransformPolyDataFilter> getTransformFilter();
    int getNumidtree();
    int getNumid_intree();

private:
    vtkSmartPointer<vtkImageActor> ellipseActor;
    float center_x;
    float center_y;
    int idSingleBand;
    int rowInTable;
    QString sourceName;
    VSTableDesktop *vstable;
    vtkSmartPointer<vtkPolyData> output;
    vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter ;
    int numidtree;
    int numid_intree;
};

#endif // VTKELLIPSE_H
