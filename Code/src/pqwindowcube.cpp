#include "pqwindowcube.h"
#include "ui_pqwindowcube.h"

#include "interactors/vtkinteractorstyleimagecustom.h"
#include "vtklegendscaleactor.h"

#include <pqActiveObjects.h>
#include <pqAlwaysConnectedBehavior.h>
#include <pqApplicationCore.h>
#include <pqLoadDataReaction.h>
#include <pqObjectBuilder.h>
#include <pqPipelineSource.h>
#include <pqRenderView.h>

#include <vtkCamera.h>
#include <vtkPVArrayInformation.h>
#include <vtkPVDataInformation.h>
#include <vtkPVDataMover.h>
#include <vtkPVDataSetAttributesInformation.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMPVRepresentationProxy.h>
#include <vtkSMRenderViewProxy.h>
#include <vtkSMSessionProxyManager.h>
#include <vtkSMTransferFunctionManager.h>
#include <vtkSMTransferFunctionPresets.h>
#include <vtkSMTransferFunctionProxy.h>
#include <vtkSMUncheckedPropertyHelper.h>
#include <vtkSMViewProxy.h>
#include <vtkTable.h>

#include <QDebug>
#include <QDir>

#include <cmath>
#include <cstring>
#include <utility>

pqWindowCube::pqWindowCube(const QString &filepath, const CubeSubset &cubeSubset)
    : ui(new Ui::pqWindowCube),
      FitsFileName(QFileInfo(filepath).fileName()),
      cubeSubset(cubeSubset),
      currentSlice(-1),
      contourFilter(nullptr),
      contourFilter2D(nullptr)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(FitsFileName);
    connect(ui->actionVolGenerate, &QAction::triggered, this,
            &pqWindowCube::generateVolumeRendering);

    // Opacity menu actions
    auto opacityGroup = new QActionGroup(this);
    opacityGroup->addAction(ui->action0);
    opacityGroup->addAction(ui->action25);
    opacityGroup->addAction(ui->action50);
    opacityGroup->addAction(ui->action75);
    opacityGroup->addAction(ui->action100);

    // Create LUTs menu actions
    auto presets = vtkSMTransferFunctionPresets::GetInstance();
    auto lutGroup = new QActionGroup(this);
    for (int i = 0; i < presets->GetNumberOfPresets(); ++i) {
        QString name = QString::fromStdString(presets->GetPresetName(i));
        auto lut = new QAction(name, lutGroup);
        lut->setCheckable(true);
        if (name == "Grayscale")
            lut->setChecked(true);

        lutGroup->addAction(lut);
        connect(lut, &QAction::triggered, this, [=]() { changeColorMap(name); });
    }
    ui->menuColorMap->addActions(lutGroup->actions());

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
    CubeSource = pqLoadDataReaction::loadData({ filepath });
    SliceSource = pqLoadDataReaction::loadData({ filepath });

    // Handle Subset selection
    setSubsetProperties(cubeSubset);

    createViews();
    showOutline();

    // Fetch information from source and header and update UI
    readInfoFromSource();
    readHeaderFromSource();
    rms = getRMSFromHeader(fitsHeader);
    lowerBound = 3 * rms;
    upperBound = dataMax;
    ui->lowerBoundText->setText(QString::number(lowerBound, 'f', 4));
    ui->upperBoundText->setText(QString::number(upperBound, 'f', 4));
    ui->minCubeText->setText(QString::number(dataMin, 'f', 4));
    ui->maxCubeText->setText(QString::number(dataMax, 'f', 4));
    ui->rmsCubeText->setText(QString::number(rms, 'f', 4));

    // Show Legend Scale Actors in both renderers
    fitsHeaderPath = createFitsHeaderFile(fitsHeader);
    showLegendScaleActors();

    // Show slice and set default LUT
    showSlice();
    setSliceDatacube(1);
    changeColorMap("Grayscale");

    // Interactor to show pixel coordinates in the status bar
    vtkNew<vtkInteractorStyleImageCustom> interactorStyle;
    interactorStyle->SetCoordsCallback(
            [this](const std::string &str) { showStatusBarMessage(str); });
    interactorStyle->SetLayerFitsReaderFunc(fitsHeaderPath.toStdString());
    interactorStyle->SetPixelZCompFunc([this]() { return currentSlice; });
    viewSlice->getViewProxy()->GetRenderWindow()->GetInteractor()->SetInteractorStyle(
            interactorStyle);

    viewSlice->resetDisplay();
    viewCube->resetDisplay();
    viewSlice->render();
    viewCube->render();
}

