#ifndef VTKINTERACTORSTYLEIMAGECUSTOM_H
#define VTKINTERACTORSTYLEIMAGECUSTOM_H

#include "../vtkfitsreader.h"

#include <vtkCoordinate.h>
#include <vtkInteractorStyleImage.h>
#include <vtkSmartPointer.h>

#include <map>

class vtkInteractorStyleImageCustom : public vtkInteractorStyleImage
{
public:
    static vtkInteractorStyleImageCustom *New();
    vtkTypeMacro(vtkInteractorStyleImageCustom, vtkInteractorStyleImage);
    void PrintSelf(std::ostream &os, vtkIndent indent) override;

    void OnMouseMove() override;
    void OnChar() override;

    void SetFitsReader(const vtkSmartPointer<vtkFitsReader> &FitsReader);
    void SetCoordsCallback(const std::function<void(std::string)> &callback);

protected:
    vtkInteractorStyleImageCustom();

private:
    vtkInteractorStyleImageCustom(const vtkInteractorStyleImageCustom &) = delete;
    void operator=(const vtkInteractorStyleImageCustom &) = delete;

    std::map<vtkImageProperty *, double> ColorLevel;
    std::map<vtkImageProperty *, double> ColorWindow;

    vtkSmartPointer<vtkCoordinate> Coordinate;
    vtkSmartPointer<vtkFitsReader> FitsReader;
    std::function<void(std::string)> CoordsCallback;
};

#endif // VTKINTERACTORSTYLEIMAGECUSTOM_H
