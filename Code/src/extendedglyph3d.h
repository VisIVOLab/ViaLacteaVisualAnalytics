/***************************************************************************
 *   Copyright (C) 2010 by Alessandro Costa                                *
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

#ifndef __ExtendedGlyph3D_h
#define __ExtendedGlyph3D_h

/**
        @author Fabio Vitello <fabio.vitello@oact.inaf.it>
        @author Alessandro Costa <alessandro.costa@oact.inaf.it>
        @author Ugo Becciani <ugo.becciani@oact.inaf.it>
*/

#include "vtkFiltersCoreModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

#define VTK_SCALE_BY_SCALAR 0
#define VTK_SCALE_BY_VECTOR 1
#define VTK_SCALE_BY_VECTORCOMPONENTS 2
#define VTK_DATA_SCALING_OFF 3

#define VTK_COLOR_BY_SCALE  0
#define VTK_COLOR_BY_SCALAR 1
#define VTK_COLOR_BY_VECTOR 2

#define VTK_USE_VECTOR 0
#define VTK_USE_NORMAL 1
#define VTK_VECTOR_ROTATION_OFF 2

#define VTK_INDEXING_OFF 0
#define VTK_INDEXING_BY_SCALAR 1
#define VTK_INDEXING_BY_VECTOR 2

class vtkTransform;

class  ExtendedGlyph3D : public vtkPolyDataAlgorithm
{
public:
    vtkTypeMacro(ExtendedGlyph3D,vtkPolyDataAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent);
    void PrintHeader(ostream& os, vtkIndent indent);
    void PrintTrailer(std::ostream& os , vtkIndent indent);

    // Description
    // Construct object with scaling on, scaling mode is by scalar value,
    // scale factor = 1.0, the range is (0,1), orient geometry is on, and
    // orientation is by vector. Clamping and indexing are turned off. No
    // initial sources are defined.
    static ExtendedGlyph3D *New();

    // Description:
    // Set the source to use for the glyph.
    // Note that this method does not connect the pipeline. The algorithm will
    // work on the input data as it is without updating the producer of the data.
    // See SetSourceConnection for connecting the pipeline.
    void SetSourceData(vtkPolyData *pd) {this->SetSourceData(0,pd);};

    // Description:
    // Specify a source object at a specified table location.
    // Note that this method does not connect the pipeline. The algorithm will
    // work on the input data as it is without updating the producer of the data.
    // See SetSourceConnection for connecting the pipeline.
    void SetSourceData(int id, vtkPolyData *pd);

    // Description:
    // Specify a source object at a specified table location. New style.
    // Source connection is stored in port 1. This method is equivalent
    // to SetInputConnection(1, id, outputPort).
    void SetSourceConnection(int id, vtkAlgorithmOutput* algOutput);
    void SetSourceConnection(vtkAlgorithmOutput* algOutput)
    {
        this->SetSourceConnection(0, algOutput);
    }

    // Description:
    // Get a pointer to a source object at a specified table location.
    vtkPolyData *GetSource(int id=0);

    // Description:
    // Turn on/off scaling of source geometry.
    vtkSetMacro(Scaling,int);
    vtkBooleanMacro(Scaling,int);
    vtkGetMacro(Scaling,int);

    // Description:
    // Either scale by scalar or by vector/normal magnitude.
    vtkSetMacro(ScaleMode,int);
    vtkGetMacro(ScaleMode,int);
    void SetScaleModeToScaleByScalar()
    {this->SetScaleMode(VTK_SCALE_BY_SCALAR);};
    void SetScaleModeToScaleByVector()
    {this->SetScaleMode(VTK_SCALE_BY_VECTOR);};
    void SetScaleModeToScaleByVectorComponents()
    {this->SetScaleMode(VTK_SCALE_BY_VECTORCOMPONENTS);};
    void SetScaleModeToDataScalingOff()
    {this->SetScaleMode(VTK_DATA_SCALING_OFF);};
    const char *GetScaleModeAsString();