pqWindowCube::~pqWindowCube()
{
    builder->destroy(CubeSource);
    builder->destroy(SliceSource);
    builder->destroySources(server);
    this->CubeSource = NULL;
    this->SliceSource = NULL;
    delete ui;
}

void pqWindowCube::readInfoFromSource()
{
    // Bounds
    auto fitsInfo = this->CubeSource->getOutputPort(0)->getDataInformation();
    fitsInfo->GetBounds(bounds);

    // Data range
    auto fitsImageInfo = fitsInfo->GetPointDataInformation()->GetArrayInformation("FITSImage");
    double dataRange[2];
    fitsImageInfo->GetComponentRange(0, dataRange);
    dataMin = dataRange[0];
    dataMax = dataRange[1];
}

void pqWindowCube::readHeaderFromSource()
{
    auto dataMoverProxy = vtk::TakeSmartPointer(serverProxyManager->NewProxy("misc", "DataMover"));
    vtkSMPropertyHelper(dataMoverProxy, "Producer").Set(this->CubeSource->getProxy());
    vtkSMPropertyHelper(dataMoverProxy, "PortNumber").Set(1);
    vtkSMPropertyHelper(dataMoverProxy, "SkipEmptyDataSets").Set(1);
    dataMoverProxy->UpdateVTKObjects();
    dataMoverProxy->InvokeCommand("Execute");

    auto dataMover = vtkPVDataMover::SafeDownCast(dataMoverProxy->GetClientSideObject());
    for (int table = 0; table < dataMover->GetNumberOfDataSets(); ++table) {
        vtkTable *headerTable = vtkTable::SafeDownCast(dataMover->GetDataSetAtIndex(table));
        for (vtkIdType i = 0; i < headerTable->GetNumberOfRows(); ++i) {
            fitsHeader.insert(QString::fromStdString(headerTable->GetValue(i, 0).ToString()),
                              QString::fromStdString(headerTable->GetValue(i, 1).ToString()));
        }
    }
}

QString pqWindowCube::createFitsHeaderFile(const FitsHeaderMap &fitsHeader)
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

double pqWindowCube::getRMSFromHeader(const FitsHeaderMap &fitsHeader)
{
    if (fitsHeader.contains("RMS")) {
        return fitsHeader.value("RMS").toDouble();
    }

    int n = fitsHeader.value("MSn").toInt();
    if (n <= 0) {
        qDebug() << Q_FUNC_INFO
                 << "Missing required header keywords to calculate RMS from partial values";
        return 0.0;
    }

    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        QString keyword("MS");
        keyword.append(QString::number(i));
        sum += fitsHeader.value(keyword).toDouble();
    }

    return std::sqrt(sum / n);
}

void pqWindowCube::createViews()
{
    viewCube = qobject_cast<pqRenderView *>(
            builder->createView(pqRenderView::renderViewType(), server));
    viewCube->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    viewSlice = qobject_cast<pqRenderView *>(
            builder->createView(pqRenderView::renderViewType(), server));
    viewSlice->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->PVLayout->addWidget(viewCube->widget());
    ui->PVLayout->addWidget(viewSlice->widget());
}

void pqWindowCube::showOutline()
{
    auto outline =
            builder->createDataRepresentation(CubeSource->getOutputPort(0), viewCube)->getProxy();
    vtkSMPropertyHelper(outline, "Representation").Set("Outline");
    double red[3] = { 1.0, 0.0, 0.0 };
    vtkSMPropertyHelper(outline, "AmbientColor").Set(red, 3);
    outline->UpdateVTKObjects();
}

