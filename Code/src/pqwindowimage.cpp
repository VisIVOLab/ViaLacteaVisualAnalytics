#include "pqwindowimage.h"
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

#include <vtkPVArrayInformation.h>
#include <vtkPVDataInformation.h>
#include <vtkPVDataMover.h>
#include <vtkPVDataSetAttributesInformation.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSMCoreUtilities.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMPVRepresentationProxy.h>
#include <vtkSMSessionProxyManager.h>
#include <vtkSMTransferFunctionManager.h>
#include <vtkSMTransferFunctionPresets.h>
#include <vtkSMTransferFunctionProxy.h>
#include <vtkSMViewProxy.h>

#include <QDir>
#include <QFileInfo>

pqWindowImage::pqWindowImage(const QString &filepath, const CubeSubset &cubeSubset)
    : ui(new Ui::pqWindowImage),
      FitsFileName(QFileInfo(filepath).fileName()),
      cubeSubset(cubeSubset)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(FitsFileName);

    // Create the LUT combo box options
    clmInit = true;
    auto presets = vtkSMTransferFunctionPresets::GetInstance();
    for (int i = 0; i < presets->GetNumberOfPresets(); ++i) {
        QString name = QString::fromStdString(presets->GetPresetName(i));
        ui->cmbxLUTSelect->addItem(name);
    }
    ui->cmbxLUTSelect->setCurrentIndex(ui->cmbxLUTSelect->findText("Grayscale"));
    ui->linearRadioButton->setChecked(true);
    clmInit = false;
    logScaleActive = false;

    //Set default opacity
    ui->opacitySlider->setSliderPosition(ui->opacitySlider->maximum());

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

    // Load Reactions
    ImageSource = pqLoadDataReaction::loadData({ filepath });

    // Handle Subset selection
    setSubsetProperties(cubeSubset);

    createView();
    imageProxy = builder->createDataRepresentation(this->ImageSource->getOutputPort(0), viewImage)->getProxy();

    // Fetch information from source and header and update UI
//    readInfoFromSource(); //What does this do? Don't need for 2D apparently.
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


    //Set up colour map controls
    vtkNew<vtkSMTransferFunctionManager> mgr;
    lutProxy = vtkSMTransferFunctionProxy::SafeDownCast(
            mgr->GetColorTransferFunction("FITSImage",
                                          imageProxy->GetSessionProxyManager()));
    //Set default colour map
    changeColorMap("Grayscale");

    // Interactor to show pixel coordinates in the status bar
    vtkNew<vtkInteractorStyleImageCustom> interactorStyle;
    interactorStyle->SetCoordsCallback(
            [this](const std::string &str) { showStatusBarMessage(str); });
    interactorStyle->SetLayerFitsReaderFunc(fitsHeaderPath.toStdString());
    viewImage->getViewProxy()->GetRenderWindow()->GetInteractor()->SetInteractorStyle(
            interactorStyle);

    viewImage->resetDisplay();
    viewImage->render();
}

pqWindowImage::~pqWindowImage()
{
    builder->destroy(ImageSource);
    builder->destroySources(server);
    ImageSource = NULL;
    delete ui;
}

void pqWindowImage::setSubsetProperties(const CubeSubset &subset)
{
    //Set subset limits for the image, to limit memory usage of large images
    auto sourceProxy = ImageSource->getProxy();
    vtkSMPropertyHelper(sourceProxy, "ReadSubExtent").Set(subset.ReadSubExtent);
    vtkSMPropertyHelper(sourceProxy, "SubExtent").Set(subset.SubExtent, 6);
    vtkSMPropertyHelper(sourceProxy, "AutoScale").Set(subset.AutoScale);
    vtkSMPropertyHelper(sourceProxy, "CubeMaxSize").Set(subset.CubeMaxSize);
    if (!subset.AutoScale)
        vtkSMPropertyHelper(sourceProxy, "ScaleFactor").Set(subset.ScaleFactor);
    sourceProxy->UpdateVTKObjects();
}