    // Description:
    // Either color by scale, scalar or by vector/normal magnitude.
    vtkSetMacro(ColorMode,int);
    vtkGetMacro(ColorMode,int);
    void SetColorModeToColorByScale()
    {this->SetColorMode(VTK_COLOR_BY_SCALE);};
    void SetColorModeToColorByScalar()
    {this->SetColorMode(VTK_COLOR_BY_SCALAR);};
    void SetColorModeToColorByVector()
    {this->SetColorMode(VTK_COLOR_BY_VECTOR);};
    const char *GetColorModeAsString();

    // Description:
    // Specify scale factor to scale object by.
    vtkSetMacro(ScaleFactor,double);
    vtkGetMacro(ScaleFactor,double);

    // Description:
    // Specify range to map scalar values into.
    vtkSetVector2Macro(Range,double);
    vtkGetVectorMacro(Range,double,2);

    // Description:
    // Turn on/off orienting of input geometry along vector/normal.
    vtkSetMacro(Orient,int);
    vtkBooleanMacro(Orient,int);
    vtkGetMacro(Orient,int);

    // Description:
    // Turn on/off clamping of "scalar" values to range. (Scalar value may be
    //  vector magnitude if ScaleByVector() is enabled.)
    vtkSetMacro(Clamping,int);
    vtkBooleanMacro(Clamping,int);
    vtkGetMacro(Clamping,int);

    // Description:
    // Specify whether to use vector or normal to perform vector operations.
    vtkSetMacro(VectorMode,int);
    vtkGetMacro(VectorMode,int);
    void SetVectorModeToUseVector() {this->SetVectorMode(VTK_USE_VECTOR);};
    void SetVectorModeToUseNormal() {this->SetVectorMode(VTK_USE_NORMAL);};
    void SetVectorModeToVectorRotationOff()
    {this->SetVectorMode(VTK_VECTOR_ROTATION_OFF);};
    const char *GetVectorModeAsString();

    // Description:
    // Index into table of sources by scalar, by vector/normal magnitude, or
    // no indexing. If indexing is turned off, then the first source glyph in
    // the table of glyphs is used. Note that indexing mode will only use the
    // InputScalarsSelection array and not the InputColorScalarsSelection
    // as the scalar source if an array is specified.
    vtkSetMacro(IndexMode,int);
    vtkGetMacro(IndexMode,int);
    void SetIndexModeToScalar() {this->SetIndexMode(VTK_INDEXING_BY_SCALAR);};
    void SetIndexModeToVector() {this->SetIndexMode(VTK_INDEXING_BY_VECTOR);};
    void SetIndexModeToOff() {this->SetIndexMode(VTK_INDEXING_OFF);};
    const char *GetIndexModeAsString();

    // Description:
    // Enable/disable the generation of point ids as part of the output. The
    // point ids are the id of the input generating point. The point ids are
    // stored in the output point field data and named "InputPointIds". Point
    // generation is useful for debugging and pick operations.
    vtkSetMacro(GeneratePointIds,int);
    vtkGetMacro(GeneratePointIds,int);
    vtkBooleanMacro(GeneratePointIds,int);

    // Description:
    // Set/Get the name of the PointIds array if generated. By default the Ids
    // are named "InputPointIds", but this can be changed with this function.
    vtkSetStringMacro(PointIdsName);
    vtkGetStringMacro(PointIdsName);

    // Description:
    // Enable/disable the generation of cell data as part of the output.
    // The cell data at each cell will match the point data of the input
    // at the glyphed point.
    vtkSetMacro(FillCellData,int);
    vtkGetMacro(FillCellData,int);
    vtkBooleanMacro(FillCellData,int);

    // Description:
    // This can be overwritten by subclass to return 0 when a point is
    // blanked. Default implementation is to always return 1;
    virtual int IsPointVisible(vtkDataSet*, vtkIdType) {return 1;};

    // Description:
    // When set, this is use to transform the source polydata before using it to
    // generate the glyph. This is useful if one wanted to reorient the source,
    // for example.
    void SetSourceTransform(vtkTransform*);
    vtkGetObjectMacro(SourceTransform, vtkTransform);

