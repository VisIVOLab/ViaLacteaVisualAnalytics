#ifndef LUTSELECTOR_H
#define LUTSELECTOR_H

#include "vtkLookupTable.h"

class LutSelector
{
public:
    LutSelector();
    void SelectLookTable(int idx,vtkLookupTable *lut=NULL);
    void lutVolRenGreen(vtkLookupTable *lut);
    void lutVolRenGlow( vtkLookupTable *lut);
    void lutTenStep( vtkLookupTable *lut);
    void lutTemperature( vtkLookupTable *lut);
    void lutSar( vtkLookupTable *lut);
    void lutPhysicsContour( vtkLookupTable *lut);
    void lutGlow( vtkLookupTable *lut);
    void lutEField( vtkLookupTable *lut);
    void lutDefault( vtkLookupTable *lut);
    void lutDefaultStep( vtkLookupTable *lut);
    void lutGray( vtkLookupTable *lut);
    void lutRun1( vtkLookupTable *lut);
    void lutRun2( vtkLookupTable *lut);
    void lutPureRed( vtkLookupTable *lut);
    void lutPureGreen( vtkLookupTable *lut);
    void lutPureBlue( vtkLookupTable *lut);
    void lutAllYellow( vtkLookupTable *lut);
    void lutAllCyane( vtkLookupTable *lut);
    void lutAllViolet( vtkLookupTable *lut);
    void lutAllWhite( vtkLookupTable *lut);
    void lutAllBlack( vtkLookupTable *lut);
    void lutAllRed( vtkLookupTable *lut);
    void lutAllGreen( vtkLookupTable *lut);
    void lutAllBlu( vtkLookupTable *lut);
    void lutMinMax( vtkLookupTable *lut);
    void lutVolRenRGB( vtkLookupTable *lut);
    void lutVolRenTwoLev( vtkLookupTable *lut);
    void lutFile( vtkLookupTable *lut);

};

#endif // LUTSELECTOR_H
