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

#ifndef POINTSPIPE_H
#define POINTSPIPE_H

/**
        @author Alessandro Costa <alessandro.costa@oact.inaf.it>
        @author Ugo Becciani <ugo.becciani@oact.inaf.it>
*/

#include "pipe.h"
//#include "vtkwindow.h"

//#include "optionssetter.h"
#include "vtkSmartPointer.h"

class vtkLODActor;
class vtkSphereSource;
class vtkConeSource;
class vtkCylinderSource;
class vtkCubeSource;
class vtkGlyph3D;
class vtkPoints;
class vtkUnstructuredGrid;
class ExtendedGlyph3D;
class vtkwindow_new;



// Define a new frame type: this is going to be our main frame
class PointsPipe: public Pipe
{
public:
    PointsPipe(VSTableDesktop *table);

    vtkRenderer *getRenderer();
    vtkLODActor *getActor();
    vtkPolyData *getPolyData();
    vtkPolyDataMapper *getMapper();
    vtkUnstructuredGrid *getUnstructuredGrid();
    ~PointsPipe();
    int createPipe();
    int createPipeFor3dSelection();
    void setGlyphs (int nGlyphs);

    void setVtkWindow(vtkwindow_new *v);
    std::string color;
    std::string palette;
    std::string scale;
    bool showColorBar;
    void setLookupTable( );
    void setLookupTable (float from, float to );
    void setLookupTableScale();
    void setActiveScalar();
    void initLut();
    void addScalar(std::string myScalar, bool color);
    void activateScale (bool active);
    void activateGrid (bool active);
    double actualFrom;
    double actualTo;

  /*
    vtkFloatArray *xAxis;
    vtkFloatArray *yAxis;
    vtkFloatArray *zAxis;
*/
//  bool SetXYZ(vtkFloatArray *xField, vtkFloatArray *yField, vtkFloatArray *zField, bool scale=false  );
    bool SetXYZ(vtkFloatArray *xField, vtkFloatArray *yField, vtkFloatArray *zField  );
    void setScaling();
protected:
    void destroyAll();

private:
    void setRadius ();
    void setResolution ();
   // void setScaling ();


    VSTableDesktop *m_VSTable;
    vtkPolyDataMapper   *m_pMapper;
    vtkLODActor         *m_pActor;
    vtkPolyData         *m_polyData;
    vtkGlyph3D          *m_glyph ;
    vtkSmartPointer <vtkSphereSource> m_sphere;
    vtkConeSource       *m_cone;
    vtkCylinderSource   *m_cylinder;
    vtkCubeSource       *m_cube;
    vtkPoints           *m_points;
    vtkPoints           *m_scaled_points;
    vtkUnstructuredGrid * m_pUnstructuredGrid;
    ExtendedGlyph3D     *m_glyphFilter;
    vtkwindow_new *vtkwin;

    bool isScaleActive;
    double m_xRange[2] ,m_yRange[2] , m_zRange[2];
    double scalingFactorsInv[3];
    int nGlyphs;
    //! value of radius and hieght for glyphs. the usere can use this with scaling or not. default is 1 for both
    double radius,height ;
    //! name of field the isd used for scale the glyphs by radius and/or heigth
    std::string radiusscalar,heightscalar;
    //! if is yes the user can select the scaling for radius and/or glyphs
    std::string scaleGlyphs;
};

#endif
