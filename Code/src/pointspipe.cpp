/***************************************************************************
 *   Copyright (C) 2010 by Ugo Becciani, Alessandro Costa                  *
 *   ugo.becciani@oact.inaf.it                                         *
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


#include <cstdlib>
#include <cstring>

#include "pointspipe.h"

#include "visivoutilsdesktop.h"
#include "luteditor.h"

#include "extendedglyph3d.h"

#include <sstream>
#include <algorithm>

#include "vtkSphereSource.h"
#include "vtkConeSource.h"
#include "vtkCylinderSource.h"
#include "vtkCubeSource.h"

#include "vtkCamera.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkLookupTable.h"

#include "vtkFloatArray.h"
#include "vtkCellArray.h"
#include"vtkGlyph3D.h"
#include "vtkScalarBarActor.h"
#include "vtkOutlineCornerFilter.h"
#include "vtkProperty.h"

#include "vtkGenericRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkLODActor.h"
#include "vtkAxesActor.h"
#include "vtkUnstructuredGrid.h"

#include "vispoint.h"
#include "qdebug.h"

#include "vtkTransform.h"
#include "vtkSmartPointer.h"
#include "vtkTransformPolyDataFilter.h"

#include "vtkRectilinearGrid.h"
#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkPlaneSource.h"
#include "vtkIdFilter.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkStringArray.h"
#include "vtkwindow_new.h"

#include "math.h"

//---------------------------------------------------------------------
PointsPipe::PointsPipe (VSTableDesktop *table): Pipe(table)
  //---------------------------------------------------------------------
{
    //constructVTK();
    m_glyphFilter   = ExtendedGlyph3D::New();
    m_glyph         = vtkGlyph3D::New();
    m_pActor    = vtkLODActor::New();
    m_polyData      = vtkPolyData::New();
    m_pUnstructuredGrid = vtkUnstructuredGrid::New();
    m_pMapper   = vtkPolyDataMapper::New();

    m_VSTable = table;
}

//---------------------------------
PointsPipe::~PointsPipe()
//---------------------------------
{
    destroyVTK();
    if ( m_glyph!=0)
        m_glyph->Delete() ;
    if ( m_glyphFilter!=0)
        m_glyphFilter->Delete() ;
    if ( m_pMapper != 0 )
        m_pMapper->Delete();
    if ( m_pActor != 0 )
        m_pActor->Delete();
    if ( m_polyData!=0)
        m_polyData->Delete() ;
}
//---------------------------------
void PointsPipe::destroyAll()
//---------------------------------
{
    if ( m_glyph!=0)
        m_glyph->Delete() ;
    if ( m_glyphFilter!=0)
        m_glyphFilter->Delete() ;
    if ( m_pMapper != 0 )
        m_pMapper->Delete();
    if ( m_pActor != 0 )
        m_pActor->Delete();
    if ( m_polyData!=0)
        m_polyData->Delete() ;
}


void PointsPipe::setVtkWindow(vtkwindow_new *v)
{
    vtkwin=v;
    constructVTK(vtkwin);

}

int PointsPipe::createPipeFor3dSelection()
{

    int i = 0;
    int j = 0;

    radius=1;
    height=1;

    scaleGlyphs="none";
    radiusscalar="none";
    heightscalar="none";


    vtkFloatArray *radiusArrays =vtkFloatArray::New();

    vtkFloatArray *xAxis=vtkFloatArray::New();
    vtkFloatArray *yAxis=vtkFloatArray::New();
    vtkFloatArray *zAxis=vtkFloatArray::New();

    m_nRows= m_VSTable->getNumberOfRows();

    xAxis->SetNumberOfTuples(m_nRows);
    yAxis->SetNumberOfTuples(m_nRows);
    zAxis->SetNumberOfTuples(m_nRows);

    xAxis->SetName(vtkwin->vispoint->getX().toUtf8().constData());
    yAxis->SetName(vtkwin->vispoint->getY().toUtf8().constData());
    zAxis->SetName(vtkwin->vispoint->getZ().toUtf8().constData());

    int xIndex, yIndex,zIndex;

    xIndex= m_VSTable->getColId(xAxis->GetName());
    yIndex= m_VSTable->getColId(yAxis->GetName());
    zIndex= m_VSTable->getColId(zAxis->GetName());
    /*
     *     points->InsertNextPoint(-15000, -10000, -200*scalingFactors);
    points->InsertNextPoint(15000, -10000, -200*scalingFactors);
    points->InsertNextPoint(-15000, 25000, -200*scalingFactors);
    points->InsertNextPoint(15000, 25000, -200*scalingFactors);

    points->InsertNextPoint(-15000, -10000, 200*scalingFactors);
    points->InsertNextPoint(15000, -10000, 200*scalingFactors);
    points->InsertNextPoint(-15000, 25000, 200*scalingFactors);
    points->InsertNextPoint(15000, 25000, 200*scalingFactors);

    */
    double xval, yval, zval;
    for(i=0; i<m_nRows;i++)
    {
        xval = atof(m_VSTable->getTableData()[xIndex][i].c_str());
        yval = atof(m_VSTable->getTableData()[yIndex][i].c_str());
        zval = atof(m_VSTable->getTableData()[zIndex][i].c_str());

        if (xval> -15000 && xval<15000 && yval>-10000 && yval< 25000 && zval > -200 && zval < 200 )
        {
            xAxis->  SetValue(i,atof(m_VSTable->getTableData()[xIndex][i].c_str()));
            yAxis->  SetValue(i,atof(m_VSTable->getTableData()[yIndex][i].c_str()));
            zAxis->  SetValue(i,atof(m_VSTable->getTableData()[zIndex][i].c_str()));

        }

    }
    xAxis->GetRange(m_xRange);  //!minimum and maximum value
    yAxis->GetRange(m_yRange);  //!minimum and maximum value
    zAxis->GetRange(m_zRange);  //!minimum and maximum value

    qDebug()<<"m_xRange: "<<m_xRange[0]<<" "<<m_xRange[1]<<" ";


    SetXYZ(xAxis,yAxis,zAxis);



    m_polyData->SetPoints(m_points);
    m_pUnstructuredGrid->SetPoints(m_points);
    activateScale(true);

    int nPoints = m_polyData->GetNumberOfPoints();
    qDebug()<<"punti: "<<nPoints;

    vtkCellArray *newVerts = vtkCellArray::New();
    newVerts->EstimateSize ( nPoints, 1  );


    for ( int i = 0; i < nPoints; i++ )
    {
        //   labels->InsertNextValue("id_"+i);
        newVerts->InsertNextCell(1);
        newVerts->InsertCellPoint ( i );
    }
    m_polyData->SetVerts(newVerts);


    xAxis->Delete();
    yAxis->Delete();
    zAxis->Delete();
    m_points->Delete();

    float tmp[1];

    colorScalar = vtkwin->vispoint->getX().toUtf8().constData();
    color = "yes";
    palette = "AllWhite";
    showColorBar=false;


    //draw axis cartesian 0,0,0
    double p0[3] = {0.0, -10000, 0.0};
    double p1[3] = {0.0, 25000, 0.0};
    vtkSmartPointer<vtkLineSource> lineSource =
            vtkSmartPointer<vtkLineSource>::New();
    lineSource->SetPoint1(p0);
    lineSource->SetPoint2(p1);
    lineSource->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(lineSource->GetOutputPort());
    vtkSmartPointer<vtkActor> actorLine =
            vtkSmartPointer<vtkActor>::New();
    actorLine->SetMapper(mapper);
    actorLine->GetProperty()->SetLineWidth(1);
    m_pRenderer->AddActor ( actorLine );

    double p0_1[3] = {-15000, 0, 0.0};
    double p1_1[3] = {15000, 0, 0.0};
    vtkSmartPointer<vtkLineSource> lineSource_1 =
            vtkSmartPointer<vtkLineSource>::New();
    lineSource_1->SetPoint1(p0_1);
    lineSource_1->SetPoint2(p1_1);
    lineSource_1->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper_1 =
            vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper_1->SetInputConnection(lineSource_1->GetOutputPort());
    vtkSmartPointer<vtkActor> actorLine_1 =
            vtkSmartPointer<vtkActor>::New();
    actorLine_1->SetMapper(mapper_1);
    actorLine_1->GetProperty()->SetLineWidth(1);
    m_pRenderer->AddActor ( actorLine_1 );

    //set a symbol on 0,8400,0

    vtkSmartPointer<vtkSphereSource> sphereSource =
            vtkSmartPointer<vtkSphereSource>::New();
    double cone_center[3] = {0, 8400, 0};
    sphereSource->SetCenter(cone_center);
    sphereSource->SetRadius(100);
    sphereSource->Update();

    //Create a mapper and actor
    vtkSmartPointer<vtkPolyDataMapper> mapper_cone =
            vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper_cone->SetInputConnection(sphereSource->GetOutputPort());

    vtkSmartPointer<vtkActor> coneActor =
            vtkSmartPointer<vtkActor>::New();
    coneActor->SetMapper(mapper_cone);

    double color[3] ={1,0,0};
    coneActor->GetProperty()->SetColor(color);
    m_pRenderer->AddActor ( coneActor );
    /*

      double p0_center_point[3] = {0.0, 8398, 0.0};
      double p1_center_point[3] = {0.0, 8402, 0.0};
      vtkSmartPointer<vtkLineSource> lineSource_center_point =
        vtkSmartPointer<vtkLineSource>::New();
      lineSource_center_point->SetPoint1(p0_center_point);
      lineSource_center_point->SetPoint2(p1_center_point);
      lineSource_center_point->Update();

      vtkSmartPointer<vtkPolyDataMapper> mapper_center_point =
         vtkSmartPointer<vtkPolyDataMapper>::New();
       mapper_center_point->SetInputConnection(lineSource_center_point->GetOutputPort());
       vtkSmartPointer<vtkActor> actorLine_center_point =
         vtkSmartPointer<vtkActor>::New();
       actorLine_center_point->SetMapper(mapper_center_point);
       actorLine_center_point->GetProperty()->SetLineWidth(30);

       double color[3] ={1,0,0};
       actorLine_center_point->GetProperty()->SetColor(color);

       m_pRenderer->AddActor ( actorLine_center_point );


       double p0_2[3] = {-2, 8400, 0.0};
       double p1_2[3] = {2, 8400, 0.0};
       vtkSmartPointer<vtkLineSource> lineSource_center_point_2 =
         vtkSmartPointer<vtkLineSource>::New();
       lineSource_center_point_2->SetPoint1(p0_2);
       lineSource_center_point_2->SetPoint2(p1_2);
       lineSource_center_point_2->Update();

       vtkSmartPointer<vtkPolyDataMapper> mapper_center_point_2 =
          vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper_center_point_2->SetInputConnection(lineSource_center_point_2->GetOutputPort());
        vtkSmartPointer<vtkActor> actorLine_center_point_2 =
          vtkSmartPointer<vtkActor>::New();
        actorLine_center_point_2->SetMapper(mapper_center_point_2);
        actorLine_center_point_2->GetProperty()->SetLineWidth(30);

        actorLine_center_point_2->GetProperty()->SetColor(color);

        m_pRenderer->AddActor ( actorLine_center_point_2 );
*/



    // m_pMapper->SetInput (m_polyData );
    m_pMapper->SetInputData (m_polyData );

    m_pActor->SetMapper ( m_pMapper );
    m_pRenderer->AddActor ( m_pActor );

    //setGlyphs (1);


    float opacity=0.650; // a default value

    if (opacity<0 )
        opacity=0;

    else if( opacity>1)
        opacity=1;

    m_pActor->GetProperty()->SetOpacity ( opacity);

    //setLookupTable();
    setCamera ();
    qDebug()<<"fine camera";




    double *bounds;
    bounds=new double[6];
    bounds[0]=m_xRange[0];
    bounds[1]=m_xRange[1];
    bounds[2]=m_yRange[0];
    bounds[3]=m_yRange[1];
    bounds[4]=m_zRange[0];
    bounds[5]=m_zRange[1];

    bool showAxes=true;
    if(showAxes) setAxes (m_polyData,bounds );
    delete [] bounds;

    setBoundingBox (m_polyData);


    // m_pRenderWindow->Render();

    radiusArrays->Delete();


    return 0;


}


