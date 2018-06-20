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

#ifndef PIPE_H
#define PIPE_H

/**
        @author Alessandro Costa <alessandro.costa@oact.inaf.it>
        @author Ugo Becciani <ugo.becciani@oact.inaf.it>
*/

#include "vtkPolyDataMapper.h"
#include <vector>
#include <map>
#include "vstabledesktop.h"
#include "vtkScalarBarActor.h"

class vtkRenderer;
class vtkRenderWindow;
class vtkCamera;
class vtkLookupTable;
class VSTableDesktop;
class vtkwindow_new;
class vtkLODActor;
class vtkCubeAxesActor2D;

class Pipe
{
    VSTableDesktop *m_VSTable;

public:
    Pipe(VSTableDesktop *table);
    VSTableDesktop *getTable();
    void saveImageAsPng(int num );
    virtual  int createPipe();
    virtual  void destroyAll(){};
    virtual  bool readData(QStringList list);
    bool readDataFor3D(QStringList list);

    virtual  int getCamera();
    vtkLODActor* outlineActor;
    vtkCubeAxesActor2D* axesActor;
    std::string colorScalar;
    vtkScalarBarActor *scalarBar;

    vtkLookupTable* getLookupTable(){return m_lut;}
    int getRows(){return m_nRows;}


protected:

    void setCamera();
    void constructVTK(vtkwindow_new *v);
    void destroyVTK();
    void setBoundingBox ( vtkDataObject *data );
    void colorBar (bool showColorBar);
    virtual  void setAxes(vtkDataSet *data,double *bounds);
    int m_nRows;
    int m_nCols;
    std::map<std::string, int> m_colNames;
    vtkCamera           *m_camera;
    vtkRenderer         *m_pRenderer;
    vtkRenderWindow     *m_pRenderWindow;
    vtkLookupTable      *m_lut;
    vtkwindow_new       *vtkwin;
    double scalingFactors;

    //new stuff here...
    float **m_tableData;
    std::vector<std::string> m_fieldNames;
    std::string m_path;
};

#endif

