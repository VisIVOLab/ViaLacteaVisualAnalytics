#include "vtkwindowimage.h"
#include "ui_vtkwindowimage.h"

#include "interactors/vtkinteractorstyleimagecustom.h"

#include "astroutils.h"
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
#include <vtkTransform.h>

#include <QListWidgetItem>

vtkWindowImage::vtkWindowImage(QWidget *parent, vtkSmartPointer<vtkFitsReader> fitsReader)
    : QMainWindow(parent), ui(new Ui::vtkWindowImage), fitsReader(fitsReader)
{
    ui->setupUi(this);
    connect(ui->layersList->model(), &QAbstractItemModel::rowsMoved, this,
            &vtkWindowImage::layerList_rowsMoved);
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

    auto imageObject = new vtkfitstoolwidgetobject(0);
    imageObject->setFitsReader(fitsReader);
    imageObject->setName(QString::fromStdString(fitsReader->GetFileName()));
    imageObject->setLutScale("Log");
    imageObject->setLutType("Gray");
    addLayerToList(imageObject);

    setInteractorStyleImage();
}

vtkWindowImage::~vtkWindowImage()
{
    delete ui;
}

vtkSmartPointer<vtkFitsReader> vtkWindowImage::getFitsReader() const
{
    return fitsReader;
}

void vtkWindowImage::render()
{
    ui->qVtk->renderWindow()->GetInteractor()->Render();
}

void vtkWindowImage::showStatusBarMessage(const std::string &msg)
{
    ui->statusBar->showMessage(QString::fromStdString(msg));
}

void vtkWindowImage::addLayerImage(vtkSmartPointer<vtkFitsReader> fitsReader, const QString &survey,
                                   const QString &species, const QString &transition)
{
    auto imageShiftScale = vtkSmartPointer<vtkImageShiftScale>::New();
    imageShiftScale->SetOutputScalarTypeToUnsignedChar();
    imageShiftScale->SetInputData(fitsReader->GetOutput());
    imageShiftScale->Update();

    // Set spacing
    double scaledPixel = AstroUtils::arcsecPixel(fitsReader->GetFileName())
            / AstroUtils::arcsecPixel(this->fitsReader->GetFileName());
    fitsReader->GetOutput()->SetSpacing(scaledPixel, scaledPixel, 1);

    // Set origin
    double sky_coord_gal[2];
    AstroUtils::xy2sky(fitsReader->GetFileName(), 0, 0, sky_coord_gal, WCS_GALACTIC);
    double coord[3];
    AstroUtils::sky2xy(this->fitsReader->GetFileName(), sky_coord_gal[0], sky_coord_gal[1], coord);
    fitsReader->GetOutput()->SetOrigin(coord[0], coord[1], 0);

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
    imageSlice->GetProperty()->SetOpacity(0.5);

    // Rotation
    double angle = 0;
    double x1 = coord[0];
    double y1 = coord[0];

    AstroUtils::xy2sky(fitsReader->GetFileName(), 0, 100, sky_coord_gal, WCS_GALACTIC);
    AstroUtils::sky2xy(this->fitsReader->GetFileName(), sky_coord_gal[0], sky_coord_gal[1], coord);

    if (x1 != coord[0]) {
        double m = fabs((coord[1] - y1) / (coord[0] - x1));
        angle = 1 * (90 - atan(m) * 180 / M_PI);
    }

    double bounds[6];
    fitsReader->GetOutput()->GetBounds(bounds);

    auto transform = vtkSmartPointer<vtkTransform>::New();
    transform->Translate(bounds[0], bounds[2], bounds[4]);
    transform->RotateWXYZ(angle, 0, 0, 1);
    transform->Translate(-bounds[0], -bounds[2], -bounds[4]);
    imageSlice->SetUserTransform(transform);
    imageStack->AddImage(imageSlice);

    auto imageObject = new vtkfitstoolwidgetobject(0);
    imageObject->setFitsReader(fitsReader);
    imageObject->setName(QString::fromStdString(fitsReader->GetFileName()));
    imageObject->setLutScale("Log");
    imageObject->setLutType("Gray");
    if (!survey.isEmpty() && !species.isEmpty() && !species.isEmpty()) {
        imageObject->setSurvey(survey);
        imageObject->setSpecies(species);
        imageObject->setTransition(transition);
    }
    addLayerToList(imageObject);
}

void vtkWindowImage::setInteractorStyleImage()
{
    auto interactorStyle = vtkSmartPointer<vtkInteractorStyleImageCustom>::New();
    interactorStyle->SetCoordsCallback([this](std::string str) { showStatusBarMessage(str); });
    interactorStyle->SetLayerFitsReaderFunc([this]() -> vtkSmartPointer<vtkFitsReader> {
        int index = selectedLayerIndex();
        return imageLayersList.at(index)->getFits();
    });
    ui->qVtk->renderWindow()->GetInteractor()->SetInteractorStyle(interactorStyle);
}

