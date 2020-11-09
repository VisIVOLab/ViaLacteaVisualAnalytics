/***************************************************************************
 *   Copyright (C) 2010 by Ugo Becciani, Alessandro Costa                  *
 *   ugo.becciani@oact.inaf.it                                             *
 *   alessandro.costa@oact.inaf.it                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <sstream>
#include <cstdlib>
#include <cstring>
#include <map>
#include <QStringList>
#include <QString>
#include "pipe.h"
#include "visivoutilsdesktop.h"
#include "vstabledesktop.h"
#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"
#include "vtkRenderWindow.h"
#include "vtkCamera.h"
#include "vtkRenderer.h"
#include "vtkLookupTable.h"
#include "vtkPolyData.h"

#include "vtkOutlineCornerFilter.h"
#include "vtkProperty.h"
#include "vtkScalarBarActor.h"

#include "vtkCubeAxesActor2D.h"
#include "vtkDataSet.h"
#include "vstabledesktop.h"
#include "vtkLODActor.h"

#include "qdebug.h"
#include "vtkwindow_new.h"
#include "vispoint.h"

#include "vtkPolygon.h"

Pipe::Pipe(VSTableDesktop *table)
{
    m_VSTable = table;
}

VSTableDesktop* Pipe::getTable()
{
    return m_VSTable;
}

void Pipe::constructVTK(vtkwindow_new *v)
{
    m_lut           = vtkLookupTable::New();
    vtkwin=v;
    // m_pRenderer     = vtkRenderer::New();
    //  m_pRenderWindow = vtkRenderWindow::New();
    m_pRenderer = vtkwin -> m_Ren1;
    m_pRenderWindow = vtkwin -> renwin;



}

int Pipe::createPipe ()

{
    return 0;
}

int  Pipe::getCamera ()

{
    return 0;
}

/*
bool Pipe::readData (QStringList list)
{
    std::ifstream inFile;
    m_path=m_VSTable->getLocator();

    inFile.open(m_path.c_str(), ios::binary);
    if(!inFile.good())
    {
        std::cerr<<"Error: binary data does not exist"<<std::endl;
        exit(1);
    }

    //--------------------------------------------------------------------------
    // m_fieldNames is a Vector of strings containing field names
    //--------------------------------------------------------------------------

    m_fieldNames.push_back(list[0].toStdString().c_str());
    m_fieldNames.push_back(list[1].toStdString().c_str());
    m_fieldNames.push_back(list[2].toStdString().c_str());


    m_nRows= m_VSTable->getNumberOfRows();
    m_nCols=0;

    //--------------------------------------------------------------------------
    // m_colNames is a  map with valid fieldNames & corresponding ColID
    //--------------------------------------------------------------------------
    for (int i = 0; i < m_fieldNames.size(); i++)
    {
        if (m_VSTable->getColId(m_fieldNames[i])>=0)
        {
            m_colNames.insert(make_pair(m_fieldNames[i],m_nCols));
            m_nCols++; // m_nCols will be  m_colNames.size()
        }
    }

    //--------------------------------------------------------------------------
    // colList is an array containing indexes of the m_colNames map
    //--------------------------------------------------------------------------
    unsigned int *colList;
    colList = new unsigned int [m_nCols];
    int count=0;
    for (int i = 0; i < m_fieldNames.size(); i++)
    {
        if (m_VSTable->getColId(m_fieldNames[i])>=0)
        {
            colList[count]=m_VSTable->getColId(m_fieldNames[i]);
            count++;
        }
    }

    //--------------------------------------------------------------------------
    // m_tableData is a bidimens. float array m_nCols*m_nRows
    //--------------------------------------------------------------------------
    m_tableData = new float* [m_nCols];
    for (int i = 0; i < m_nCols; i++)

    {
        try
        {
            m_tableData[i]=new  float[m_nRows];
        }
        catch(std::bad_alloc &e)
        {
            m_tableData[i]=NULL;
        }
        if(m_tableData[i]==NULL)
        {
            for(unsigned int j=0;j<i;j++) delete [] m_tableData[j];
            delete [] m_tableData;
            return false;
        }
    }

 //int VSTable::getColumn(unsigned int *colList, unsigned int nOfCol, unsigned long long int fromRow, unsigned long long int toRow, float **fArray)

    //--------------------------------------------------------------------------
    // VSTable::getColumn reads the data
    //--------------------------------------------------------------------------
    m_VSTable->getColumn(colList,m_nCols,0,m_nRows-1, m_tableData);
    //delete table;
    return true;
}
*/