//-----------------------------------------------------------------------------------
int PointsPipe::createPipe ()
//------------------------------------------------------------------------------------
{
    int i = 0;
    int j = 0;

    vtkFloatArray *radiusArrays =vtkFloatArray::New();

    vtkFloatArray *xAxis=vtkFloatArray::New();
    vtkFloatArray *yAxis=vtkFloatArray::New();
    vtkFloatArray *zAxis=vtkFloatArray::New();

    xAxis->SetNumberOfTuples(m_nRows);
    yAxis->SetNumberOfTuples(m_nRows);
    zAxis->SetNumberOfTuples(m_nRows);

    qDebug()<<"post set tuple";

    xAxis->SetName(vtkwin->vispoint->getX().toUtf8().constData());
    yAxis->SetName(vtkwin->vispoint->getY().toUtf8().constData());
    zAxis->SetName(vtkwin->vispoint->getZ().toUtf8().constData());

    qDebug()<<"post set name"<<vtkwin->vispoint->getX();

    int xIndex, yIndex,zIndex;

    xIndex= m_colNames.find(xAxis->GetName())->second;
    yIndex= m_colNames.find(yAxis->GetName())->second;
    zIndex= m_colNames.find(zAxis->GetName())->second;

    qDebug()<<"index: "<<xIndex<<" "<<yIndex<<" "<<zIndex;


    for(i=0; i<m_nRows;i++)
    {
        xAxis->  SetValue(i,m_tableData[xIndex][i]); //! this is the row that fill xAxis array
        yAxis->  SetValue(i,m_tableData[yIndex][i]);
        zAxis->  SetValue(i,m_tableData[zIndex][i]);
    }


    xAxis->GetRange(m_xRange);  //!minimum and maximum value
    yAxis->GetRange(m_yRange);  //!minimum and maximum value
    zAxis->GetRange(m_zRange);  //!minimum and maximum value

    SetXYZ(xAxis,yAxis,zAxis);


    //m_polyData->GetPointData()->AddArray()

    m_polyData->SetPoints(m_points);
    m_pUnstructuredGrid->SetPoints(m_points);
    /*
    vtkSmartPointer<vtkIdFilter> idFilter = vtkSmartPointer<vtkIdFilter>::New();
    idFilter->SetInput(m_pUnstructuredGrid);
    idFilter->SetIdsArrayName("OriginalIds");//Deprecated method, replace with SetPointIdsArrayName or SetCellIdsArrayName
    idFilter->Update();
*/

    int nPoints = m_polyData->GetNumberOfPoints();
    qDebug()<<"punti: "<<nPoints;

    vtkCellArray *newVerts = vtkCellArray::New();
    newVerts->EstimateSize ( nPoints, 1  );

    /*
    vtkSmartPointer<vtkStringArray> labels = vtkSmartPointer<vtkStringArray>::New();
    labels->SetNumberOfValues(nPoints);
    labels->SetName("labels");

*/
    for ( int i = 0; i < nPoints; i++ )
    {
        //   labels->InsertNextValue("id_"+i);
        newVerts->InsertNextCell(1);
        newVerts->InsertCellPoint ( i );
    }
    m_polyData->SetVerts(newVerts);
    // m_polyData->GetPointData()->AddArray(labels);


    //START
    /*
    std::cout << "There are " << m_polyData->GetNumberOfPoints() << " points." << std::endl;
    std::cout << "There are " << m_polyData->GetNumberOfCells() << " cells." << std::endl;

    vtkSmartPointer<vtkIdFilter> idFilter =
            vtkSmartPointer<vtkIdFilter>::New();
    idFilter->SetInputConnection(m_polyData->GetProducerPort());
    idFilter->SetIdsArrayName("ids");//Deprecated method, replace with SetPointIdsArrayName or SetCellIdsArrayName
    idFilter->Update();

    vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
    surfaceFilter->SetInputConnection(idFilter->GetOutputPort());
    surfaceFilter->Update();

    //vtkPolyData* input = surfaceFilter->GetOutput();
    m_polyData=surfaceFilter->GetOutput();



    vtkIdTypeArray* pointIds = vtkIdTypeArray::SafeDownCast(idFilter->GetOutput()->GetPointData()->GetArray("ids"));
    std::cout << "There are " << pointIds->GetNumberOfTuples() << " point ids" << std::endl;

    vtkIdTypeArray* cellIds = vtkIdTypeArray::SafeDownCast(idFilter->GetOutput()->GetCellData()->GetArray("ids"));
    std::cout << "There are " << cellIds->GetNumberOfTuples() << " point ids" << std::endl;

    vtkIdTypeArray* ids = vtkIdTypeArray::SafeDownCast(m_polyData->GetPointData()->GetArray("ids"));
    for(vtkIdType i = 0; i < ids->GetNumberOfTuples(); i++)
    {
        std::cout << "Id " << i << " : " << ids->GetValue(i) << std::endl;
    }
*/
    //END


    xAxis->Delete();
    yAxis->Delete();
    zAxis->Delete();
    m_points->Delete();

    float tmp[1];

    colorScalar = vtkwin->vispoint->getX().toUtf8().constData();
    color = "yes";
    palette = "AllWhite";
    showColorBar=false;




    // m_pMapper->SetInput (m_polyData );
    m_pMapper->SetInputData (m_polyData );

    m_pActor->SetMapper ( m_pMapper );

    //
    // m_pActor->PickableOn();

    //


    m_pRenderer->AddActor ( m_pActor );

    float opacity=0.650; // a default value


    if (opacity<0 )
        opacity=0;

    else if( opacity>1)
        opacity=1;

    m_pActor->GetProperty()->SetOpacity ( opacity);



    setLookupTable();

    setCamera ();
    //qDebug()<<"fine camera";

    double *bounds;
    bounds=new double[6];
    bounds[0]=m_xRange[0];
    bounds[1]=m_xRange[1];
    bounds[2]=m_yRange[0];
    bounds[3]=m_yRange[1];
    bounds[4]=m_zRange[0];
    bounds[5]=m_zRange[1];

    bool showAxes=true;
    if(showAxes) setAxes (m_polyData,bounds );
    delete [] bounds;

    setBoundingBox (m_polyData);


    // m_pRenderWindow->Render();

    radiusArrays->Delete();
    return 0;
}