void pqWindowCube::showLegendScaleActors()
{
    // Show Legend Scale Actor in both renderers
    auto legendActorCube = vtkSmartPointer<vtkLegendScaleActor>::New();
    legendActorCube->LegendVisibilityOff();
    legendActorCube->setFitsHeader(fitsHeaderPath.toStdString());
    auto rwCube = viewCube->getViewProxy()->GetRenderWindow()->GetRenderers();
    vtkRenderer::SafeDownCast(rwCube->GetItemAsObject(1))->AddActor(legendActorCube);

    auto legendActorSlice = vtkSmartPointer<vtkLegendScaleActor>::New();
    legendActorSlice->LegendVisibilityOff();
    legendActorSlice->setFitsHeader(fitsHeaderPath.toStdString());
    auto rwSlice = viewSlice->getViewProxy()->GetRenderWindow()->GetRenderers();
    rwSlice->GetFirstRenderer()->AddActor(legendActorSlice);
    vtkRenderer::SafeDownCast(rwSlice->GetItemAsObject(1))->AddActor(legendActorSlice);
}

void pqWindowCube::showSlice()
{
    // Slice in Cube Renderer
   
    
    cubeSliceProxy = builder->createDataRepresentation(this->CubeSource->getOutputPort(0), viewCube)
                             ->getProxy();
    vtkSMPropertyHelper(cubeSliceProxy, "Representation").Set("Slice");
    vtkSMPVRepresentationProxy::SetScalarColoring(cubeSliceProxy, "FITSImage",
                                                  vtkDataObject::POINT);
    cubeSliceProxy->UpdateVTKObjects();

    // vtkNew<vtkSMTransferFunctionManager> mgr;
    // lutProxy = vtkSMTransferFunctionProxy::SafeDownCast(
    //                                                     mgr->GetColorTransferFunction("FITSImage",
    // cubeSliceProxy->GetSessionProxyManager()));

    // Filter to extract the slice (used for slice data range and contour 2D)
    // extractGrid = builder->createFilter("filters", "ExtractGrid", this->CubeSource);

    // Setup reader to get a slice using SubExtent Property
    int extent[6] = { -1, -1, -1, -1, 0, 0 };
    auto slicePropertyProxy = this->SliceSource->getProxy();
    vtkSMPropertyHelper(slicePropertyProxy, "ReadSubExtent").Set(true);
    vtkSMPropertyHelper(slicePropertyProxy, "SubExtent").Set(extent, 6);
    slicePropertyProxy->UpdateVTKObjects();
    SliceSource->updatePipeline();

    // Slice Render (right view)
    sliceProxy = builder->createDataRepresentation(this->SliceSource->getOutputPort(0), viewSlice)
                         ->getProxy();
    vtkSMPropertyHelper(sliceProxy, "Representation").Set("Slice");
    vtkSMPVRepresentationProxy::SetScalarColoring(sliceProxy, "FITSImage", vtkDataObject::POINT);
    vtkNew<vtkSMTransferFunctionManager> mgr;
    lutProxy = vtkSMTransferFunctionProxy::SafeDownCast(
            mgr->GetColorTransferFunction("FITSImage", sliceProxy->GetSessionProxyManager()));

    ui->sliceSlider->setRange(1, bounds[5] + 1);
    ui->sliceSpinBox->setRange(1, bounds[5] + 1);
}

void pqWindowCube::showStatusBarMessage(const std::string &msg)
{
    ui->statusBar->showMessage(QString::fromStdString(msg));
}

void pqWindowCube::setSubsetProperties(const CubeSubset &subset)
{
    auto sourceProxy = CubeSource->getProxy();
    vtkSMPropertyHelper(sourceProxy, "ReadSubExtent").Set(subset.ReadSubExtent);
    vtkSMPropertyHelper(sourceProxy, "SubExtent").Set(subset.SubExtent, 6);
    vtkSMPropertyHelper(sourceProxy, "AutoScale").Set(subset.AutoScale);
    vtkSMPropertyHelper(sourceProxy, "CubeMaxSize").Set(subset.CubeMaxSize);
    vtkSMPropertyHelper(sourceProxy, "ScaleFactor").Set(subset.ScaleFactor);
    sourceProxy->UpdateVTKObjects();
}

