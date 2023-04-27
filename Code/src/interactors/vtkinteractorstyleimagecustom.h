#ifndef VTKINTERACTORSTYLEIMAGECUSTOM_H
#define VTKINTERACTORSTYLEIMAGECUSTOM_H

#include "../vtkfitsreader2.h"

#include <vtkCoordinate.h>
#include <vtkInteractorStyleImage.h>
#include <vtkSmartPointer.h>

#include <functional>
#include <map>

class vtkInteractorStyleImageCustom : public vtkInteractorStyleImage
{
public:
    static vtkInteractorStyleImageCustom *New();
    vtkTypeMacro(vtkInteractorStyleImageCustom, vtkInteractorStyleImage);
    void PrintSelf(std::ostream &os, vtkIndent indent) override;

    void OnMouseMove() override;
    void OnChar() override;

    void SetReader(vtkSmartPointer<vtkFitsReader2> reader);
    void SetCoordsCallback(const std::function<void(std::string)> &callback);

protected:
    vtkSmartPointer<vtkFitsReader2> reader;
    vtkSmartPointer<vtkCoordinate> Coordinate;

    vtkInteractorStyleImageCustom();

private:
    vtkInteractorStyleImageCustom(const vtkInteractorStyleImageCustom &) = delete;
    void operator=(const vtkInteractorStyleImageCustom &) = delete;

    std::map<vtkImageProperty *, double> ColorLevel;
    std::map<vtkImageProperty *, double> ColorWindow;

    std::function<void(std::string)> CoordsCallback;
};

#endif // VTKINTERACTORSTYLEIMAGECUSTOM_H
