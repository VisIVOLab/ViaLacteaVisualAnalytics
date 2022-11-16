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

#include "luteditor.h"
#include "visivoutilsdesktop.h"

#include "extendedglyph3d.h"

#include <algorithm>
#include <sstream>

#include "vtkConeSource.h"
#include "vtkCubeSource.h"
#include "vtkCylinderSource.h"
#include "vtkSphereSource.h"

#include "vtkCamera.h"
#include "vtkCellData.h"
#include "vtkLookupTable.h"
#include "vtkPointData.h"

#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkGlyph3D.h"
#include "vtkOutlineCornerFilter.h"
#include "vtkProperty.h"
#include "vtkScalarBarActor.h"

#include "vtkAxesActor.h"
#include "vtkGenericRenderWindowInteractor.h"
#include "vtkLODActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkUnstructuredGrid.h"

#include "qdebug.h"
#include "vispoint.h"

#include "vtkSmartPointer.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"

#include "vtkDataSetSurfaceFilter.h"
#include "vtkIdFilter.h"
#include "vtkPlaneSource.h"
#include "vtkRectilinearGrid.h"
#include "vtkRectilinearGridGeometryFilter.h"
#include "vtkStringArray.h"
#include "vtkwindow_new.h"

#include "math.h"

//---------------------------------------------------------------------
PointsPipe::PointsPipe(VSTableDesktop *table) : Pipe(table)
  //---------------------------------------------------------------------
{
    // constructVTK();
    m_glyphFilter = ExtendedGlyph3D::New();
    m_glyph = vtkGlyph3D::New();
    m_pActor = vtkLODActor::New();
    m_polyData = vtkPolyData::New();
    m_pUnstructuredGrid = vtkUnstructuredGrid::New();
    m_pMapper = vtkPolyDataMapper::New();

    m_VSTable = table;
}

//---------------------------------
PointsPipe::~PointsPipe()
//---------------------------------
{
    destroyVTK();
    if (m_glyph != 0)
        m_glyph->Delete();
    if (m_glyphFilter != 0)
        m_glyphFilter->Delete();
    if (m_pMapper != 0)
        m_pMapper->Delete();
    if (m_pActor != 0)
        m_pActor->Delete();
    if (m_polyData != 0)
        m_polyData->Delete();
}
//---------------------------------
void PointsPipe::destroyAll()
//---------------------------------
{
    if (m_glyph != 0)
        m_glyph->Delete();
    if (m_glyphFilter != 0)
        m_glyphFilter->Delete();
    if (m_pMapper != 0)
        m_pMapper->Delete();
    if (m_pActor != 0)
        m_pActor->Delete();
    if (m_polyData != 0)
        m_polyData->Delete();
}

void PointsPipe::setVtkWindow(vtkwindow_new *v)
{
    vtkwin = v;
    constructVTK(vtkwin);
}

