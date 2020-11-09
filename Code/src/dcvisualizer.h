#ifndef DCVISUALIZER
#define DCVISUALIZER
#include <QString>
#include "vtkfitsreader.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkMarchingCubes.h"
#include "vtkStructuredPoints.h"
#include "vtkStructuredPointsGeometryFilter.h"
#include "vtkProperty.h"
#include "vtkOutlineFilter.h"
#include "vtkAlgorithmOutput.h"
#include <vtkSmartPointer.h>
//#include <curl/curl.h>
#include <string.h>
#include <stdio.h>

class dcvisualizer{
public:
    dcvisualizer();
    ~dcvisualizer();
    void visualize(const QString &filename);
    vtkMarchingCubes *shellE;
    vtkRenderWindow *renWin;
};



#endif // DCVISUALIZER