//-----------------------------------------------------------------------------------
vtkRenderer *PointsPipe::getRenderer ()
//-----------------------------------------------------------------------------------
{
    return m_pRenderer;
}

//---------------------------------------------------------------------
vtkLODActor *PointsPipe::getActor ()
//---------------------------------------------------------------------
{
    return m_pActor;
}

//---------------------------------------------------------------------
vtkPolyDataMapper *PointsPipe::getMapper ()
//---------------------------------------------------------------------
{
    return m_pMapper;
}

vtkUnstructuredGrid *PointsPipe::getUnstructuredGrid()
{
    return m_pUnstructuredGrid;
}
//---------------------------------------------------------------------
vtkPolyData *PointsPipe::getPolyData ()
//---------------------------------------------------------------------
{
    return m_polyData;
}

void PointsPipe::addScalar(std::string myScalar, bool color)
{
    vtkFloatArray *lutArrays =vtkFloatArray::New();
    vtkFloatArray *newArrays =vtkFloatArray::New();

    lutArrays->SetNumberOfTuples(m_nRows);

    qDebug()<<"m_nRows: "<<m_nRows;

    int ilut;
    std::map<std::string, int>::iterator p;
    ilut=m_VSTable->getColId(myScalar);
    qDebug()<<"pre  "<<QString::fromStdString(myScalar) <<" ilut: "<<ilut;



    for(int i=0; i<m_nRows;i++){
        double value = atof(m_VSTable->getTableData()[ilut][i].c_str());
        //if(value!=-9999){
        lutArrays -> SetValue(i, atof(m_VSTable->getTableData()[ilut][i].c_str()));
        //}
    }

    lutArrays->SetName(myScalar.c_str());
    
    if(!color){

        float *range= new float[2];
        lutArrays->GetValueRange(range,0);
        qDebug()<<"Range: "<<range[0]<<" "<<range[1];

        newArrays->SetNumberOfTuples(m_nRows);

        for(int i=0; i<m_nRows;i++){
            double value = lutArrays->GetValue(i);
            double new_value=(1*(value-range[0]))/(range[1]-range[0])+0.5;
            newArrays -> SetValue(i, new_value);
        }

        float *newrange= new float[2];
        newArrays->GetValueRange(newrange,0);
        qDebug()<<"New Range: "<<newrange[0]<<" "<<newrange[1];
        newArrays->SetName("scaleGlyph");

        m_polyData->GetPointData()->SetScalars(newArrays);
    }else{
        m_polyData->GetPointData()->AddArray(lutArrays);
    }
}





