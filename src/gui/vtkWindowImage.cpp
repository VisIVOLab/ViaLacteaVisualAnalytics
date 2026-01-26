#include "vtkWindowImage.h"
#include "ui_vtkWindowImage.h"

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNew.h>
#include <vtkRenderer.h>

vtkWindowImage::vtkWindowImage(QWidget *parent) : QMainWindow(parent), ui(new Ui::vtkWindowImage)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setupRenderer();
}

vtkWindowImage::~vtkWindowImage()
{
    delete ui;
}

void vtkWindowImage::setupRenderer()
{
    vtkNew<vtkRenderer> renderer;
    renderer->SetBackground(0.21, 0.23, 0.25);

    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    renderWindow->AddRenderer(renderer);
    ui->vtk->setRenderWindow(renderWindow);
}
