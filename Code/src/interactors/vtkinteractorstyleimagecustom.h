#ifndef VTKINTERACTORSTYLEIMAGECUSTOM_H
#define VTKINTERACTORSTYLEIMAGECUSTOM_H

#include "../vtkfitsreader.h"

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

    void SetCoordsCallback(const std::function<void(std::string)> &callback);
    void SetLayerFitsReaderFunc(const std::function<vtkSmartPointer<vtkFitsReader>()> &callback);

protected:
    vtkInteractorStyleImageCustom();

private:
    vtkInteractorStyleImageCustom(const vtkInteractorStyleImageCustom &) = delete;
    void operator=(const vtkInteractorStyleImageCustom &) = delete;

    std::map<vtkImageProperty *, double> ColorLevel;
    std::map<vtkImageProperty *, double> ColorWindow;

    vtkSmartPointer<vtkCoordinate> Coordinate;
    std::function<void(std::string)> CoordsCallback;
    std::function<vtkSmartPointer<vtkFitsReader>()> CurrentLayerFitsReader;
};

#endif // VTKINTERACTORSTYLEIMAGECUSTOM_H