/*
void PointsPipe::initLut()
{
    this->addScalar(colorScalar);
}
*/

void PointsPipe::initLut()
{
    vtkFloatArray *lutArrays =vtkFloatArray::New();
    lutArrays->SetNumberOfTuples(m_nRows);

    qDebug()<<"m_nRows: "<<m_nRows;

    int ilut;
    std::map<std::string, int>::iterator p;
    ilut=m_VSTable->getColId(colorScalar);
    qDebug()<<"pre  "<<QString::fromStdString(colorScalar) <<" ilut: "<<ilut;



    for(int i=0; i<m_nRows;i++){
        double value = atof(m_VSTable->getTableData()[ilut][i].c_str());
        //if(value!=-9999){
        lutArrays -> SetValue(i, atof(m_VSTable->getTableData()[ilut][i].c_str()) );
        //}
    }

    lutArrays->SetName(colorScalar.c_str());
    m_polyData->GetPointData()->SetScalars(lutArrays);

}

void PointsPipe::setLookupTable ()
{


    initLut();

    double *b;
    b=new double[2];

    m_polyData->GetPointData()->SetActiveScalars(colorScalar.c_str());
    m_polyData->GetPointData()->GetScalars(colorScalar.c_str())->GetRange(b);

    setLookupTable (b[0], b[1]);


}

