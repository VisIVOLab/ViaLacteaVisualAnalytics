#include "pqwindowimage.h"
#include "pqFileDialog.h"
#include "ui_pqwindowimage.h"

#include "interactors/vtkinteractorstyleimagecustom.h"
#include "vtklegendscaleactor.h"

#include <pqActiveObjects.h>
#include <pqAlwaysConnectedBehavior.h>
#include <pqApplicationCore.h>
#include <pqLoadDataReaction.h>
#include <pqObjectBuilder.h>
#include <pqPipelineSource.h>
#include <pqRenderView.h>
#include <pqSMAdaptor.h>

#include <vtkPVArrayInformation.h>
#include <vtkPVDataInformation.h>
#include <vtkPVDataMover.h>
#include <vtkPVDataSetAttributesInformation.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSMCoreUtilities.h>
#include <vtkSMProperty.h>
#include <vtkSMPropertyHelper.h>
#include "vtkSMProxyManager.h"
#include <vtkSMPVRepresentationProxy.h>
#include "vtkSMReaderFactory.h"
#include <vtkSMSessionProxyManager.h>
#include <vtkSMTransferFunctionManager.h>
#include <vtkSMTransferFunctionPresets.h>
#include <vtkSMTransferFunctionProxy.h>
#include <vtkSMViewProxy.h>

#include <QCheckBox>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QSignalMapper>

#define logScaleDefault true

pqWindowImage::pqWindowImage(const QString &filepath, const CubeSubset &cubeSubset) : ui(new Ui::pqWindowImage)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    // Create the LUT combo box options
    clmInit = true;
    auto presets = vtkSMTransferFunctionPresets::GetInstance();
    for (int i = 0; i < presets->GetNumberOfPresets(); ++i) {
        QString name = QString::fromStdString(presets->GetPresetName(i));
        ui->cmbxLUTSelect->addItem(name);
    }
    ui->cmbxLUTSelect->setCurrentIndex(ui->cmbxLUTSelect->findText("Grayscale"));
    ui->linearRadioButton->setChecked(true);
    this->images = std::vector<vlvaStackImage*>();

    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->lstImageList->setDragDropMode(QAbstractItemView::InternalMove);
    connect(ui->lstImageList->model(), SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)),
            this, SLOT(movedLayersRow(QModelIndex, int, int, QModelIndex, int)));

    // Set default opacity
    ui->opacitySlider->setSliderPosition(ui->opacitySlider->maximum());

    clmInit = false;

    // ParaView Init
    builder = pqApplicationCore::instance()->getObjectBuilder();
    server = pqActiveObjects::instance().activeServer();
    serverProxyManager = server->proxyManager();
    new pqAlwaysConnectedBehavior(this);

    // Enable annotations such as Remote Rendering, FPS, etc...
    auto renderingSettings = serverProxyManager->GetProxy("settings", "RenderViewSettings");
    vtkSMPropertyHelper(renderingSettings, "ShowAnnotation").Set(1);

    vtkSMPropertyHelper(renderingSettings, "LODThreshold").Set(512);
    vtkSMPropertyHelper(renderingSettings, "LODResolution").Set(0);
    vtkSMPropertyHelper(renderingSettings, "UseOutlineForLODRendering").Set(1);
    vtkSMPropertyHelper(renderingSettings, "RemoteRenderThreshold").Set(512);
    vtkSMPropertyHelper(renderingSettings, "CompressorConfig").Set("vtkLZ4Compressor 0 5");

    createView();
    addImageToStack(filepath, cubeSubset);
    updateUI();

    /*    // Load Reactions

    BaseImageSource = pqLoadDataReaction::loadData({ filepath });

    // Handle Subset selection
    setSubsetProperties(cubeSubset);

    imageProxy = builder->createDataRepresentation(this->BaseImageSource->getOutputPort(0), viewImage)->getProxy();
    addImageToStack(filepath.toStdString());

    // Fetch information from source and header and update UI
    readInfoFromSource();
    readHeaderFromSource();
    //    rms = getRMSFromHeader(fitsHeader);
    //    lowerBound = 3 * rms;
    //    upperBound = dataMax;
    //    ui->lowerBoundText->setText(QString::number(lowerBound, 'f', 4));
    //    ui->upperBoundText->setText(QString::number(upperBound, 'f', 4));
    //    ui->minCubeText->setText(QString::number(dataMin, 'f', 4));
    //    ui->maxCubeText->setText(QString::number(dataMax, 'f', 4));
    //    ui->rmsCubeText->setText(QString::number(rms, 'f', 4));

    // Show Legend Scale Actors in image renderer
    fitsHeaderPath = createFitsHeaderFile(fitsHeader);
    showLegendScaleActor();

    // Set up colour map controls
    vtkNew<vtkSMTransferFunctionManager> mgr;
    lutProxy = vtkSMTransferFunctionProxy::SafeDownCast(
            mgr->GetColorTransferFunction("FITSImage", imageProxy->GetSessionProxyManager()));
    // Set default colour map
    changeColorMap("Grayscale");
    setLogScale(false);*/

    viewImage->resetDisplay();
    viewImage->render();
}