void pqWindowCube::updateVelocityText()
{
    double crval3 = fitsHeader.value("CRVAL3").toDouble();
    double cdelt3 = fitsHeader.value("CDELT3").toDouble();
    double crpix3 = fitsHeader.value("CRPIX3").toDouble();

    double initSlice = crval3 - (cdelt3 * (crpix3 - 1));
    double velocity = initSlice + cdelt3 * currentSlice;

    if (fitsHeader.value("CUNIT3").startsWith("m")) {
        // Return value in km/s
        velocity /= 1000;
    }
    ui->velocityText->setText(QString::number(velocity).append(" Km/s"));
}

void pqWindowCube::setThreshold(double threshold)
{
    auto filterProxy = contourFilter->getProxy();
    vtkSMPropertyHelper(filterProxy, "ContourValues").Set(threshold);
    filterProxy->UpdateVTKObjects();
    contourFilter->updatePipeline();
    viewCube->render();
}

void pqWindowCube::showContours()
{
    removeContours();

    contourFilter2D = builder->createFilter("filters", "Contour", this->SliceSource);
    auto contourProxy = contourFilter2D->getProxy();

    int level = ui->levelText->text().toInt();
    double min = ui->lowerBoundText->text().toDouble();
    double max = ui->upperBoundText->text().toDouble();

    if (level == 1) {
        vtkSMPropertyHelper(contourFilter2D->getProxy(), "ContourValues").Set(min);
    } else {
        for (int i = 0; i < level; ++i) {
            double val = min + i * (max - min) / (level - 1);
            vtkSMPropertyHelper(contourFilter2D->getProxy(), "ContourValues").Set(i, val);
        }
    }

    contourProxy->UpdateVTKObjects();
    contourFilter2D->updatePipeline();

    auto contourSurface =
            builder->createDataRepresentation(contourFilter2D->getOutputPort(0), viewSlice)
                    ->getProxy();
    vtkSMPropertyHelper(contourSurface, "Representation").Set("Surface");
    auto separateProperty = vtkSMPVRepresentationProxy::SafeDownCast(contourSurface)
                                    ->GetProperty("UseSeparateColorMap");
    vtkSMPropertyHelper(separateProperty).Set(1);
    vtkSMPVRepresentationProxy::SetScalarColoring(contourSurface, "FITSImage",
                                                  vtkDataObject::POINT);

    contourSurface->UpdateVTKObjects();
    viewSlice->render();
}

void pqWindowCube::removeContours()
{
    if (contourFilter2D) {
        builder->destroy(contourFilter2D);
        contourFilter2D = nullptr;
        viewSlice->render();
    }
}

void pqWindowCube::on_sliceSlider_sliderReleased()
{
    int value = ui->sliceSlider->value();

    // Match slider and spinbox values
    if (ui->sliceSpinBox->value() != value) {
        ui->sliceSpinBox->setValue(value);
    }

    setSliceDatacube(value);
}

void pqWindowCube::on_sliceSlider_valueChanged()
{
    if (!loadChange)
        return;

    int value = ui->sliceSlider->value();

    // Match slider and spinbox values
    if (ui->sliceSpinBox->value() != value) {
        ui->sliceSpinBox->setValue(value);
    }

    setSliceDatacube(value);
    loadChange = false;
}

void pqWindowCube::on_sliceSlider_actionTriggered(int action)
{
    switch (action) {
    case 0:
        loadChange = false;
        break;
    case 7:
        loadChange = false;
        break;
    default:
        loadChange = true;
        break;
    }
}

void pqWindowCube::on_sliceSpinBox_valueChanged(int arg1){
    // Match slider and spinbox values
    if (ui->sliceSlider->value() != arg1) {
        ui->sliceSlider->setValue(arg1);
        ui->sliceSlider->update();
    }
    
    setSliceDatacube(arg1);
}