void PointsPipe::setActiveScalar()
{
    m_polyData->GetPointData()->SetActiveScalars(colorScalar.c_str());
    vtkwin->updateScene();
}

void PointsPipe::setLookupTableScale ()
{
    initLut();
    double *b;
    b=new double[2];
    
    m_polyData->GetPointData()->SetActiveScalars(colorScalar.c_str());
    m_polyData->GetPointData()->GetScalars(colorScalar.c_str())->GetRange(b);


    setLookupTable (b[0], b[1]);
    if(scale=="Linear")
        m_lut->SetScaleToLinear();
    else if (scale=="Log")
        m_lut->SetScaleToLog10();

    //    vtkwin->updateScene();

}

void PointsPipe::setLookupTable (float from, float to)
{

    qDebug()<<"from: "<<from<<" to: "<<to;
    actualFrom=from;
    actualTo=to;

    // initLut();

    m_lut->SetTableRange(from,to);
    //    m_lut->SetScaleToLinear();
    // m_lut->SetScaleToLog10();

    m_lut->Build();

    SelectLookTable(palette.c_str(),m_lut);

    qDebug()<<" "<<palette.c_str();

    m_pMapper->SetLookupTable(m_lut);
    m_pMapper->SetScalarVisibility(1);
    m_pMapper->UseLookupTableScalarRangeOn();
    m_pActor->SetMapper(m_pMapper);

    colorBar(showColorBar);


}