pqWindowImage::~pqWindowImage()
{
    builder->destroySources(server);
    delete ui;
}

/**
 * @brief pqWindowImage::changeColorMap
 * Changes the colour map used to visualize the FITS image.
 * @param name The name of the colour map (provided by paraview presets)
 */
void pqWindowImage::changeColorMap(const QString &name)
{
    this->images[activeIndex]->changeColorMap(name);
    viewImage->render();
}

/**
 * @brief pqWindowImage::showStatusBarMessage
 * Displays a message on the status bar of the window
 * @param msg The message to display
 */
void pqWindowImage::showStatusBarMessage(const std::string &msg)
{
    ui->statusBar->showMessage(QString::fromStdString(msg));
}

void pqWindowImage::updateUI()
{
    if (images.size() == 0)
        return;

    clmInit = true;
    setWindowTitle(this->images[activeIndex]->getFitsFileName());

    // Interactor to show pixel coordinates in the status bar
    vtkNew<vtkInteractorStyleImageCustom> interactorStyle;
    interactorStyle->SetCoordsCallback(
            [this](const std::string &str) { showStatusBarMessage(str); });
    interactorStyle->SetLayerFitsReaderFunc(this->images[activeIndex]->getFitsHeaderPath());
    viewImage->getViewProxy()->GetRenderWindow()->GetInteractor()->SetInteractorStyle(
            interactorStyle);
    this->ui->sbxStackActiveLayer->setValue(this->activeIndex);
    this->ui->lstImageList->clear();
    std::sort(images.begin(), images.end(), [](vlvaStackImage* a, vlvaStackImage* b){ return a->getIndex() < b->getIndex();});

    for (vlvaStackImage* i : images)
    {
        if (!i->isEnabled())
            continue;
        i->setPosition();
        this->ui->lstImageList->addItem(i->getFitsFileName());
    }

    this->ui->lstImageList->setCurrentItem(this->ui->lstImageList->item(this->activeIndex));

    int newCMIndex = this->ui->cmbxLUTSelect->findText(this->images[activeIndex]->getColourMap());
    this->ui->cmbxLUTSelect->setCurrentIndex(newCMIndex);

    this->ui->sbxStackActiveLayer->setValue(this->activeIndex);
    this->ui->sbxStackActiveLayer->setMaximum(images.size() - 1);

    this->ui->opacitySlider->setValue(this->images.at(activeIndex)->getOpacity() * 100);
    this->ui->logRadioButton->setChecked(this->images.at(activeIndex)->getLogScale());
    this->ui->linearRadioButton->setChecked(!this->images.at(activeIndex)->getLogScale());

    this->ui->tableWidget->resizeColumnsToContents();
    viewImage->render();
    clmInit = false;
}

void pqWindowImage::showLegendScaleActor()
{
    // Show Legend Scale Actor in image renderer
    auto legendActorCube = vtkSmartPointer<vtkLegendScaleActor>::New();
    legendActorCube->LegendVisibilityOff();
    legendActorCube->setFitsHeader(this->images[activeIndex]->getFitsHeaderPath());
    auto rwImage = viewImage->getViewProxy()->GetRenderWindow()->GetRenderers();
    vtkRenderer::SafeDownCast(rwImage->GetItemAsObject(1))->AddActor(legendActorCube);
}

/**
 * @brief pqWindowImage::setLogScale
 * Sets the image visualization to use log10 scaling on the colour map.
 * @param logScale
 * A boolean whether or not to use log10 scaling.
 */
void pqWindowImage::setLogScale(bool logScale)
{
    // If in the process of initialising the UI, ignore this command.
    if (clmInit)
        return;

    this->images[activeIndex]->setLogScale(logScale);
    viewImage->render();
}

/**
 * @brief pqWindowImage::setOpacity
 * Function that sets the image opacity according to the given value.
 * @param value
 * A value between 0 and 1, with 0 being fully transparent and 1 being fully opaque.
 */