bool Pipe::readData (QStringList list)
{
    //  std::ifstream inFile;
    m_path=m_VSTable->getLocator();
    /*
    inFile.open(m_path.c_str(), ios::binary);
    if(!inFile.good())
    {
        std::cerr<<"Error: binary data does not exist"<<std::endl;
        exit(1);
    }
*/
    //--------------------------------------------------------------------------
    // m_fieldNames is a Vector of strings containing field names
    //--------------------------------------------------------------------------


    m_fieldNames.push_back(list[0].toStdString().c_str());
    m_fieldNames.push_back(list[1].toStdString().c_str());
    m_fieldNames.push_back(list[2].toStdString().c_str());

    m_nRows= m_VSTable->getNumberOfRows();
    m_nCols= m_VSTable->getNumberOfColumns();

    qDebug()<<"m_nCols="<<m_nCols<<" m_nRows: "<<m_nRows;
    qDebug()<<list[0];
    qDebug()<<list[1];
    qDebug()<<list[2];

    //  unsigned int *colList;
    //  colList = new unsigned int [m_nCols];
    /*
    //--------------------------------------------------------------------------
    // m_colNames is a  map with valid fieldNames & corresponding ColID
    //--------------------------------------------------------------------------
    for (int i = 0; i < m_nCols; i++)
    {

        m_colNames.insert(make_pair(m_VSTable->getColName(i),i));

        colList[i]=m_VSTable->getColId(m_VSTable->getColName(i));
        //qDebug()<<"i="<<i<<" collist[]="<<colList[i]<<m_VSTable->getColName(i).c_str();

    }
*/

    //--------------------------------------------------------------------------
    // colList is an array containing indexes of the m_colNames map
    //--------------------------------------------------------------------------
    unsigned int *colList;
    colList = new unsigned int [m_nCols];
    int count=0;
    for (int i = 0; i < m_fieldNames.size(); i++)
    {
        if (m_VSTable->getColId(m_fieldNames[i])>=0)
        {
            m_colNames.insert(make_pair(m_VSTable->getColName(i),i));

            colList[count]=m_VSTable->getColId(m_fieldNames[i]);
            count++;
        }
    }


    //--------------------------------------------------------------------------
    // m_tableData is a bidimens. float array m_nCols*m_nRows
    //--------------------------------------------------------------------------
    m_tableData = new float* [m_nCols];
    for (int i = 0; i < m_nCols; i++)
    {
        try
        {
            m_tableData[i]=new  float[m_nRows];
        }
        catch(std::bad_alloc &e)
        {
            m_tableData[i]=NULL;
        }
        if(m_tableData[i]==NULL)
        {
            for(unsigned int j=0;j<i;j++) delete [] m_tableData[j];
            delete [] m_tableData;
            return false;
        }
    }


    //--------------------------------------------------------------------------
    // VSTable::getColumn reads the data
    //--------------------------------------------------------------------------
    m_VSTable->getColumn(colList,m_nCols,0,m_nRows-1, m_tableData);

    //delete table;
    return true;
}

void Pipe::destroyVTK()

{
    if ( m_pRenderer != 0 )
        m_pRenderer->Delete();
    if(m_pRenderWindow!=0)
        m_pRenderWindow->Delete();
    if ( m_lut!=0)
        m_lut->Delete() ;
}

void Pipe::saveImageAsPng(int num )

{

    int magnification=1;
    vtkWindowToImageFilter *w2i=vtkWindowToImageFilter::New();
    std::string path;
    std::string numero;
    std::stringstream ss;
    ss<<num;
    ss>>numero;
    std::string fileName;
    w2i->SetInput(m_pRenderWindow);
    w2i->SetScale(magnification,magnification);
    w2i->Update();


    this->destroyVTK();
    return ;

}


void Pipe::setCamera ()
{

    m_camera =m_pRenderer->GetActiveCamera();

    //m_camera->SetFocalPoint(0,0,0);
    m_camera->Azimuth(0.0);
    m_camera->Elevation(0.0);
    m_camera->Zoom (1);

    vtkwin->updateScene();

}