/*
//---------------------------------------------------------------------
    void PointsPipe::setRadius ()
//---------------------------------------------------------------------
{
  if (m_visOpt.nGlyphs==1)
    m_sphere->SetRadius ( m_visOpt.radius);


  else if (m_visOpt.nGlyphs==2)
  {

    m_cone->SetRadius ( m_visOpt.radius );
    m_cone->SetHeight (m_visOpt.height );
  }

  else if (m_visOpt.nGlyphs==3)
  {

    m_cylinder->SetRadius (m_visOpt.radius );
    m_cylinder->SetHeight ( m_visOpt.height );
  }
  else if (m_visOpt.nGlyphs==4)
  {
    m_cube->SetXLength ( m_visOpt.radius );
    m_cube->SetYLength ( m_visOpt.height );
    m_cube->SetZLength ( 1 );

  }
}

//---------------------------------------------------------------------
void PointsPipe::setResolution ()
//---------------------------------------------------------------------
{
  if (m_visOpt.nGlyphs==1)
  {
    m_sphere->SetPhiResolution ( 10 );
    m_sphere->SetThetaResolution ( 20 );
  }

  else if (m_visOpt.nGlyphs==2)
    m_cone->SetResolution ( 10 );

  else if (m_visOpt.nGlyphs==3)
    m_cylinder->SetResolution ( 10);


}

*/
//-------------------------------------------------------------------------
bool PointsPipe::SetXYZ(vtkFloatArray *xField, vtkFloatArray *yField, vtkFloatArray *zField  )
//-------------------------------------------------------------------------
{
    double scalingFactors[3];
    scalingFactors[0]=scalingFactors[1]=scalingFactors[2]=0;

    m_points=vtkPoints::New();
    m_points->SetNumberOfPoints(m_nRows);

    m_scaled_points=vtkPoints::New();
    m_scaled_points->SetNumberOfPoints(m_nRows);

    if(xField->GetNumberOfComponents() != yField->GetNumberOfComponents())
    {
        if(zField && (xField->GetNumberOfComponents() != zField->GetNumberOfComponents() \
                      || yField->GetNumberOfComponents() != zField->GetNumberOfComponents()))
        {
            false;
        }
        false; // component mismatch, do nothing
    }

    std::string scale = "no";
    // if(scale=="yes")
    // {

    double size = 0;


    size = (m_xRange[1] - m_xRange[0] != 0 ? m_xRange[1] - m_xRange[0] : m_xRange[1]);
    scalingFactors[0] = size * 0.1;


    size = (m_yRange[1] - m_yRange[0] != 0 ? m_yRange[1] - m_yRange[0] : m_yRange[1]);
    scalingFactors[1] = size * 0.1;

    qDebug()<<"m_zRange: "<< m_zRange[1]<<" - "<<m_zRange[0];

    size = (m_zRange[1] - m_zRange[0] != 0 ? m_zRange[1] - m_zRange[0] : m_zRange[1]);
    scalingFactors[2] = size * 0.1;
    //   }

    //double scalingFactorsInv[3];

    int i = 0;
    for(i = 0; i < 3; i++)
        scalingFactorsInv[i] = ((scalingFactors && scalingFactors[i] != 0) ? 1/scalingFactors[i] : 0);


    // scalingFactorsInv[2]*=1000;
    scalingFactorsInv[2]=40;
    // Set the points data

    //  if(scale=="yes")
    //  {
    for(i = 0; i < m_nRows; i++)
    {
        float inPoint[3];
        float outPoint[3];
        /*
        inPoint[0] = outPoint[0] = xField->GetValue(i) * scalingFactorsInv[0];
        inPoint[1] = outPoint[1] = yField->GetValue(i) * scalingFactorsInv[1];
        inPoint[2] = outPoint[2] = zField->GetValue(i) * scalingFactorsInv[2];
           */
        inPoint[0] = outPoint[0] = xField->GetValue(i) ;
        inPoint[1] = outPoint[1] = yField->GetValue(i) ;
        inPoint[2] = outPoint[2] = zField->GetValue(i) * scalingFactorsInv[2];

        m_scaled_points->SetPoint(i,outPoint);

        outPoint[0] = xField->GetValue(i) ;
        outPoint[1] = yField->GetValue(i) ;
        outPoint[2] = zField->GetValue(i) ;

        m_points->SetPoint(i,outPoint);


    }



    return true;
}