void pqWindowImage::setOpacity(float value)
{
    this->images[activeIndex]->setOpacity(value);
    viewImage->render();
}

int pqWindowImage::addImageToStack(QString file, const CubeSubset &subset)
{
    // If in the process of initialising the UI, ignore this command.
    if (clmInit)
        return 1;
    std::cerr << "Adding image " << file.toStdString() << " to stack via proxy call." << std::endl;
    int index = images.size();
    auto im = new vlvaStackImage(file, index, logScaleDefault, builder, viewImage, serverProxyManager);
    images.push_back(im);
    this->activeIndex = images.size() - 1;

    if (images[activeIndex]->init(file, subset))
    {
        addImageToLists(images[activeIndex]);
        this->updateUI();
        viewImage->render();
        return 1;
    } else
    {
        std::cerr << "Failed to initialise image for file " << file.toStdString() << "." << std::endl;
        removeImageFromStack(activeIndex);
        return 0;
    }
}

int pqWindowImage::removeImageFromStack(const int index)
{
    // If in the process of initialising the UI, ignore this command.
    if (clmInit)
        return 1;
    std::cerr << "Removing image at pos " << index << " from stack via proxy call." << std::endl;
    images.erase(images.begin() + index);
    return 1;
}

int pqWindowImage::addImageToLists(vlvaStackImage *stackImage)
{
    QSignalMapper *mapper = new QSignalMapper(this);
    QSignalMapper *mapper_slider = new QSignalMapper(this);

    int row = ui->tableWidget->model()->rowCount();
    ui->tableWidget->insertRow(row);

    QCheckBox *cb1 = new QCheckBox();

    if (stackImage->isEnabled())
        cb1->setChecked(true);
    else
        cb1->setChecked(false);

    if (stackImage->getType() != 3) {
        double r, g, b;
//        double r = stackImage->getActor()->GetProperty()->GetColor()[0] * 255;
//        double g = stackImage->getActor()->GetProperty()->GetColor()[1] * 255;
//        double b = stackImage->getActor()->GetProperty()->GetColor()[2] * 255;
        r = g = b = 160;

        cb1->setStyleSheet("background-color: rgb(" + QString::number(r) + "," + QString::number(g)
                           + " ," + QString::number(b) + ")");


//         //FV MODIFICARE PER AGGIUNGERE COLORE E GESTIONE CHECKBOX
//         QListWidgetItem* item = new QListWidgetItem(o->getName());
//         item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
//         item->setCheckState(Qt::Checked); // AND initialize check state
//         QColor c1(r,g,b);
//         QBrush b1(c1);
//         item->setForeground(b1);
//         ui->listWidget->addItem(item);


    }
    ui->tableWidget->setCellWidget(row, 0, cb1);

    connect(cb1, SIGNAL(stateChanged(int)), mapper, SLOT(map()));
    mapper->setMapping(cb1, row);

    QTableWidgetItem *item_1 = new QTableWidgetItem();
    item_1->setFlags(item_1->flags() ^ Qt::ItemIsEditable);

    item_1->setText(stackImage->getFitsFileName());

    ui->tableWidget->setItem(row, 1, item_1);

    connect(mapper, SIGNAL(mapped(int)), this, SLOT(tableCheckboxClicked(int)));

    return 1;
}

/**
 * @brief pqWindowImage::createView
 * Function that creates the window in which the data is visualized.
 */
void pqWindowImage::createView()
{
    viewImage = qobject_cast<pqRenderView *>(
            builder->createView(pqRenderView::renderViewType(), server));
    viewImage->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->PVLayout->addWidget(viewImage->widget());
}



void pqWindowImage::tableCheckboxClicked(int cb, bool status)
{
    auto im = images.at(cb);
    if (im->isEnabled()){
        im->setActive(false);
    }
    else{
        im->setActive(true);
    }
    updateUI();
}

void pqWindowImage::movedLayersRow(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow)
{
    if (sourceStart > destinationRow) { // down

        for (int i = sourceStart - 1; i >= destinationRow; i--) {
            images.at(i)->setIndex(i + 1);
        }

        images.at(sourceStart)->setIndex(destinationRow);
        this->activeIndex = destinationRow;
    } else { // up

        for (int i = sourceStart + 1; i < destinationRow; i++) {
            images.at(i)->setIndex(i - 1);
        }
        images.at(sourceStart)->setIndex(destinationRow - 1);
        this->activeIndex = destinationRow - 1;
    }
    updateUI();
}