void Pipe::setBoundingBox ( vtkDataObject *data )
{

    qDebug()<<"setBoundingBox";

    vtkSmartPointer<vtkPoints> points =
            vtkSmartPointer<vtkPoints>::New();

    //qui
    points->InsertNextPoint(-15000, -10000, -200*scalingFactors);
    points->InsertNextPoint(15000, -10000, -200*scalingFactors);
    points->InsertNextPoint(-15000, 25000, -200*scalingFactors);
    points->InsertNextPoint(15000, 25000, -200*scalingFactors);

    points->InsertNextPoint(-15000, -10000, 200*scalingFactors);
    points->InsertNextPoint(15000, -10000, 200*scalingFactors);
    points->InsertNextPoint(-15000, 25000, 200*scalingFactors);
    points->InsertNextPoint(15000, 25000, 200*scalingFactors);



    // Create the polygon
    vtkSmartPointer<vtkPolygon> polygon =
            vtkSmartPointer<vtkPolygon>::New();
    polygon->GetPointIds()->SetNumberOfIds(8); //make a quad
    polygon->GetPointIds()->SetId(0, 0);
    polygon->GetPointIds()->SetId(1, 1);
    polygon->GetPointIds()->SetId(2, 2);
    polygon->GetPointIds()->SetId(3, 3);
    polygon->GetPointIds()->SetId(4, 4);
    polygon->GetPointIds()->SetId(5, 5);
    polygon->GetPointIds()->SetId(6, 6);
    polygon->GetPointIds()->SetId(7, 7);

    // Add the polygon to a list of polygons
    vtkSmartPointer<vtkCellArray> polygons =
            vtkSmartPointer<vtkCellArray>::New();
    polygons->InsertNextCell(polygon);

    // Create a PolyData
    vtkSmartPointer<vtkPolyData> polygonPolyData =
            vtkSmartPointer<vtkPolyData>::New();
    polygonPolyData->SetPoints(points);
    polygonPolyData->SetPolys(polygons);




    bool showBox=true;
    vtkOutlineCornerFilter*corner= vtkOutlineCornerFilter::New();
    // corner->SetInputData(data);
    corner->SetInputData(polygonPolyData);
    corner->ReleaseDataFlagOn();

    vtkPolyDataMapper *outlineMapper = vtkPolyDataMapper::New();
    //   outlineMapper->SetInput ( corner->GetOutput() );
    outlineMapper->SetInputConnection( corner->GetOutputPort() );

    vtkProperty *outlineProperty = vtkProperty::New();
    outlineProperty->SetColor ( 1,1,1 ); // Set the color to white
    outlineProperty->SetAmbient ( 1 );
    if(showBox)
        outlineProperty->SetOpacity ( 0.999 );
    else
        outlineProperty->SetOpacity ( 0.0 );
    outlineProperty->SetRepresentationToWireframe();
    outlineProperty->SetInterpolationToFlat();

    //vtkLODActor* outlineActor = vtkLODActor::New();
    outlineActor = vtkLODActor::New();
    outlineActor->SetMapper ( outlineMapper );
    outlineActor->SetProperty ( outlineProperty );
    outlineActor->SetPickable ( false );
    outlineActor->SetVisibility ( true );


    m_pRenderer->AddActor ( outlineActor );

    if (outlineActor!=0)
        outlineActor->Delete();
    if (outlineMapper!=0)
        outlineMapper->Delete();
    if (outlineProperty!=0)
        outlineProperty->Delete();
    if (corner!=0)
        corner->Delete();


    qDebug()<<"setBoundingBox - end";

}

/*
void Pipe::setBoundingBox ( vtkDataObject *data )
{

    qDebug()<<"*******************SETTO IL BOUNDING BOX";
    bool showBox=true;
    vtkOutlineCornerFilter*corner= vtkOutlineCornerFilter::New();
    corner->SetInput(data);
    corner->ReleaseDataFlagOn();

    vtkPolyDataMapper *outlineMapper = vtkPolyDataMapper::New();
    outlineMapper->SetInput ( corner->GetOutput() );

    vtkProperty *outlineProperty = vtkProperty::New();
    outlineProperty->SetColor ( 1,1,1 ); // Set the color to white
    outlineProperty->SetAmbient ( 1 );
    if(showBox)
        outlineProperty->SetOpacity ( 0.999 );
    else
        outlineProperty->SetOpacity ( 0.0 );
    outlineProperty->SetRepresentationToWireframe();
    outlineProperty->SetInterpolationToFlat();

    vtkLODActor* outlineActor = vtkLODActor::New();
    outlineActor->SetMapper ( outlineMapper );
    outlineActor->SetProperty ( outlineProperty );
    outlineActor->SetPickable ( false );
    outlineActor->SetVisibility ( true );


    qDebug()<<"******************* ADD ACTOR";

    m_pRenderer->AddActor ( outlineActor );

    if (outlineActor!=0)
        outlineActor->Delete();
    if (outlineMapper!=0)
        outlineMapper->Delete();
    if (outlineProperty!=0)
        outlineProperty->Delete();
    if (corner!=0)
        corner->Delete();
}

*/