void PointsPipe::activateGrid(bool active)
{
    if(active)
    {

        int size_x_plan1 = m_xRange[1] - m_xRange[0];

        qDebug()<<"m_yRange[0]: "<<m_xRange[0]<<" scaled: "<<m_xRange[1]*scalingFactorsInv[0];
        qDebug()<<"m_yRange[1]: "<<m_yRange[1]<<" scaled: "<<m_yRange[1]*scalingFactorsInv[0];
        qDebug()<<"m_zRange[1]: "<<m_zRange[1]<<" scaled: "<<m_zRange[1]*scalingFactorsInv[0];
        qDebug()<<"size_x_plan1: "<<size_x_plan1;


        int num_element_x= ceil(size_x_plan1/5000.0);
        double step=size_x_plan1/(double)num_element_x;

        float *x= new float[num_element_x+2];
        for (int i=0; i<num_element_x+2; i++)
        {
            // x[i]= (m_xRange[0]+(5000*i)) *scalingFactorsInv[0];
            x[i]= (m_xRange[0]+(step*i)) *scalingFactorsInv[0];

            qDebug()<<"x["<<i<<"]="<<x[i];

        }

        int size_y_plan1 = m_yRange[1] - m_yRange[0];

        qDebug()<<"m_yRange[0]: "<<m_yRange[0]<<" scaled: "<<m_yRange[1]*scalingFactorsInv[1];
        qDebug()<<"m_yRange[1]: "<<m_yRange[1]<<" scaled: "<<m_yRange[1]*scalingFactorsInv[1];
        qDebug()<<"size_y_plan1: "<<size_y_plan1;
        int num_element_y= ceil(size_y_plan1/5000.0);

        qDebug()<<"num_element_y: "<<num_element_y;

        step= size_y_plan1/(double)num_element_y;

        qDebug()<<"step: "<<step;


        float *y= new float[num_element_y+1];
        for (int i=0; i<num_element_y+1; i++)
        {
            //y[i]=(m_yRange[0]+(5000*i))*scalingFactorsInv[1];
            y[i]=(m_yRange[0]+(step*i))*scalingFactorsInv[1];
            qDebug()<<"y["<<i<<"]="<<y[i];
        }


        int i;

        // Create a rectilinear grid by defining three arrays specifying the
        // coordinates in the x-y-z directions.
        vtkFloatArray *xCoords = vtkFloatArray::New();
        for (i=0; i<num_element_x+2; i++){

            qDebug()<<"x["<<i<<"]="<<x[i];

            xCoords->InsertNextValue(x[i]);
        }
        vtkFloatArray *yCoords = vtkFloatArray::New();
        for (i=0; i<num_element_y+1; i++) yCoords->InsertNextValue(y[i]);

        vtkFloatArray *zCoords = vtkFloatArray::New();
        zCoords->InsertNextValue(m_zRange[0]*scalingFactorsInv[2]);

        // The coordinates are assigned to the rectilinear grid. Make sure that
        // the number of values in each of the XCoordinates, YCoordinates,
        // and ZCoordinates is equal to what is defined in SetDimensions().
        //
        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
        rgrid->SetDimensions(num_element_x,num_element_y,1);
        rgrid->SetXCoordinates(xCoords);
        rgrid->SetYCoordinates(yCoords);
        rgrid->SetZCoordinates(zCoords);

        // Extract a plane from the grid to see what we've got.
        vtkRectilinearGridGeometryFilter *plane = vtkRectilinearGridGeometryFilter::New();
        // plane->SetInput(rgrid);
        plane->SetInputData(rgrid);
        plane->SetExtent (0,10000, 0,100000, 4,4);

        vtkPolyDataMapper *rgridMapper = vtkPolyDataMapper::New();
        rgridMapper->SetInputConnection(plane->GetOutputPort());

        vtkActor *wireActor = vtkActor::New();
        wireActor->SetMapper(rgridMapper);
        wireActor->GetProperty()->SetRepresentationToWireframe();
        wireActor->GetProperty()->SetColor(0,1,0);

        m_pRenderer->AddActor(wireActor);

        /*
        int size2_1 = m_xRange[1] - m_xRange[0];

        float *x2= new float[size2_1/1000];
        for (int i=0; i<size2_1/1000; i++)
        {
           // qDebug()<<"x["<<i<<"]="<<x2[i];
        }

        int size2_2 = m_yRange[1] - m_yRange[0];
        float *y2= new float[size2_2/1000];
        for (int i=0; i<size2_2/1000; i++)
        {
            y2[i]=(m_yRange[0]+(1000*i))*scalingFactorsInv[1];
         //   qDebug()<<"y["<<i<<"]="<<y2[i];
        }




        // Create a rectilinear grid by defining three arrays specifying the
        // coordinates in the x-y-z directions.
        vtkFloatArray *xCoords2 = vtkFloatArray::New();
          for (i=0; i<size2_1/1000; i++) xCoords2->InsertNextValue(x2[i]);

        vtkFloatArray *yCoords2 = vtkFloatArray::New();
        //for (i=0; i<33; i++) yCoords->InsertNextValue(y[i]);
         for (i=0; i<size2_2/1000; i++) yCoords2->InsertNextValue(y2[i]);

        vtkFloatArray *zCoords2 = vtkFloatArray::New();
        zCoords2->InsertNextValue(m_zRange[0]*scalingFactorsInv[2]);

        //qDebug()<<m_zRange[0]<<" sc:"<<scalingFactorsInv[2];

        // The coordinates are assigned to the rectilinear grid. Make sure that
        // the number of values in each of the XCoordinates, YCoordinates,
        // and ZCoordinates is equal to what is defined in SetDimensions().
        //
        vtkRectilinearGrid *rgrid2 = vtkRectilinearGrid::New();
        rgrid2->SetDimensions(size2_1/1000,size2_2/1000,1);
        rgrid2->SetXCoordinates(xCoords2);
        rgrid2->SetYCoordinates(yCoords2);
        rgrid2->SetZCoordinates(zCoords2);

        // Extract a plane from the grid to see what we've got.
        vtkRectilinearGridGeometryFilter *plane2 = vtkRectilinearGridGeometryFilter::New();
        plane2->SetInput(rgrid2);
        plane2->SetExtent (0,10000, 0,100000, 4,4);

        vtkPolyDataMapper *rgridMapper2 = vtkPolyDataMapper::New();
        rgridMapper2->SetInputConnection(plane2->GetOutputPort());

        vtkActor *wireActor2 = vtkActor::New();
        wireActor2->SetMapper(rgridMapper2);
        wireActor2->GetProperty()->SetRepresentationToWireframe();
        wireActor2->GetProperty()->SetColor(1,0,0);

        m_pRenderer->AddActor(wireActor2);
*/





    }
    vtkwin->updateScene();
}