void pqWindowCube::on_sliceSpinBox_editingFinished()
{
    int value = ui->sliceSpinBox->value();

    // Match slider and spinbox values
    if (ui->sliceSlider->value() != value) {
        ui->sliceSlider->setValue(value);
        ui->sliceSlider->update();
    }

    setSliceDatacube(value);
}

void pqWindowCube::setSliceDatacube(int value)
{
    if (currentSlice == (value - 1)) {
        // No need to update the slice
        return;
    }

    currentSlice = value - 1;
    updateVelocityText();

    // vtkSMPropertyHelper(sliceProxy, "Slice").Set(currentSlice);
    // vtkSMPropertyHelper(sliceProxy, "SliceMode").Set(VTK_XY_PLANE);
    //  Setup reader to get a slice using SubExtent Property
    int extent[6] = { -1, -1, -1, -1, currentSlice, currentSlice };
    auto slicePropertyProxy = this->SliceSource->getProxy();
    vtkSMPropertyHelper(slicePropertyProxy, "ReadSubExtent").Set(true);
    vtkSMPropertyHelper(slicePropertyProxy, "SubExtent").Set(extent, 6);
    slicePropertyProxy->UpdateVTKObjects();
    sliceProxy->UpdateVTKObjects();
    SliceSource->updatePipeline();

    int slicePos = std::round(((float) currentSlice) / cubeSubset.ScaleFactor);
    vtkSMPropertyHelper(cubeSliceProxy, "Slice").Set(slicePos);
    vtkSMPropertyHelper(cubeSliceProxy, "SliceMode").Set(VTK_XY_PLANE);
    cubeSliceProxy->UpdateVTKObjects();

    // Get slice data range and update UI
    /*
    auto extractGridProxy = extractGrid->getProxy();
    int selectedSlice[] = { (int)bounds[0], (int)bounds[1], (int)bounds[2],
                            (int)bounds[3], (currentSlice), (currentSlice) };
    vtkSMPropertyHelper(extractGridProxy, "VOI").Set(selectedSlice, 6);
    extractGridProxy->UpdateVTKObjects();
    extractGrid->updatePipeline();
    */

    auto dataInformation = this->SliceSource->getOutputPort(0)->getDataInformation();
    auto fitsImageInfo =
            dataInformation->GetPointDataInformation()->GetArrayInformation("FITSImage");
    double dataRange[2];
    fitsImageInfo->GetComponentRange(0, dataRange);
    ui->minSliceText->setText(QString::number(dataRange[0], 'f', 4));
    ui->maxSliceText->setText(QString::number(dataRange[1], 'f', 4));
    lutProxy->RescaleTransferFunction(dataRange, true);

    viewSlice->resetDisplay();
    viewSlice->render();
    viewCube->render();

    if (ui->contourCheckBox->isChecked()) {
        showContours();
    }
}

void pqWindowCube::generateVolumeRendering()
{
    if (contourFilter != nullptr) {
        return;
    }

    // Default contour value is lowerBound (RMS*3)
    contourFilter = builder->createFilter("filters", "Contour", this->CubeSource);
    ui->thresholdText->setText(QString::number(lowerBound, 'f', 4));
    setThreshold(lowerBound);

    contourProxy = builder->createDataRepresentation(contourFilter->getOutputPort(0), viewCube)
                           ->getProxy();
    vtkSMPVRepresentationProxy::SetScalarColoring(contourProxy, nullptr, 0);
    vtkSMPropertyHelper(contourProxy, "Representation").Set("Surface");
    vtkSMPropertyHelper(contourProxy, "Ambient").Set(0.5);
    vtkSMPropertyHelper(contourProxy, "Diffuse").Set(0.5);
    vtkSMPropertyHelper(contourProxy, "Opacity").Set(1);
    double red[3] = { 1.0, 0.0, 0.0 };
    vtkSMPropertyHelper(contourProxy, "AmbientColor").Set(red, 3);
    contourProxy->UpdateVTKObjects();
    viewCube->render();
}

