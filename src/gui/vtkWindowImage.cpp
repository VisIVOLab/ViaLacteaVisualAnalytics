#include "vtkWindowImage.h"
#include "ui_vtkWindowImage.h"

#include "ColorMaps.h"
#include "vtkFITSReader.h"

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkInteractorStyleImage.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>

#include <vtkImageMapToColors.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkLookupTable.h>

vtkWindowImage::vtkWindowImage(QWidget *parent) : QMainWindow(parent), ui(new Ui::vtkWindowImage)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);

    this->setupRenderer();
    this->showImage();
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
    ui->vtk->setEnableTouchEventProcessing(false);

    vtkNew<vtkInteractorStyleImage> style;
    renderWindow->GetInteractor()->SetInteractorStyle(style);
}

void vtkWindowImage::showImage()
{
    vtkNew<vtkFITSReader> fitsReader;
    fitsReader->SetFileName("/Users/giuseppe/Misc/images/HorseHead.fits");
    fitsReader->Update();

    vtkNew<vtkLookupTable> lut;
    lut->SetTableRange(fitsReader->GetMin(), fitsReader->GetMax());
    lut->SetNanColor(1., 1., 1., 1.);
    ColorMaps::SetColorMap(lut, "Viridis");

    vtkNew<vtkImageMapToColors> colorMap;
    colorMap->SetInputConnection(fitsReader->GetOutputPort());
    colorMap->SetLookupTable(lut);
    vtkNew<vtkImageSliceMapper> sliceMapper;
    sliceMapper->SetInputConnection(colorMap->GetOutputPort());
    sliceMapper->BorderOn();

    vtkNew<vtkImageSlice> slice;
    slice->SetMapper(sliceMapper);
    slice->GetProperty()->SetInterpolationTypeToNearest();

    ui->vtk->renderWindow()->GetRenderers()->GetFirstRenderer()->AddViewProp(slice);
    ui->vtk->renderWindow()->Render();
}