    // Description:
    // Overridden to include SourceTransform's MTime.
    virtual unsigned long GetMTime();
    vtkSetMacro(ScalarVisibility,int);
    vtkBooleanMacro(ScalarVisibility,int);
    vtkGetMacro(ScalarVisibility,int);



    vtkSetMacro(UseSecondScalar,bool);
    vtkSetMacro(UseThirdScalar,bool);

    vtkSetStringMacro(InputScalarsSelectionXZ);
    vtkSetStringMacro(InputScalarsSelectionY);


protected:
    ExtendedGlyph3D();
    ~ExtendedGlyph3D();

    int ScalarVisibility;

    bool UseSecondScalar;
    bool UseThirdScalar;
    char *InputScalarsSelectionXZ;
    char *InputScalarsSelectionY;
    virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
    virtual int RequestUpdateExtent(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
    virtual int FillInputPortInformation(int, vtkInformation *);

    vtkPolyData* GetSource(int idx, vtkInformationVector *sourceInfo);

    // Description:
    // Method called in RequestData() to do the actual data processing. This will
    // glyph the \c input, filling up the \c output based on the filter
    // parameters.
    virtual bool Execute(vtkDataSet* input,
                         vtkInformationVector* sourceVector,
                         vtkPolyData* output, int requestedGhostLevel);

    vtkPolyData **Source; // Geometry to copy to each point
    int Scaling; // Determine whether scaling of geometry is performed
    int ScaleMode; // Scale by scalar value or vector magnitude
    int ColorMode; // new scalars based on scale, scalar or vector
    double ScaleFactor; // Scale factor to use to scale geometry
    double Range[2]; // Range to use to perform scalar scaling
    int Orient; // boolean controls whether to "orient" data
    int VectorMode; // Orient/scale via normal or via vector data
    int Clamping; // whether to clamp scale factor
    int IndexMode; // what to use to index into glyph table
    int GeneratePointIds; // produce input points ids for each output point
    int FillCellData; // whether to fill output cell data
    char *PointIdsName;
    vtkTransform* SourceTransform;

private:
    ExtendedGlyph3D(const ExtendedGlyph3D&);  // Not implemented.
    void operator=(const ExtendedGlyph3D&);  // Not implemented.
};

// Description:
// Return the method of scaling as a descriptive character string.
inline const char *ExtendedGlyph3D::GetScaleModeAsString(void)
{
    if ( this->ScaleMode == VTK_SCALE_BY_SCALAR )
    {
        return "ScaleByScalar";
    }
    else if ( this->ScaleMode == VTK_SCALE_BY_VECTOR )
    {
        return "ScaleByVector";
    }
    else
    {
        return "DataScalingOff";
    }
}

// Description:
// Return the method of coloring as a descriptive character string.
inline const char *ExtendedGlyph3D::GetColorModeAsString(void)
{
    if ( this->ColorMode == VTK_COLOR_BY_SCALAR )
    {
        return "ColorByScalar";
    }
    else if ( this->ColorMode == VTK_COLOR_BY_VECTOR )
    {
        return "ColorByVector";
    }
    else
    {
        return "ColorByScale";
    }
}

// Description:
// Return the vector mode as a character string.
inline const char *ExtendedGlyph3D::GetVectorModeAsString(void)
{
    if ( this->VectorMode == VTK_USE_VECTOR)
    {
        return "UseVector";
    }
    else if ( this->VectorMode == VTK_USE_NORMAL)
    {
        return "UseNormal";
    }
    else
    {
        return "VectorRotationOff";
    }
}

// Description:
// Return the index mode as a character string.
inline const char *ExtendedGlyph3D::GetIndexModeAsString(void)
{
    if ( this->IndexMode == VTK_INDEXING_OFF)
    {
        return "IndexingOff";
    }
    else if ( this->IndexMode == VTK_INDEXING_BY_SCALAR)
    {
        return "IndexingByScalar";
    }
    else
    {
        return "IndexingByVector";
    }
}

#endif
