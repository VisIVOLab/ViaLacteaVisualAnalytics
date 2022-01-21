/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkExtractHistogram.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef vtkExtractHistogram_h
#define vtkExtractHistogram_h

//#include "vtkPVVTKExtensionsDefaultModule.h" //needed for exports
#include "vtkTableAlgorithm.h"

// BTX
class vtkDoubleArray;
class vtkFieldData;
class vtkIntArray;
struct vtkEHInternals;
// ETX

// .NAME vtkExtractHistogram - Extract histogram data (binned values) from any
// dataset
// .SECTION Description
// vtkExtractHistogram accepts any vtkDataSet as input and produces a
// vtkPolyData containing histogram data as output.  The output vtkPolyData
// will have contain a vtkDoubleArray named "bin_extents" which contains
// the boundaries between each histogram bin, and a vtkUnsignedLongArray
// named "bin_values" which will contain the value for each bin.

class vtkExtractHistogram : public vtkTableAlgorithm
{
public:
    static vtkExtractHistogram *New();
    vtkTypeMacro(vtkExtractHistogram, vtkTableAlgorithm);
    void PrintHeader(ostream &os, vtkIndent indent);
    void PrintTrailer(std::ostream &os, vtkIndent indent);
    void PrintSelf(ostream &os, vtkIndent indent);
    // Description:
    // Controls which input data component should be binned, for input arrays
    // with more-than-one component
    vtkSetClampMacro(Component, int, 0, VTK_INT_MAX);
    vtkGetMacro(Component, int);

    // Description:
    // Controls the number of bins N in the output histogram data
    vtkSetClampMacro(BinCount, int, 1, VTK_INT_MAX);
    vtkGetMacro(BinCount, int);

    // Description:
    // Get/Set custom bin ranges to use. These are used only when
    // UseCustomBinRanges is set to true.
    vtkSetVector2Macro(CustomBinRanges, double);
    vtkGetVector2Macro(CustomBinRanges, double);

    // Description:
    // When set to true, CustomBinRanges will  be used instead of using the full
    // range for the selected array. By default, set to false.
    vtkSetMacro(UseCustomBinRanges, bool);
    vtkGetMacro(UseCustomBinRanges, bool);
    vtkBooleanMacro(UseCustomBinRanges, bool);

    // Description:
    // This option controls whether the algorithm calculates averages
    // of variables other than the primary variable that fall into each
    // bin. False by default.
    vtkSetMacro(CalculateAverages, int);
    vtkGetMacro(CalculateAverages, int);
    vtkBooleanMacro(CalculateAverages, int);

protected:
    vtkExtractHistogram();
    ~vtkExtractHistogram();

    // Description:
    // Returns the data range for the input array to process.
    // This method is not called with this->UseCustomBinRanges is true.
    // Returns true is range could be determined correctly, otherwise returns
    // false and range is set to {VTK_DOUBLE_MAX, VTK_DOUBLE_MIN}. When returning
    // true, the actual data range is returned (without any extra padding).
    virtual bool GetInputArrayRange(vtkInformationVector **inputVector, double range[2]);

    virtual int FillInputPortInformation(int port, vtkInformation *info);

    virtual int RequestData(vtkInformation *request, vtkInformationVector **inputVector,
                            vtkInformationVector *outputVector);

    // Initialize the bin_extents using the data range for the selected
    // array.
    virtual bool InitializeBinExtents(vtkInformationVector **inputVector,
                                      vtkDoubleArray *bin_extents, double &min, double &max);

    void BinAnArray(vtkDataArray *src, vtkIntArray *vals, double min, double max,
                    vtkFieldData *field);

    void FillBinExtents(vtkDoubleArray *bin_extents, double min, double max);

    double CustomBinRanges[2];
    bool UseCustomBinRanges;
    int Component;
    int BinCount;
    int CalculateAverages;

    vtkEHInternals *Internal;

private:
    void operator=(const vtkExtractHistogram &); // Not implemented
    vtkExtractHistogram(const vtkExtractHistogram &); // Not implemented

    int GetInputFieldAssociation();
    vtkFieldData *GetInputFieldData(vtkDataObject *input);
};

#endif