int PointsPipe::createPipeFor3dSelection()
{
    int i = 0;
    int j = 0;

    radius = 1;
    height = 1;

    scaleGlyphs = "none";
    radiusscalar = "none";
    heightscalar = "none";

    vtkFloatArray *radiusArrays = vtkFloatArray::New();

    vtkFloatArray *xAxis = vtkFloatArray::New();
    vtkFloatArray *yAxis = vtkFloatArray::New();
    vtkFloatArray *zAxis = vtkFloatArray::New();

    m_nRows = m_VSTable->getNumberOfRows();

    xAxis->SetNumberOfTuples(m_nRows);
    yAxis->SetNumberOfTuples(m_nRows);
    zAxis->SetNumberOfTuples(m_nRows);

    xAxis->SetName(vtkwin->vispoint->getX().toUtf8().constData());
    yAxis->SetName(vtkwin->vispoint->getY().toUtf8().constData());
    zAxis->SetName(vtkwin->vispoint->getZ().toUtf8().constData());

    int xIndex, yIndex, zIndex;

    xIndex = m_VSTable->getColId(xAxis->GetName());
    yIndex = m_VSTable->getColId(yAxis->GetName());
    zIndex = m_VSTable->getColId(zAxis->GetName());

    double xval, yval, zval;
    for (i = 0; i < m_nRows; i++) {
        xval = atof(m_VSTable->getTableData()[xIndex][i].c_str());
        yval = atof(m_VSTable->getTableData()[yIndex][i].c_str());
        zval = atof(m_VSTable->getTableData()[zIndex][i].c_str());

        if (xval > -15000 && xval < 15000 && yval > -10000 && yval < 25000 && zval > -200
                && zval < 200) {
            xAxis->SetValue(i, atof(m_VSTable->getTableData()[xIndex][i].c_str()));
            yAxis->SetValue(i, atof(m_VSTable->getTableData()[yIndex][i].c_str()));
            zAxis->SetValue(i, atof(m_VSTable->getTableData()[zIndex][i].c_str()));
        }
    }
    xAxis->GetRange(m_xRange); //!minimum and maximum value
    yAxis->GetRange(m_yRange); //!minimum and maximum value
    zAxis->GetRange(m_zRange); //!minimum and maximum value
    SetXYZ(xAxis, yAxis, zAxis);

    m_polyData->SetPoints(m_points);
    m_pUnstructuredGrid->SetPoints(m_points);
    activateScale(true);

    int nPoints = m_polyData->GetNumberOfPoints();

    vtkCellArray *newVerts = vtkCellArray::New();
    newVerts->EstimateSize(nPoints, 1);

    for (int i = 0; i < nPoints; i++) {
        newVerts->InsertNextCell(1);
        newVerts->InsertCellPoint(i);
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
    showColorBar = false;

    // draw axis cartesian 0,0,0
    double p0[3] = { 0.0, -10000, 0.0 };
    double p1[3] = { 0.0, 25000, 0.0 };
    vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();
    lineSource->SetPoint1(p0);
    lineSource->SetPoint2(p1);
    lineSource->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(lineSource->GetOutputPort());
    vtkSmartPointer<vtkActor> actorLine = vtkSmartPointer<vtkActor>::New();
    actorLine->SetMapper(mapper);
    actorLine->GetProperty()->SetLineWidth(1);
    m_pRenderer->AddActor(actorLine);

    double p0_1[3] = { -15000, 0, 0.0 };
    double p1_1[3] = { 15000, 0, 0.0 };
    vtkSmartPointer<vtkLineSource> lineSource_1 = vtkSmartPointer<vtkLineSource>::New();
    lineSource_1->SetPoint1(p0_1);
    lineSource_1->SetPoint2(p1_1);
    lineSource_1->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper_1 = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper_1->SetInputConnection(lineSource_1->GetOutputPort());
    vtkSmartPointer<vtkActor> actorLine_1 = vtkSmartPointer<vtkActor>::New();
    actorLine_1->SetMapper(mapper_1);
    actorLine_1->GetProperty()->SetLineWidth(1);
    m_pRenderer->AddActor(actorLine_1);

    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    double cone_center[3] = { 0, 8400, 0 };
    sphereSource->SetCenter(cone_center);
    sphereSource->SetRadius(100);
    sphereSource->Update();

    // Create a mapper and actor
    vtkSmartPointer<vtkPolyDataMapper> mapper_cone = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper_cone->SetInputConnection(sphereSource->GetOutputPort());

    vtkSmartPointer<vtkActor> coneActor = vtkSmartPointer<vtkActor>::New();
    coneActor->SetMapper(mapper_cone);

    double color[3] = { 1, 0, 0 };
    coneActor->GetProperty()->SetColor(color);
    m_pRenderer->AddActor(coneActor);
    m_pMapper->SetInputData(m_polyData);

    m_pActor->SetMapper(m_pMapper);
    m_pRenderer->AddActor(m_pActor);

    float opacity = 0.650; // a default value

    if (opacity < 0)
        opacity = 0;

    else if (opacity > 1)
        opacity = 1;

    m_pActor->GetProperty()->SetOpacity(opacity);

    setCamera();

    double *bounds;
    bounds = new double[6];
    bounds[0] = m_xRange[0];
    bounds[1] = m_xRange[1];
    bounds[2] = m_yRange[0];
    bounds[3] = m_yRange[1];
    bounds[4] = m_zRange[0];
    bounds[5] = m_zRange[1];

    bool showAxes = true;
    if (showAxes)
        setAxes(m_polyData, bounds);
    delete[] bounds;

    setBoundingBox(m_polyData);
    radiusArrays->Delete();

    return 0;
}

//-----------------------------------------------------------------------------------
int PointsPipe::createPipe()
//------------------------------------------------------------------------------------
{
    int i = 0;
    int j = 0;

    vtkFloatArray *radiusArrays = vtkFloatArray::New();

    vtkFloatArray *xAxis = vtkFloatArray::New();
    vtkFloatArray *yAxis = vtkFloatArray::New();
    vtkFloatArray *zAxis = vtkFloatArray::New();

    xAxis->SetNumberOfTuples(m_nRows);
    yAxis->SetNumberOfTuples(m_nRows);
    zAxis->SetNumberOfTuples(m_nRows);

    xAxis->SetName(vtkwin->vispoint->getX().toUtf8().constData());
    yAxis->SetName(vtkwin->vispoint->getY().toUtf8().constData());
    zAxis->SetName(vtkwin->vispoint->getZ().toUtf8().constData());

    int xIndex, yIndex, zIndex;

    xIndex = m_colNames.find(xAxis->GetName())->second;
    yIndex = m_colNames.find(yAxis->GetName())->second;
    zIndex = m_colNames.find(zAxis->GetName())->second;

    for (i = 0; i < m_nRows; i++) {
        xAxis->SetValue(i, m_tableData[xIndex][i]); //! this is the row that fill xAxis array
        yAxis->SetValue(i, m_tableData[yIndex][i]);
        zAxis->SetValue(i, m_tableData[zIndex][i]);
    }

    xAxis->GetRange(m_xRange); //!minimum and maximum value
    yAxis->GetRange(m_yRange); //!minimum and maximum value
    zAxis->GetRange(m_zRange); //!minimum and maximum value

    SetXYZ(xAxis, yAxis, zAxis);

    m_polyData->SetPoints(m_points);
    m_pUnstructuredGrid->SetPoints(m_points);

    int nPoints = m_polyData->GetNumberOfPoints();
    vtkCellArray *newVerts = vtkCellArray::New();
    newVerts->EstimateSize(nPoints, 1);

    for (int i = 0; i < nPoints; i++) {
        newVerts->InsertNextCell(1);
        newVerts->InsertCellPoint(i);
    }
    m_polyData->SetVerts(newVerts);

    xAxis->Delete();
    yAxis->Delete();
    zAxis->Delete();
    m_points->Delete();

    colorScalar = vtkwin->vispoint->getX().toUtf8().constData();
    color = "yes";
    palette = "AllWhite";
    showColorBar = false;
    m_pMapper->SetInputData(m_polyData);
    m_pActor->SetMapper(m_pMapper);
    m_pRenderer->AddActor(m_pActor);

    float opacity = 0.650; // a default value

    if (opacity < 0)
        opacity = 0;

    else if (opacity > 1)
        opacity = 1;

    m_pActor->GetProperty()->SetOpacity(opacity);
    setLookupTable();
    setCamera();

    double *bounds;
    bounds = new double[6];
    bounds[0] = m_xRange[0];
    bounds[1] = m_xRange[1];
    bounds[2] = m_yRange[0];
    bounds[3] = m_yRange[1];
    bounds[4] = m_zRange[0];
    bounds[5] = m_zRange[1];

    bool showAxes = true;
    if (showAxes)
        setAxes(m_polyData, bounds);
    delete[] bounds;

    setBoundingBox(m_polyData);

    radiusArrays->Delete();
    return 0;
}

//-----------------------------------------------------------------------------------
vtkRenderer *PointsPipe::getRenderer()
//-----------------------------------------------------------------------------------
{
    return m_pRenderer;
}

//---------------------------------------------------------------------
vtkLODActor *PointsPipe::getActor()
//---------------------------------------------------------------------
{
    return m_pActor;
}

//---------------------------------------------------------------------
vtkPolyDataMapper *PointsPipe::getMapper()
//---------------------------------------------------------------------
{
    return m_pMapper;
}

vtkUnstructuredGrid *PointsPipe::getUnstructuredGrid()
{
    return m_pUnstructuredGrid;
}
//---------------------------------------------------------------------
vtkPolyData *PointsPipe::getPolyData()
//---------------------------------------------------------------------
{
    return m_polyData;
}

void PointsPipe::addScalar(std::string myScalar, bool color)
{
    vtkFloatArray *lutArrays = vtkFloatArray::New();
    vtkFloatArray *newArrays = vtkFloatArray::New();

    lutArrays->SetNumberOfTuples(m_nRows);

    int ilut;
    std::map<std::string, int>::iterator p;
    ilut = m_VSTable->getColId(myScalar);

    for (int i = 0; i < m_nRows; i++) {
        double value = atof(m_VSTable->getTableData()[ilut][i].c_str());
        lutArrays->SetValue(i, atof(m_VSTable->getTableData()[ilut][i].c_str()));
    }

    lutArrays->SetName(myScalar.c_str());
    if (!color) {
        float *range = new float[2];
        lutArrays->GetValueRange(range, 0);
        newArrays->SetNumberOfTuples(m_nRows);

        for (int i = 0; i < m_nRows; i++) {
            double value = lutArrays->GetValue(i);
            double new_value = (1 * (value - range[0])) / (range[1] - range[0]) + 0.5;
            newArrays->SetValue(i, new_value);
        }

        float *newrange = new float[2];
        newArrays->GetValueRange(newrange, 0);
        newArrays->SetName("scaleGlyph");

        m_polyData->GetPointData()->SetScalars(newArrays);
    } else {
        m_polyData->GetPointData()->AddArray(lutArrays);
    }
}

void PointsPipe::initLut()
{
    vtkFloatArray *lutArrays = vtkFloatArray::New();
    lutArrays->SetNumberOfTuples(m_nRows);
    int ilut;
    std::map<std::string, int>::iterator p;
    ilut = m_VSTable->getColId(colorScalar);

    for (int i = 0; i < m_nRows; i++) {
        double value = atof(m_VSTable->getTableData()[ilut][i].c_str());
        lutArrays->SetValue(i, atof(m_VSTable->getTableData()[ilut][i].c_str()));
    }

    lutArrays->SetName(colorScalar.c_str());
    m_polyData->GetPointData()->SetScalars(lutArrays);
}

void PointsPipe::setLookupTable()
{
    initLut();

    double *b;
    b = new double[2];

    m_polyData->GetPointData()->SetActiveScalars(colorScalar.c_str());
    m_polyData->GetPointData()->GetScalars(colorScalar.c_str())->GetRange(b);

    setLookupTable(b[0], b[1]);
}

void PointsPipe::setActiveScalar()
{
    m_polyData->GetPointData()->SetActiveScalars(colorScalar.c_str());
    vtkwin->updateScene();
}

void PointsPipe::setLookupTableScale()
{
    initLut();
    double *b;
    b = new double[2];

    m_polyData->GetPointData()->SetActiveScalars(colorScalar.c_str());
    m_polyData->GetPointData()->GetScalars(colorScalar.c_str())->GetRange(b);

    setLookupTable(b[0], b[1]);
    if (scale == "Linear")
        m_lut->SetScaleToLinear();
    else if (scale == "Log")
        m_lut->SetScaleToLog10();
}

void PointsPipe::setLookupTable(float from, float to)
{

    actualFrom = from;
    actualTo = to;
    m_lut->SetTableRange(from, to);

    m_lut->Build();

    SelectLookTable(palette.c_str(), m_lut);
    m_pMapper->SetLookupTable(m_lut);
    m_pMapper->SetScalarVisibility(1);
    m_pMapper->UseLookupTableScalarRangeOn();
    m_pActor->SetMapper(m_pMapper);

    colorBar(showColorBar);
}

//-------------------------------------------------------------------------
bool PointsPipe::SetXYZ(vtkFloatArray *xField, vtkFloatArray *yField, vtkFloatArray *zField)
//-------------------------------------------------------------------------
{
    double scalingFactors[3];
    scalingFactors[0] = scalingFactors[1] = scalingFactors[2] = 0;

    m_points = vtkPoints::New();
    m_points->SetNumberOfPoints(m_nRows);

    m_scaled_points = vtkPoints::New();
    m_scaled_points->SetNumberOfPoints(m_nRows);

    if (xField->GetNumberOfComponents() != yField->GetNumberOfComponents()) {
        if (zField
                && (xField->GetNumberOfComponents() != zField->GetNumberOfComponents()
                    || yField->GetNumberOfComponents() != zField->GetNumberOfComponents())) {
            false;
        }
        false; // component mismatch, do nothing
    }

    std::string scale = "no";
    double size = 0;

    size = (m_xRange[1] - m_xRange[0] != 0 ? m_xRange[1] - m_xRange[0] : m_xRange[1]);
    scalingFactors[0] = size * 0.1;
    size = (m_yRange[1] - m_yRange[0] != 0 ? m_yRange[1] - m_yRange[0] : m_yRange[1]);
    scalingFactors[1] = size * 0.1;
    size = (m_zRange[1] - m_zRange[0] != 0 ? m_zRange[1] - m_zRange[0] : m_zRange[1]);
    scalingFactors[2] = size * 0.1;
    int i = 0;
    for (i = 0; i < 3; i++)
        scalingFactorsInv[i] =
                ((scalingFactors && scalingFactors[i] != 0) ? 1 / scalingFactors[i] : 0);

    scalingFactorsInv[2] = 40;

    for (i = 0; i < m_nRows; i++) {
        float inPoint[3];
        float outPoint[3];

        inPoint[0] = outPoint[0] = xField->GetValue(i);
        inPoint[1] = outPoint[1] = yField->GetValue(i);
        inPoint[2] = outPoint[2] = zField->GetValue(i) * scalingFactorsInv[2];

        m_scaled_points->SetPoint(i, outPoint);

        outPoint[0] = xField->GetValue(i);
        outPoint[1] = yField->GetValue(i);
        outPoint[2] = zField->GetValue(i);

        m_points->SetPoint(i, outPoint);
    }

    return true;
}

void PointsPipe::activateGrid(bool active)
{
    if (active) {

        int size_x_plan1 = m_xRange[1] - m_xRange[0];

        int num_element_x = ceil(size_x_plan1 / 5000.0);
        double step = size_x_plan1 / (double)num_element_x;

        float *x = new float[num_element_x + 2];
        for (int i = 0; i < num_element_x + 2; i++) {
            x[i] = (m_xRange[0] + (step * i)) * scalingFactorsInv[0];
        }

        int size_y_plan1 = m_yRange[1] - m_yRange[0];
        int num_element_y = ceil(size_y_plan1 / 5000.0);
        step = size_y_plan1 / (double)num_element_y;
        float *y = new float[num_element_y + 1];
        for (int i = 0; i < num_element_y + 1; i++) {
            y[i] = (m_yRange[0] + (step * i)) * scalingFactorsInv[1];
        }

        int i;
        // Create a rectilinear grid by defining three arrays specifying the
        // coordinates in the x-y-z directions.
        vtkFloatArray *xCoords = vtkFloatArray::New();
        for (i = 0; i < num_element_x + 2; i++) {
            xCoords->InsertNextValue(x[i]);
        }
        vtkFloatArray *yCoords = vtkFloatArray::New();
        for (i = 0; i < num_element_y + 1; i++)
            yCoords->InsertNextValue(y[i]);

        vtkFloatArray *zCoords = vtkFloatArray::New();
        zCoords->InsertNextValue(m_zRange[0] * scalingFactorsInv[2]);

        // The coordinates are assigned to the rectilinear grid. Make sure that
        // the number of values in each of the XCoordinates, YCoordinates,
        // and ZCoordinates is equal to what is defined in SetDimensions().
        //
        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
        rgrid->SetDimensions(num_element_x, num_element_y, 1);
        rgrid->SetXCoordinates(xCoords);
        rgrid->SetYCoordinates(yCoords);
        rgrid->SetZCoordinates(zCoords);

        // Extract a plane from the grid to see what we've got.
        vtkRectilinearGridGeometryFilter *plane = vtkRectilinearGridGeometryFilter::New();
        plane->SetInputData(rgrid);
        plane->SetExtent(0, 10000, 0, 100000, 4, 4);

        vtkPolyDataMapper *rgridMapper = vtkPolyDataMapper::New();
        rgridMapper->SetInputConnection(plane->GetOutputPort());

        vtkActor *wireActor = vtkActor::New();
        wireActor->SetMapper(rgridMapper);
        wireActor->GetProperty()->SetRepresentationToWireframe();
        wireActor->GetProperty()->SetColor(0, 1, 0);

        m_pRenderer->AddActor(wireActor);
    }
    vtkwin->updateScene();
}

//---------------------------------------------------------------------
void PointsPipe::activateScale(bool active)
//---------------------------------------------------------------------
{

    if (active)
        m_polyData->SetPoints(m_scaled_points);
    else
        m_polyData->SetPoints(m_points);

    isScaleActive = active;
    vtkwin->updateScene();
}

void PointsPipe::setScaling()
{
    m_glyphFilter->SetUseSecondScalar(true);
    m_glyphFilter->SetUseThirdScalar(true);

    m_glyphFilter->SetScaling(1);

    if (heightscalar != "none" && scaleGlyphs != "none" && nGlyphs != 0 && nGlyphs != 1)
        m_glyphFilter->SetInputScalarsSelectionY("x");

    if (radiusscalar != "none" && scaleGlyphs != "none" && nGlyphs != 0)
        m_glyphFilter->SetInputScalarsSelectionXZ("y");

    if (nGlyphs != 0)
        m_glyphFilter->SetScaleModeToScaleByScalar();
    else
        m_glyphFilter->ScalarVisibilityOff();
}

//---------------------------------------------------------------------
void PointsPipe::setGlyphs(int n)
//---------------------------------------------------------------------
{
    int max = 3000;

    nGlyphs = n;
    if (m_nRows < max) {
        m_glyph->SetInputData(m_polyData);

        if (isScaleActive)
            m_glyph->SetScaleFactor(0.04);
        else
            m_glyph->SetScaleFactor(2.5);

        m_pMapper->SetInputConnection(m_glyph->GetOutputPort());

        if (nGlyphs == 1) {
            m_sphere = vtkSmartPointer<vtkSphereSource>::New();
            setResolution();
            setRadius();
            m_glyph->SetSourceData(m_sphere->GetOutput());
        }

        else if (nGlyphs == 2) {
            m_cone = vtkConeSource::New();
            setResolution();
            setRadius();
            m_glyph->SetSourceData(m_cone->GetOutput());
            m_cone->Delete();
        }

        else if (nGlyphs == 3) {
            m_cylinder = vtkCylinderSource::New();
            setResolution();
            setRadius();
            m_glyph->SetSourceData(m_cylinder->GetOutput());
            m_cylinder->Delete();
        }

        else if (nGlyphs == 4) {
            m_cube = vtkCubeSource::New();
            setRadius();
            m_glyph->SetSourceData(m_cube->GetOutput());
            m_cube->Delete();
        }
    }
}

//---------------------------------------------------------------------
void PointsPipe::setRadius()
//---------------------------------------------------------------------
{
    if (nGlyphs == 1) {
        m_sphere->SetRadius(radius);
    }

    else if (nGlyphs == 2) {
        m_cone->SetRadius(radius);
        m_cone->SetHeight(height);
    }

    else if (nGlyphs == 3) {
        m_cylinder->SetRadius(radius);
        m_cylinder->SetHeight(height);
    } else if (nGlyphs == 4) {
        m_cube->SetXLength(radius);
        m_cube->SetYLength(height);
        m_cube->SetZLength(1);
    }
}

//---------------------------------------------------------------------
void PointsPipe::setResolution()
//---------------------------------------------------------------------
{

    if (nGlyphs == 1) {
        m_sphere->SetPhiResolution(10);
        m_sphere->SetThetaResolution(20);
    }

    else if (nGlyphs == 2)
        m_cone->SetResolution(10);

    else if (nGlyphs == 3)
        m_cylinder->SetResolution(10);
}
