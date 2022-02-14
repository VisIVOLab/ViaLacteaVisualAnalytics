#include "vtkwindowimage.h"
#include "ui_vtkwindowimage.h"

#include "interactors/vtkinteractorstyleimagecustom.h"

#include "luteditor.h"
#include "vtkfitsreader.h"
#include "vtkfitstoolwidgetobject.h"
#include "vtklegendscaleactor.h"

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageMapToColors.h>
#include <vtkImageProperty.h>
#include <vtkImageShiftScale.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceCollection.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageStack.h>
#include <vtkLookupTable.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

#include <QListWidgetItem>

vtkWindowImage::vtkWindowImage(QWidget *parent, vtkSmartPointer<vtkFitsReader> fitsReader)
    : QMainWindow(parent), ui(new Ui::vtkWindowImage), fitsReader(fitsReader)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(QString::fromStdString(fitsReader->GetFileName()));

    // Start image pipeline
    auto imageShiftScale = vtkSmartPointer<vtkImageShiftScale>::New();
    imageShiftScale->SetOutputScalarTypeToUnsignedChar();
    imageShiftScale->SetInputData(fitsReader->GetOutput());
    imageShiftScale->Update();

    auto lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->SetScaleToLog10();
    lut->SetTableRange(fmax(0, fitsReader->GetMin()), fitsReader->GetMax());
    SelectLookTable("Gray", lut);

    auto colors = vtkSmartPointer<vtkImageMapToColors>::New();
    colors->SetInputData(fitsReader->GetOutput());
    colors->SetLookupTable(lut);
    colors->Update();

    auto imageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
    imageSliceMapper->SetInputData(colors->GetOutput());
    imageSliceMapper->Update();

    auto imageSlice = vtkSmartPointer<vtkImageSlice>::New();
    imageSlice->SetMapper(imageSliceMapper);
    imageSlice->GetProperty()->SetInterpolationTypeToNearest();
    imageSlice->GetProperty()->SetLayerNumber(0);

    imageStack = vtkSmartPointer<vtkImageStack>::New();
    imageStack->AddImage(imageSlice);
    // End image pipeline

    auto legendActor = vtkSmartPointer<vtkLegendScaleActor>::New();
    legendActor->LegendVisibilityOff();
    legendActor->setFitsFile(fitsReader);

    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->SetBackground(0.21, 0.23, 0.25);
    renderer->GlobalWarningDisplayOff();

    renderer->AddActor(legendActor);
    renderer->AddViewProp(imageStack);
    renderer->ResetCamera();

    auto renWin = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    renWin->AddRenderer(renderer);

    ui->qVtk->setDefaultCursor(Qt::ArrowCursor);
    ui->qVtk->setRenderWindow(renWin);
    renWin->GetInteractor()->Render();
    setInteractorStyleImage();

    auto imageObject = new vtkfitstoolwidgetobject(0);
    imageObject->setFitsReader(fitsReader);
    imageObject->setName(QString::fromStdString(fitsReader->GetFileName()));
    imageObject->setLutScale("Log");
    imageObject->setLutType("Gray");
    addLayerToList(imageObject);
}

vtkWindowImage::~vtkWindowImage()
{
    delete ui;
}

vtkSmartPointer<vtkFitsReader> vtkWindowImage::getFitsReader() const
{
    return fitsReader;
}

void vtkWindowImage::showStatusBarMessage(std::string msg)
{
    ui->statusBar->showMessage(QString::fromStdString(msg));
}

void vtkWindowImage::setInteractorStyleImage()
{
    auto interactorStyle = vtkSmartPointer<vtkInteractorStyleImageCustom>::New();
    interactorStyle->SetFitsReader(fitsReader);
    interactorStyle->SetCoordsCallback([this](std::string str) { showStatusBarMessage(str); });
    ui->qVtk->renderWindow()->GetInteractor()->SetInteractorStyle(interactorStyle);
}

void vtkWindowImage::addLayerToList(vtkfitstoolwidgetobject *layer, bool enabled)
{
    /// \todo
    /// Fetch survey, species and transition from selected item in vlkb inventory table

    layer->setLayerNumber(imageStack->GetImages()->GetNumberOfItems() - 1);

    auto item = new QListWidgetItem(layer->getName(), ui->layersList);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
    ui->qVtk->renderWindow()->GetInteractor()->Render();
}

int vtkWindowImage::selectedLayerIndex() const { }

void vtkWindowImage::on_opacitySlider_valueChanged(int value)
{
    int index = 0;

    auto selectedRows = ui->layersList->selectionModel()->selectedRows();
    if (selectedRows.count() > 0)
        index = selectedRows.at(0).row();

    vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(index))
            ->GetProperty()
            ->SetOpacity(value / 100.0);
    ui->qVtk->renderWindow()->GetInteractor()->Render();
}