void pqWindowCube::changeColorMap(const QString &name)
{
    if (vtkSMProperty *lutProperty = sliceProxy->GetProperty("LookupTable")) {

        auto presets = vtkSMTransferFunctionPresets::GetInstance();
        lutProxy->ApplyPreset(presets->GetFirstPresetWithName(name.toStdString().c_str()));
        vtkSMPropertyHelper(lutProperty).Set(lutProxy);
        lutProxy->UpdateVTKObjects();
        vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRange(sliceProxy, false, false);
        sliceProxy->UpdateVTKObjects();

        viewSlice->resetDisplay();
        viewSlice->render();
        viewCube->render();
    }
}

void pqWindowCube::on_actionFront_triggered()
{
    // Negative Z
    viewCube->resetViewDirection(0, 0, -1, 0, 1, 0);
    viewCube->render();
}

void pqWindowCube::on_actionBack_triggered()
{
    // Positive Z
    viewCube->resetViewDirection(0, 0, 1, 0, 1, 0);
    viewCube->render();
}

void pqWindowCube::on_actionTop_triggered()
{
    // Negative Y
    viewCube->resetViewDirection(0, -1, 0, 0, 0, 1);
    viewCube->render();
}

void pqWindowCube::on_actionRight_triggered()
{
    // Negative X, rotation 90
    viewCube->resetViewDirection(-1, 0, 0, 0, 0, 1);
    viewCube->getRenderViewProxy()->GetActiveCamera()->Roll(90);
    viewCube->render();
}

void pqWindowCube::on_actionBottom_triggered()
{
    // Positive Y
    viewCube->resetViewDirection(0, 1, 0, 0, 0, 1);
    viewCube->render();
}

void pqWindowCube::on_actionLeft_triggered()
{
    // Positive X, rotation -90
    viewCube->resetViewDirection(1, 0, 0, 0, 0, 1);
    viewCube->getRenderViewProxy()->GetActiveCamera()->Roll(-90);
    viewCube->render();
}

void pqWindowCube::on_thresholdText_editingFinished()
{
    if (contourFilter == nullptr) {
        return;
    }

    double threshold = ui->thresholdText->text().toDouble();
    // Clamp threshold
    threshold = fmin(fmax(threshold, lowerBound), upperBound);
    ui->thresholdText->setText(QString::number(threshold, 'f', 4));

    int tickPosition = 100 * (threshold - lowerBound) / (upperBound - lowerBound);
    ui->thresholdSlider->setValue(tickPosition);

    setThreshold(threshold);
}

void pqWindowCube::on_thresholdSlider_sliderReleased()
{
    if (contourFilter == nullptr) {
        return;
    }

    double threshold =
            (ui->thresholdSlider->value() * (upperBound - lowerBound) / 100) + lowerBound;
    ui->thresholdText->setText(QString::number(threshold, 'f', 4));
    setThreshold(threshold);
}

void pqWindowCube::on_contourCheckBox_toggled(bool checked)
{
    if (checked) {
        showContours();
    } else {
        removeContours();
    }
}

void pqWindowCube::on_levelText_editingFinished()
{
    if (ui->contourCheckBox->isChecked()) {
        showContours();
    }
}

void pqWindowCube::on_lowerBoundText_editingFinished()
{
    if (ui->contourCheckBox->isChecked()) {
        showContours();
    }
}

void pqWindowCube::on_upperBoundText_editingFinished()
{
    if (ui->contourCheckBox->isChecked()) {
        showContours();
    }
}

void pqWindowCube::on_action0_triggered()
{
    setVolumeRenderingOpacity(0);
}

void pqWindowCube::on_action25_triggered()
{
    setVolumeRenderingOpacity(0.25);
}

void pqWindowCube::on_action50_triggered()
{
    setVolumeRenderingOpacity(0.5);
}

void pqWindowCube::on_action75_triggered()
{
    setVolumeRenderingOpacity(0.75);
}

void pqWindowCube::on_action100_triggered()
{
    setVolumeRenderingOpacity(1);
}

void pqWindowCube::setVolumeRenderingOpacity(double opacity)
{
    vtkSMPropertyHelper(contourProxy, "Opacity").Set(opacity);
    contourProxy->UpdateVTKObjects();
    viewCube->render();
}