void Pipe::colorBar (bool showColorBar)
{




    if (scalarBar != 0)
    {

        //scalarBar->Delete();
        m_pRenderer->RemoveActor( scalarBar );

    }


    scalarBar=vtkScalarBarActor::New();
    scalarBar->SetTitle ( colorScalar.c_str());

    scalarBar->SetLabelFormat ( "%.3g" );
    scalarBar->SetOrientationToHorizontal();
    scalarBar->SetPosition ( 0.1,0 );
    scalarBar->SetPosition2 ( 0.8,0.1 );
    scalarBar->SetLookupTable (m_lut );
    m_pRenderer->AddActor ( scalarBar );
    scalarBar->SetVisibility(showColorBar);

    /*
    if (scalarBar!=0)
        scalarBar->Delete();
*/
}

void Pipe::setAxes ( vtkDataSet *data, double *bounds  )
{

    double size = (bounds[5] - bounds[4] != 0 ? bounds[5] - bounds[4] : bounds[5]);
    scalingFactors = size * 0.1;
    scalingFactors = 1000/scalingFactors;
    scalingFactors=40;
    qDebug()<<"size: "<<size<<" "<<scalingFactors;
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->InsertNextPoint(-15000, -10000, -200*scalingFactors);
    points->InsertNextPoint(15000, -10000, -200*scalingFactors);
    points->InsertNextPoint(-15000, 25000, -200*scalingFactors);
    points->InsertNextPoint(15000, 25000, -200*scalingFactors);

    points->InsertNextPoint(-15000, -10000, 200*scalingFactors);
    points->InsertNextPoint(15000, -10000, 200*scalingFactors);
    points->InsertNextPoint(-15000, 25000, 200*scalingFactors);
    points->InsertNextPoint(15000, 25000, 200*scalingFactors);



    // Create the polygon
    vtkSmartPointer<vtkPolygon> polygon =
            vtkSmartPointer<vtkPolygon>::New();
    polygon->GetPointIds()->SetNumberOfIds(8); //make a quad
    polygon->GetPointIds()->SetId(0, 0);
    polygon->GetPointIds()->SetId(1, 1);
    polygon->GetPointIds()->SetId(2, 2);
    polygon->GetPointIds()->SetId(3, 3);
    polygon->GetPointIds()->SetId(4, 4);
    polygon->GetPointIds()->SetId(5, 5);
    polygon->GetPointIds()->SetId(6, 6);
    polygon->GetPointIds()->SetId(7, 7);

    // Add the polygon to a list of polygons
    vtkSmartPointer<vtkCellArray> polygons =
            vtkSmartPointer<vtkCellArray>::New();
    polygons->InsertNextCell(polygon);

    // Create a PolyData
    vtkSmartPointer<vtkPolyData> polygonPolyData =
            vtkSmartPointer<vtkPolyData>::New();
    polygonPolyData->SetPoints(points);
    polygonPolyData->SetPolys(polygons);






    axesActor=vtkCubeAxesActor2D::New();


    //axesActor->SetInputData ( data );
    axesActor->SetInputData ( polygonPolyData );


    axesActor->UseRangesOn();


    //FUNZIONANTI
    //axesActor->SetBounds ( bounds[0],bounds[1],bounds[2],bounds[3],bounds[4],bounds[5]);
    //axesActor->SetRanges (  bounds[0],bounds[1],bounds[2],bounds[3],bounds[4],bounds[5] );

    axesActor->SetBounds ( -15000, 15000, -10000,25000,-200*scalingFactors,200*scalingFactors);
    axesActor->SetRanges (  -15000,15000, -10000,25000,-200,200);


    axesActor->SetNumberOfLabels(6);

    axesActor->SetViewProp ( NULL );
    axesActor->SetScaling ( 0 );
    axesActor->SetPickable ( 0 );
    axesActor->SetCamera ( m_pRenderer->GetActiveCamera() );
    axesActor->SetCornerOffset ( 0.1 );

    //axesActor->SetLabelFormat ( "%6.5g" );

    axesActor->SetLabelFormat ( "%3.3g" );

    axesActor->SetInertia ( 100 );
    axesActor->SetFlyModeToOuterEdges();
    axesActor->SetVisibility ( true );

    axesActor->SetXLabel (vtkwin->vispoint->getX().toUtf8().constData() );
    axesActor->SetYLabel (vtkwin->vispoint->getY().toUtf8().constData() );
    axesActor->SetZLabel ( vtkwin->vispoint->getZ().toUtf8().constData());

    axesActor->Modified();

    m_pRenderer->AddActor2D ( axesActor );



    if(axesActor!=0)
        axesActor->Delete();
}