/**
 * @brief pqWindowImage::on_cmbxLUTSelect_currentIndexChanged
 * Changes the colour map used to visualize the image if the
 * user selects from the combo box.
 * @param index The index of the selected item in the combobox,
 * used to retrieve the name of the colour map.
 */
void pqWindowImage::on_cmbxLUTSelect_currentIndexChanged(int index)
{
    // If the UI is being initialised, ignore this command.
    if (index >= 0 && !clmInit) {
        changeColorMap(ui->cmbxLUTSelect->itemText(index));
        setLogScale(this->images.at(activeIndex)->getLogScale());
    }
}

/**
 * @brief pqWindowImage::on_linearRadioButton_toggled
 * Function that changes the scaling used for visualizing the image
 * to be appropriate to which radio button is selected.
 * @param checked Bool that says if the linear radio button is selected.
 */
void pqWindowImage::on_linearRadioButton_toggled(bool checked)
{
    // If in the process of initialising the UI, ignore this command.
    if (clmInit)
        return;
    setLogScale(!checked);
}

/**
 * @brief pqWindowImage::on_opacitySlider_actionTriggered
 * Function that checks what action was triggered by the slider and
 * sets a boolean that is used to determine if the changed value should
 * be sent to the server.
 * @param action The enum value for the action triggered (see documentation
 * for QAbstractSlider::actionTriggered(int action) for details).
 */
void pqWindowImage::on_opacitySlider_actionTriggered(int action)
{
    // Set the slider to only update the image when released or changed by keyboard/mouse
    switch (action) {
    case QSlider::SliderNoAction:
        loadOpacityChange = false;
        break;
    case QSlider::SliderMove:
        loadOpacityChange = false;
        break;
    default:
        loadOpacityChange = true;
        break;
    }
}

/**
 * @brief pqWindowImage::on_opacitySlider_valueChanged
 * Function that updates the opacity value if the slider's value is changed
 * by keyboard or mouse click (not drag).
 * @param value The new value on the opacity slider.
 */
void pqWindowImage::on_opacitySlider_valueChanged(int value)
{
    // If in the process of initialising the UI, ignore this command.
    if (clmInit)
        return;
    // Check what caused the value to change, and don't update while sliding.
    if (!loadOpacityChange)
        return;

    // Opacity is from 0 to 1, slider is from 0 to 100. So divide by 100 to get value to be sent to
    // server.
    float opacityValue = ui->opacitySlider->value() / 100.0;

    setOpacity(opacityValue);
    loadOpacityChange = false;
}

/**
 * @brief pqWindowImage::on_opacitySlider_sliderReleased
 * Function that updates the opacity value when the slider is released
 * after being dragged.
 */
void pqWindowImage::on_opacitySlider_sliderReleased()
{
    // Opacity is from 0 to 1, slider is from 0 to 100. So divide by 100 to get value to be sent to
    // server.
    float opacityValue = ui->opacitySlider->value() / 100.0;

    setOpacity(opacityValue);
    loadOpacityChange = false;
}

void pqWindowImage::on_btnAddImageToStack_clicked()
{
    vtkSMReaderFactory *readerFactory = vtkSMProxyManager::GetProxyManager()->GetReaderFactory();
    QString filters = readerFactory->GetSupportedFileTypes(server->session());
    pqFileDialog dialog(server, this, QString(), QString(), filters);
    dialog.setFileMode(pqFileDialog::ExistingFile);
    if (dialog.exec() == pqFileDialog::Accepted) {
        QString file = dialog.getSelectedFiles().first();
        auto subsetSelector = new SubsetSelectorDialog(this);
        subsetSelector->setAttribute(Qt::WA_DeleteOnClose);
        connect(subsetSelector, &SubsetSelectorDialog::subsetSelected, this, [=](const CubeSubset &subset){this->addImageToStack(file, subset);});
        subsetSelector->show();
        subsetSelector->activateWindow();
        subsetSelector->raise();
    }
}

void pqWindowImage::on_btnRemoveImageFromStack_clicked()
{
    removeImageFromStack(ui->sbxStackActiveLayer->value() + 1);
}


void pqWindowImage::on_sbxStackActiveLayer_valueChanged(int arg1)
{
    if (clmInit)
        return;
    this->activeIndex = this->ui->sbxStackActiveLayer->value();
    updateUI();
}


void pqWindowImage::on_lstImageList_itemClicked(QListWidgetItem *item)
{
    auto imName = item->text();
    for (auto i : images){
        if (imName == i->getFitsFileName())
            activeIndex = i->getIndex();
    }
    updateUI();
}