//---------------------------------------------------------------------
void PointsPipe::activateScale (bool active)
//---------------------------------------------------------------------
{


    if(active)
        m_polyData->SetPoints(m_scaled_points);
    else
        m_polyData->SetPoints(m_points);

    isScaleActive=active;

    vtkwin->updateScene();


    /*
    for(vtkIdType i = 0; i < m_polyData->GetNumberOfPoints(); i++)
    {
        double outPoint[3];
        m_polyData->GetPoint(i,outPoint);

        outPoint[0]  =  outPoint[0] * scalingFactorsInv[0];
        outPoint[1]  = outPoint[0] * scalingFactorsInv[1];
        outPoint[2]  =outPoint[0] * scalingFactorsInv[2];

        m_polyData->SetPoint(outPoint);


    }
*/

    /*

    m_polyData->GetPoints()->


    m_polyData->SetPoints(m_points);
    m_pMapper->SetInput (m_polyData );
    m_pActor->SetMapper(m_pMapper);

    vtkwin->updateScene();

*/

}

void PointsPipe::setScaling ()
{
    m_glyphFilter->SetUseSecondScalar(true);
    m_glyphFilter->SetUseThirdScalar(true);

    m_glyphFilter->SetScaling(1);


    if( heightscalar!="none" && scaleGlyphs!="none" && nGlyphs!=0 && nGlyphs!=1)
        m_glyphFilter->SetInputScalarsSelectionY("x");
    //m_glyphFilter->SetInputScalarsSelectionY(m_visOpt.heightscalar.c_str());

    if( radiusscalar!="none" && scaleGlyphs!="none" && nGlyphs!=0)
        m_glyphFilter->SetInputScalarsSelectionXZ("y");
    // m_glyphFilter->SetInputScalarsSelectionXZ(m_visOpt.heightscalar.c_str());


    if( nGlyphs!=0)
        m_glyphFilter->SetScaleModeToScaleByScalar();
    else
        m_glyphFilter->ScalarVisibilityOff();


}

//---------------------------------------------------------------------
void PointsPipe::setGlyphs ( int n)
//---------------------------------------------------------------------
{
    int max=3000;

    qDebug()<<"setGlyphs type: "<<n<<" m_nRows: "<<m_nRows<<" max: "<<max;

    nGlyphs=n;
    if ( m_nRows<max )
    {
        qDebug()<<"set Input m_glyph";
        m_glyph->SetInputData(m_polyData );


        if (isScaleActive)
            m_glyph->SetScaleFactor ( 0.04 );
        else
            m_glyph->SetScaleFactor ( 2.5 );


        qDebug()<<"set Input m_pMapper";
        m_pMapper->SetInputConnection( m_glyph->GetOutputPort() );


        if (nGlyphs==1)
        {
            m_sphere   = vtkSmartPointer<vtkSphereSource>::New();
            setResolution ( );
            setRadius ();
            m_glyph->SetSourceData( m_sphere->GetOutput() );
            qDebug()<<"post m_sphere";

        }

        else if (nGlyphs==2)
        {
            m_cone   = vtkConeSource::New();
            setResolution ( );
            setRadius ();
            m_glyph->SetSourceData ( m_cone->GetOutput() );
            m_cone->Delete();
        }

        else if (nGlyphs==3)
        {
            m_cylinder   = vtkCylinderSource::New();
            setResolution ( );
            setRadius ();
            m_glyph->SetSourceData ( m_cylinder->GetOutput() );
            m_cylinder->Delete();
        }

        else if (nGlyphs==4)
        {
            m_cube   = vtkCubeSource::New();
            setRadius ();
            m_glyph->SetSourceData ( m_cube->GetOutput() );
            m_cube->Delete();
        }


    }
}

//---------------------------------------------------------------------
void PointsPipe::setRadius ()
//---------------------------------------------------------------------
{
    qDebug()<<"setRadius";

    if (nGlyphs==1)
    {
        m_sphere->SetRadius ( radius);
        qDebug()<<"post radius: "<<radius;

    }

    else if (nGlyphs==2)
    {
        m_cone->SetRadius ( radius );
        m_cone->SetHeight (height );
    }

    else if (nGlyphs==3)
    {
        m_cylinder->SetRadius (radius );
        m_cylinder->SetHeight ( height );
    }
    else if (nGlyphs==4)
    {
        m_cube->SetXLength ( radius );
        m_cube->SetYLength ( height );
        m_cube->SetZLength ( 1 );

    }
}

//---------------------------------------------------------------------
void PointsPipe::setResolution ()
//---------------------------------------------------------------------
{
    qDebug()<<"Set Res";

    if (nGlyphs==1)
    {
        m_sphere->SetPhiResolution ( 10 );
        m_sphere->SetThetaResolution ( 20 );
        qDebug()<<"post res";

    }

    else if (nGlyphs==2)
        m_cone->SetResolution ( 10 );

    else if (nGlyphs==3)
        m_cylinder->SetResolution ( 10);


}



