#include "vtkellipse.h"
#include "vtkMath.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "qdebug.h"
#include <cmath>
#include "vtkImageViewer2.h"
#include "vtkImageData.h"
#include <vtkWindowToImageFilter.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>
#include <vtkTransform.h>
#include "vtkTransformPolyDataFilter.h"
#include "vtkRotationFilter.h"


vtkEllipse::vtkEllipse(float SemiMajorAxisLength, float SemiMinorAxisLength, float angle, float c_x, float c_y,float arcpixel,int band,int row,QString  name, VSTableDesktop *table, int numid, int numid_in )
//vtkEllipse::vtkEllipse(float SemiMajorAxisLength, float SemiMinorAxisLength, float angle, float c_x, float c_y,float arcpixel,int band,int row, VSTable *table )
{

    idSingleBand=band;
    rowInTable=row;

    vstable=table;
    sourceName=name;

    numidtree=numid;
    numid_intree=numid_in;

    int NumberOfSteps=20;

    center_x=c_x;
    center_y=c_y;

    //double arcpixel=6;

    double interval = 2 * vtkMath::Pi() / NumberOfSteps;
    double alpha, sinalpha, cosalpha;
    double pt[3];
    pt[2] = 0.0;

    vtkPoints *new_points;
    new_points = vtkPoints::New();
    new_points->Allocate(NumberOfSteps);
    vtkCellArray *new_lines;
    new_lines = vtkCellArray::New();
    new_lines->Allocate(NumberOfSteps);

    vtkIdType prev_pt=0, cur_pt=0, pts[2];



    // calculate points
    for (int i = 0; i < NumberOfSteps; i++)
    {
        /*
       alpha = i * interval + angle/arcpixel;//+41.7; // current angle
       sinalpha = sin(alpha);
       cosalpha = cos(alpha);


       pt[0] = SemiMajorAxisLength/arcpixel * cosalpha + c_x;//150;
       pt[1] = SemiMinorAxisLength/arcpixel * sinalpha + c_y;//1197;


       prev_pt = cur_pt;
       cur_pt = new_points->InsertNextPoint(pt);


       if (i > 0)
       {
         pts[0] = prev_pt;
         pts[1] = cur_pt;
         new_lines->InsertNextCell(2, pts);
       }
        */



        //double alpha2 = ((angle * (vtkMath::Pi() / 180.0) ) / arcpixel);

        alpha = i * interval; // current angle


        sinalpha = sin(alpha);
        cosalpha = cos(alpha);

        //        pt[0] = SemiMajorAxisLength/arcpixel  * cosalpha + c_x;
        //        pt[1] = SemiMinorAxisLength/arcpixel  * sinalpha + c_y;

        double x = SemiMajorAxisLength/arcpixel  * cosalpha + c_x;
        double y = SemiMinorAxisLength/arcpixel  * sinalpha +c_y;

        pt[0] =c_x + (x-c_x) * cos (angle) - (y-c_y) * sin (angle);
        pt[1] =c_y + (y-c_y) * cos (angle) + (x-c_x) * sin (angle);



        prev_pt = cur_pt;
        cur_pt = new_points->InsertNextPoint(pt);

        if (i > 0)
        {
            pts[0] = prev_pt;
            pts[1] = cur_pt;
            new_lines->InsertNextCell(2, pts);
        }



    }

    // connect up the last segment
    pts[0] = pts[1];
    pts[1] = 0;
    new_lines->InsertNextCell(2, pts);


    output = vtkSmartPointer<vtkPolyData>::New();
    output->SetPoints(new_points);
    output->SetLines(new_lines);

    new_points->Delete();
    new_lines->Delete();
}

vtkSmartPointer<vtkPolyData> vtkEllipse::getPolyData()
{
    return output;
}

vtkSmartPointer<vtkTransformPolyDataFilter> vtkEllipse::getTransformFilter()
{
    return transformFilter;
}



VSTableDesktop* vtkEllipse::getTable()
{
    return vstable;
}


vtkSmartPointer<vtkImageActor> vtkEllipse::getActor()
{

    return ellipseActor;
}

bool vtkEllipse::isInsideRect(vtkRectf *rect)
{

    float x1 = rect->GetX();
    float y1 = rect->GetY();

    float x2 = rect->GetX() + rect->GetWidth();
    float y2 = rect->GetY();

    float x3 = rect->GetX() + rect->GetWidth();
    float y3 = rect->GetY() + rect->GetHeight();

    float x4 = rect->GetX();
    float y4 = rect->GetY()+rect->GetHeight();

    //START
    //1 - center - 4
    float area_triangolo_1 = fabs (0.5*( x1*(center_y-y4) + center_x*(y4-y1) + x4*(y1-center_y)) );
    //4 - center - 3
    float area_triangolo_2 = fabs (0.5*( x4*(center_y-y3) + center_x*(y3-y4) + x3*(y4-center_y)) );
    //3 - center - 2
    float area_triangolo_3 = fabs (0.5*( x3*(center_y-y2) + center_x*(y2-y3) + x2*(y3-center_y)) );
    //center - 2 - 1
    float area_triangolo_4 = fabs( 0.5*( center_x*(y2-y1) + x2*(y1-center_y) + x1*(center_y-y2)) );

    float area_rettangolo = fabs( rect->GetWidth()*rect->GetHeight());

    float area_sum=area_triangolo_1+area_triangolo_2+area_triangolo_3+area_triangolo_4;

    if (fabs( (area_rettangolo - area_sum)/area_rettangolo ) < 0.1)
        return true;
    else
        return false;
    //END

}

QString vtkEllipse::getSourceName()
{
    return sourceName;
}

int vtkEllipse::getNumidtree()
{
    return numidtree;
}

int vtkEllipse::getNumid_intree()
{
    return numid_intree;
}


vtkEllipse::~vtkEllipse()
{

    output->Delete();


}

