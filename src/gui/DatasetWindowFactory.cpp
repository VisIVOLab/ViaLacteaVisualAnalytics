#include "DatasetWindowFactory.h"

#include "vtkWindowCube.h"
#include "vtkWindowImage.h"

#include <QWidget>

QWidget *DatasetWindowFactory::createWindow(const DatasetOpenInfo &dataset, QWidget *parent) const
{
    switch (dataset.kind) {
    case DatasetKind::Image:
        return new vtkWindowImage(dataset.filepath, parent);
    case DatasetKind::Cube:
        return new vtkWindowCube(dataset.filepath, parent);
    case DatasetKind::Unknown:
    default:
        return nullptr;
    }
}