void vtkWindowImage::addLayerToList(vtkfitstoolwidgetobject *layer, bool enabled)
{
    /// \todo
    /// Fetch survey, species and transition from selected item in vlkb inventory table

    QString text;
    if (!layer->getSurvey().isEmpty() && !layer->getSpecies().isEmpty()
        && !layer->getTransition().isEmpty()) {
        text = layer->getSurvey() + "_" + layer->getSpecies() + "_" + layer->getTransition();
    } else {
        text = layer->getName();
    }

    layer->setLayerNumber(imageStack->GetImages()->GetNumberOfItems() - 1);
    imageLayersList.append(layer);

    auto item = new QListWidgetItem(text, ui->layersList);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);

    render();
}

void vtkWindowImage::changeFitsLut(const QString &palette, const QString &scale)
{
    int index = selectedLayerIndex();
    auto imageObject = imageLayersList.at(index);
    auto fitsReader = imageObject->getFits();

    QString lutScale = scale.isEmpty() ? imageObject->getLutScale() : scale;
    auto lut = vtkSmartPointer<vtkLookupTable>::New();
    if (lutScale == "Log") {
        lut->SetScaleToLog10();
    } else {
        lut->SetScaleToLinear();
    }
    lut->SetTableRange(fmax(0, fitsReader->GetMin()), fitsReader->GetMax());
    SelectLookTable(palette, lut);

    // Update info on the image object
    imageObject->setLutType(palette);
    imageObject->setLutScale(lutScale);

    auto colors = vtkSmartPointer<vtkImageMapToColors>::New();
    colors->SetInputData(fitsReader->GetOutput());
    colors->SetLookupTable(lut);
    colors->Update();

    auto mapper = vtkSmartPointer<vtkImageSliceMapper>::New();
    mapper->SetInputData(colors->GetOutput());
    vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(index))->SetMapper(mapper);
}

void vtkWindowImage::changeLayerVisibility(int index, bool visibility)
{
    auto image = vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(index));
    if (image) {
        image->SetVisibility(visibility);
        render();
    }
}

int vtkWindowImage::selectedLayerIndex() const
{
    int index = 0;

    auto list = ui->layersList->selectedItems();
    if (!list.empty()) {
        index = ui->layersList->row(list.first());
    }

    return index;
}

void vtkWindowImage::on_opacitySlider_valueChanged(int value)
{
    int index = selectedLayerIndex();

    vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(index))
            ->GetProperty()
            ->SetOpacity(value / 100.0);

    render();
}

void vtkWindowImage::on_lutComboBox_textActivated(const QString &lut)
{
    changeFitsLut(lut);
    render();
}

void vtkWindowImage::on_logRadioBtn_toggled(bool checked)
{
    QString scale = checked ? "Log" : "Linear";
    changeFitsLut(ui->lutComboBox->currentText(), scale);
    render();
}

void vtkWindowImage::on_layersList_itemClicked(QListWidgetItem *item)
{
    item->setSelected(true);
    int index = item->listWidget()->row(item);
    auto imageObject = imageLayersList.at(index);
    auto image = vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(index));
    int opacity = static_cast<int>(image->GetProperty()->GetOpacity() * 100.0);

    imageStack->SetActiveLayer(index);
    ui->opacitySlider->setValue(opacity);
    ui->lutComboBox->setCurrentText(imageObject->getLutType());
    auto radioBtn = imageObject->getLutScale() == "Log" ? ui->logRadioBtn : ui->linearRadioBtn;
    radioBtn->setChecked(true);
}

void vtkWindowImage::on_layersList_itemChanged(QListWidgetItem *item)
{
    int index = item->listWidget()->row(item);
    bool visibility = item->checkState() == Qt::Checked;
    changeLayerVisibility(index, visibility);
}

void vtkWindowImage::layerList_rowsMoved(const QModelIndex &parent, int start, int end,
                                         const QModelIndex &destination, int row)
{
    Q_UNUSED(parent);
    Q_UNUSED(end);
    Q_UNUSED(destination);

    if (start > row) {
        // Down
        for (int i = start - 1; i >= row; --i) {
            vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(i))
                    ->GetProperty()
                    ->SetLayerNumber(i + 1);
            imageLayersList.swapItemsAt(i, i + 1);
        }
        vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(start))
                ->GetProperty()
                ->SetLayerNumber(row);
    } else {
        // Up
        for (int i = start + 1; i < row; ++i) {
            vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(i))
                    ->GetProperty()
                    ->SetLayerNumber(i - 1);
            imageLayersList.swapItemsAt(i, i - 1);
        }
        vtkImageSlice::SafeDownCast(imageStack->GetImages()->GetItemAsObject(start))
                ->GetProperty()
                ->SetLayerNumber(row - 1);
    }

    render();
}