/**
 * @brief pqWindowImage::changeColorMap
 * Changes the colour map used to visualize the FITS image.
 * @param name The name of the colour map (provided by paraview presets)
 */
void pqWindowImage::changeColorMap(const QString &name)
{
    if (vtkSMProperty *lutProperty = imageProxy->GetProperty("LookupTable")) {

        auto presets = vtkSMTransferFunctionPresets::GetInstance();

        lutProxy->ApplyPreset(presets->GetFirstPresetWithName(name.toStdString().c_str()));
        vtkSMPropertyHelper(lutProperty).Set(lutProxy);
        lutProxy->UpdateVTKObjects();
        vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRange(imageProxy, false, false);
        imageProxy->UpdateVTKObjects();

        viewImage->render();
    }
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

QString pqWindowImage::createFitsHeaderFile(const FitsHeaderMap &fitsHeader)
{
    fitsfile *fptr;
    int status = 0;

    QString headerFile = QDir::homePath()
                                 .append(QDir::separator())
                                 .append("VisIVODesktopTemp")
                                 .append(QDir::separator())
                                 .append(this->FitsFileName);

    fits_create_file(&fptr, ("!" + headerFile).toStdString().c_str(), &status);
    fits_update_key_log(fptr, "SIMPLE", TRUE, "", &status);

    int datatype;
    foreach (const auto &key, fitsHeader.keys()) {
        const auto &value = fitsHeader[key];
        if (key.compare("SIMPLE") == 0) {
            continue;
        }

        if (strchr(value.toStdString().c_str(), '\'')) {
            datatype = TSTRING;
        } else if (strchr(value.toStdString().c_str(), '\"'))
            datatype = TSTRING;
        else if (strchr(value.toStdString().c_str(), '.'))
            datatype = TDOUBLE;
        else if (isdigit(value.toStdString().c_str()[0]))
            datatype = TLONG;
        else if (strcasecmp(value.toStdString().c_str(), "TRUE") == 0
                 || strcasecmp(value.toStdString().c_str(), "FALSE") == 0)
            datatype = TLOGICAL;
        else
            datatype = TLONG;

        switch (datatype) {
        case TSTRING: {
            // remove quotes
            char *cstr = new char[value.toStdString().length() + 1];
            std::strcpy(cstr, value.toStdString().c_str());
            std::string str(cstr);
            std::string result = str.substr(1, str.size() - 2);

            fits_update_key_str(fptr, key.toStdString().c_str(), result.c_str(), "", &status);
            break;
        }
        case TFLOAT:
            fits_update_key_flt(fptr, key.toStdString().c_str(), atof(value.toStdString().c_str()),
                                -7, "", &status);
            break;
        case TDOUBLE:
            fits_update_key_dbl(fptr, key.toStdString().c_str(), atof(value.toStdString().c_str()),
                                -15, "", &status);
            break;
        case TLONG: {
            fits_update_key_lng(fptr, key.toStdString().c_str(), atol(value.toStdString().c_str()),
                                "", &status);
            break;
        }
        }
    }
    fits_close_file(fptr, &status);

    return headerFile;
}

void pqWindowImage::showLegendScaleActor()
{
    // Show Legend Scale Actor in image renderer
    auto legendActorCube = vtkSmartPointer<vtkLegendScaleActor>::New();
    legendActorCube->LegendVisibilityOff();
    legendActorCube->setFitsHeader(fitsHeaderPath.toStdString());
    auto rwImage = viewImage->getViewProxy()->GetRenderWindow()->GetRenderers();
    vtkRenderer::SafeDownCast(rwImage->GetItemAsObject(1))->AddActor(legendActorCube);
}

/**
 * @brief pqWindowImage::rescaleForLog
 * Rescales the data range to be in a format suitable for a log scaling.
 * Reused from the paraview/vtk source code.
 */
void pqWindowImage::rescaleForLog()
{
    double range[2];

    vtkSMTransferFunctionProxy::GetRange(lutProxy, range);

    if (vtkSMCoreUtilities::AdjustRangeForLog(range))
    {
        vtkGenericWarningMacro("Ranges not valid for log-space. Changed the range to ("
            << range[0] << ", " << range[1] << ").");
        vtkSMTransferFunctionProxy::RescaleTransferFunction(lutProxy, range);
    }
}

/**
 * @brief pqWindowImage::setLogScale
 * Sets the image visualization to use log10 scaling on the colour map.
 * @param logScale
 * A boolean whether or not to use log10 scaling.
 */
void pqWindowImage::setLogScale(bool logScale)
{
    //If in the process of initialising the UI, ignore this command.
    if (clmInit) return;

    if (logScale){
        logScaleActive = true;
        rescaleForLog();
        if (!vtkSMTransferFunctionProxy::MapControlPointsToLogSpace(lutProxy))
            std::cerr << "Error in mapping data to log space!" << std::endl;
    }
    else{
        logScaleActive = false;
        if (!vtkSMTransferFunctionProxy::MapControlPointsToLinearSpace(lutProxy))
            std::cerr << "Error in mapping data to linear space!" << std::endl;
        changeColorMap(ui->cmbxLUTSelect->currentText());
    }
    vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRange(lutProxy, false, false);
    lutProxy->UpdateVTKObjects();

    vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRange(imageProxy, false, false);
    imageProxy->UpdateVTKObjects();

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
    if (vtkSMProperty *opacityProperty = imageProxy->GetProperty("Opacity")) {

        vtkSMPropertyHelper(opacityProperty).Set(value);
        imageProxy->UpdateVTKObjects();

        viewImage->render();
    }
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

void pqWindowImage::readInfoFromSource()
{
    // Bounds
    auto fitsInfo = this->ImageSource->getOutputPort(0)->getDataInformation();
    fitsInfo->GetBounds(bounds);

    // Data range
    auto fitsImageInfo = fitsInfo->GetPointDataInformation()->GetArrayInformation("FITSImage");
    double dataRange[2];
    fitsImageInfo->GetComponentRange(0, dataRange);
    dataMin = dataRange[0];
    dataMax = dataRange[1];
}

void pqWindowImage::readHeaderFromSource()
{
    auto dataMoverProxy = vtk::TakeSmartPointer(serverProxyManager->NewProxy("misc", "DataMover"));
    vtkSMPropertyHelper(dataMoverProxy, "Producer").Set(this->ImageSource->getProxy());
    vtkSMPropertyHelper(dataMoverProxy, "PortNumber").Set(1);
    vtkSMPropertyHelper(dataMoverProxy, "SkipEmptyDataSets").Set(1);
    dataMoverProxy->UpdateVTKObjects();
    dataMoverProxy->InvokeCommand("Execute");

    auto dataMover = vtkPVDataMover::SafeDownCast(dataMoverProxy->GetClientSideObject());
    int datasets = dataMover->GetNumberOfDataSets();
    for (int table = 0; table < datasets; ++table) {
        vtkTable *headerTable = vtkTable::SafeDownCast(dataMover->GetDataSetAtIndex(table));
        for (vtkIdType i = 0; i < headerTable->GetNumberOfRows(); ++i) {
            fitsHeader.insert(QString::fromStdString(headerTable->GetValue(i, 0).ToString()),
                              QString::fromStdString(headerTable->GetValue(i, 1).ToString()));
        }
    }
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
    //If the UI is being initialised, ignore this command.
    if (index >= 0 && !clmInit){
        changeColorMap(ui->cmbxLUTSelect->itemText(index));
        if (logScaleActive)
            setLogScale(logScaleActive);
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
    // Check what caused the value to change, and don't update while sliding.
    if (!loadOpacityChange)
        return;

    //Opacity is from 0 to 1, slider is from 0 to 100. So divide by 100 to get value to be sent to server.
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
    //Opacity is from 0 to 1, slider is from 0 to 100. So divide by 100 to get value to be sent to server.
    float opacityValue = ui->opacitySlider->value() / 100.0;

    setOpacity(opacityValue);
    loadOpacityChange = false;
}

