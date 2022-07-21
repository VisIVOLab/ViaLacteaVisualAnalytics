#include "vtkwindowcube.h"
#include "ui_vtkwindowcube.h"

#include "interactors/vtkinteractorstyleimagecustom.h"

#include "astroutils.h"
#include "luteditor.h"
#include "vtkfitsreader.h"
#include "vtklegendscaleactor.h"

#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCaptionActor2D.h>
#include <vtkContourFilter.h>
#include <vtkCutter.h>
#include <vtkDoubleArray.h>
#include <vtkFlyingEdges3D.h>
#include <vtkFrustumSource.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageActor.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkLookupTable.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkOutlineFilter.h>
#include <vtkPlane.h>
#include <vtkPlanes.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkPVLODActor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkResliceImageViewer.h>
#include <vtkTextActor.h>
#include <vtkTransform.h>

#include <cmath>

#include "singleton.h"
#include "vialactea.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqRenderView.h"

#include "vtkDataObject.h"
#include "vtkSMParaViewPipelineControllerWithRendering.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxySelectionModel.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMReaderFactory.h"
#include "vtkSMSessionProxyManager.h"

#include "pqAlwaysConnectedBehavior.h"
#include "pqPersistentMainWindowStateBehavior.h"

#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqSMAdaptor.h"

#include "vtkPVArrayInformation.h"
#include "vtkPVDataInformation.h"
#include "vtkPVDataSetAttributesInformation.h"
#include "vtkPVRenderView.h"

#include "vtkPVDataMover.h"
#include "vtkSMTransferFunctionManager.h"
#include "vtkSMTransferFunctionPresets.h"
#include "vtkSMTransferFunctionProxy.h"

#include "vtkSMTooltipSelectionPipeline.h"

#include "vtkImageSliceRepresentation.h"
#include "vtkSMViewProxy.h"

#include "pqActiveObjects.h"

#include "mainwindow.h"
#include "vtkSMProperty.h"
#include <QDebug>

vtkWindowCube::vtkWindowCube(QWidget *parent, vtkSmartPointer<vtkFitsReader> fitsReader,
                             QString velocityUnit)
    : QMainWindow(parent),
      ui(new Ui::vtkWindowCube),
      fitsReader(fitsReader),
      parentWindow(qobject_cast<vtkWindowImage *>(parent)),
      currentSlice(0),
      velocityUnit(velocityUnit)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(QString::fromStdString(fitsReader->GetFileName()));

    fitsReader->is3D = true;
    fitsReader->CalculateRMS();

    lowerBound = 3 * fitsReader->GetRMS();
    upperBound = fitsReader->GetMax();

    // Start datacube pipeline
    auto renWinCube = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->qVtkCube->setDefaultCursor(Qt::ArrowCursor);
    ui->qVtkCube->setRenderWindow(renWinCube);

    auto rendererCube = vtkSmartPointer<vtkRenderer>::New();
    rendererCube->SetBackground(0.21, 0.23, 0.25);
    rendererCube->GlobalWarningDisplayOff();
    renWinCube->AddRenderer(rendererCube);

    ui->thresholdText->setText(QString::number(lowerBound, 'f', 4));
    ui->lowerBoundText->setText(QString::number(lowerBound, 'f', 4));
    ui->upperBoundText->setText(QString::number(upperBound, 'f', 4));
    ui->minCubeText->setText(QString::number(fitsReader->GetMin(), 'f', 4));
    ui->maxCubeText->setText(QString::number(fitsReader->GetMax(), 'f', 4));
    ui->rmsCubeText->setText(QString::number(fitsReader->GetRMS(), 'f', 4));

    // Outline
    auto outlineFilter = vtkSmartPointer<vtkOutlineFilter>::New();
    outlineFilter->SetInputData(fitsReader->GetOutput());
    auto outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    outlineMapper->SetInputConnection(outlineFilter->GetOutputPort());
    outlineMapper->ScalarVisibilityOff();
    auto outlineActor = vtkSmartPointer<vtkActor>::New();
    outlineActor->SetMapper(outlineMapper);

    // Isosurface
    isosurface = vtkSmartPointer<vtkFlyingEdges3D>::New();
    isosurface->SetInputData(fitsReader->GetOutput());
    isosurface->ComputeNormalsOn();
    isosurface->SetValue(0, lowerBound);
    auto isosurfaceMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    isosurfaceMapper->SetInputConnection(isosurface->GetOutputPort());
    isosurfaceMapper->ScalarVisibilityOff();
    auto isosurfaceActor = vtkSmartPointer<vtkActor>::New();
    isosurfaceActor->SetMapper(isosurfaceMapper);
    isosurfaceActor->GetProperty()->SetColor(1.0, 0.5, 1.0);

    // Plane
    double *bounds = fitsReader->GetOutput()->GetBounds();
    auto planes = vtkSmartPointer<vtkPlanes>::New();
    planes->SetBounds(bounds[0], bounds[1], bounds[2], bounds[3], 0, 1);
    auto frustumSource = vtkSmartPointer<vtkFrustumSource>::New();
    frustumSource->ShowLinesOff();
    frustumSource->SetPlanes(planes);
    frustumSource->Update();
    auto planeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    planeMapper->SetInputData(frustumSource->GetOutput());
    planeActor = vtkSmartPointer<vtkActor>::New();
    planeActor->SetMapper(planeMapper);

    // Axes
    auto axesActor = vtkSmartPointer<vtkAxesActor>::New();
    axesActor->SetXAxisLabelText("X");
    axesActor->SetYAxisLabelText("Y");
    axesActor->SetZAxisLabelText("Z");
    axesActor->DragableOn();
    axesActor->GetXAxisCaptionActor2D()->GetTextActor()->SetTextScaleMode(0);
    axesActor->GetYAxisCaptionActor2D()->GetTextActor()->SetTextScaleMode(0);
    axesActor->GetZAxisCaptionActor2D()->GetTextActor()->SetTextScaleMode(0);
    axesWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    axesWidget->SetInteractor(renWinCube->GetInteractor());
    axesWidget->SetOrientationMarker(axesActor);
    axesWidget->SetOutlineColor(0.9300, 0.5700, 0.1300);
    axesWidget->SetViewport(0.0, 0.0, 0.2, 0.2);
    axesWidget->SetEnabled(1);
    axesWidget->InteractiveOff();

    // Legend
    auto legendActorCube = vtkSmartPointer<vtkLegendScaleActor>::New();
    legendActorCube->LegendVisibilityOff();
    legendActorCube->setFitsFile(fitsReader);

    rendererCube->GetActiveCamera()->GetPosition(initialCameraPosition);
    rendererCube->GetActiveCamera()->GetFocalPoint(initialCameraFocalPoint);

    rendererCube->AddActor(outlineActor);
    rendererCube->AddActor(isosurfaceActor);
    rendererCube->AddActor(planeActor);
    rendererCube->AddActor(legendActorCube);

    rendererCube->ResetCamera();
    renWinCube->GetInteractor()->Render();
    // End datacube pipeline

    // Start slice pipeline
    auto renWinSlice = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->qVtkSlice->setDefaultCursor(Qt::ArrowCursor);
    ui->qVtkSlice->setRenderWindow(renWinSlice);

    auto rendererSlice = vtkSmartPointer<vtkRenderer>::New();
    rendererSlice->SetBackground(0.21, 0.23, 0.25);
    rendererSlice->GlobalWarningDisplayOff();
    renWinSlice->AddRenderer(rendererSlice);

    auto lutSlice = vtkSmartPointer<vtkLookupTable>::New();
    lutSlice->SetTableRange(fitsReader->GetRangeSlice(0)[0], fitsReader->GetRangeSlice(0)[1]);
    SelectLookTable("Gray", lutSlice);

    sliceViewer = vtkSmartPointer<vtkResliceImageViewer>::New();
    sliceViewer->SetInputData(fitsReader->GetOutput());
    sliceViewer->GetWindowLevel()->SetOutputFormatToRGB();
    sliceViewer->GetWindowLevel()->SetLookupTable(lutSlice);
    sliceViewer->GetImageActor()->InterpolateOff();
    sliceViewer->SetRenderWindow(renWinSlice);
    sliceViewer->SetRenderer(rendererSlice);

    ui->sliceSlider->setRange(1, fitsReader->GetNaxes(2));
    ui->sliceSpinBox->setRange(1, fitsReader->GetNaxes(2));

    // Added to renderers when the contour checkbox is checked
    contoursActor = vtkSmartPointer<vtkPVLODActor>::New();
    contoursActor->GetProperty()->SetLineWidth(1);
    contoursActorForParent = vtkSmartPointer<vtkPVLODActor>::New();
    contoursActorForParent->GetProperty()->SetLineWidth(1);

    auto legendActorSlice = vtkSmartPointer<vtkLegendScaleActor>::New();
    legendActorSlice->LegendVisibilityOff();
    legendActorSlice->setFitsFile(fitsReader);
    rendererSlice->AddActor(legendActorSlice);

    auto interactorStyle = vtkSmartPointer<vtkInteractorStyleImageCustom>::New();
    interactorStyle->SetCoordsCallback([this](std::string str) { showStatusBarMessage(str); });
    interactorStyle->SetLayerFitsReaderFunc(
            [this]() -> vtkSmartPointer<vtkFitsReader> { return this->fitsReader; });
    interactorStyle->SetPixelZCompFunc([this]() -> int { return this->sliceViewer->GetSlice(); });
    ui->qVtkSlice->renderWindow()->GetInteractor()->SetInteractorStyle(interactorStyle);

    rendererSlice->ResetCamera();
    renWinSlice->GetInteractor()->Render();
}

// paraview
vtkWindowCube::vtkWindowCube(QPointer<pqPipelineSource> fitsSource, std::string fn)
    : ui(new Ui::vtkWindowCube)
{
    MainWindow *w = &Singleton<MainWindow>::Instance();

    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("paraview");

    // temp hack, if those two widget are in place
    ui->qVtkCube->hide();
    ui->qVtkSlice->hide();

    ui->action100->setChecked(true);

    this->filename = fn;

    this->FitsSource = fitsSource;

    new pqAlwaysConnectedBehavior(this);
    new pqPersistentMainWindowStateBehavior(this);

    viewCube = qobject_cast<pqRenderView *>(
            pqApplicationCore::instance()->getObjectBuilder()->createView(
                    pqRenderView::renderViewType(), pqActiveObjects::instance().activeServer()));
    pqActiveObjects::instance().setActiveView(viewCube);
    viewCube->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->PVLayout->addWidget(viewCube->widget());

    viewSlice = qobject_cast<pqRenderView *>(
            pqApplicationCore::instance()->getObjectBuilder()->createView(
                    pqRenderView::renderViewType(), pqActiveObjects::instance().activeServer()));
    pqActiveObjects::instance().setActiveView(viewSlice);
    viewSlice->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->PVLayout->addWidget(viewSlice->widget());

    vtkNew<vtkSMParaViewPipelineControllerWithRendering> controller;
    vtkSMSessionProxyManager *pxm = Singleton<ViaLactea>::Instance().getServer()->proxyManager();

    auto renderingSettings = pxm->GetProxy("settings", "RenderViewSettings");
    vtkSMPropertyHelper(renderingSettings, "ShowAnnotation").Set(1);

    pqActiveObjects::instance().setActiveSource(this->FitsSource);
    pqObjectBuilder *builder = pqApplicationCore::instance()->getObjectBuilder();

    // Outline
    auto drepOutline =
            builder->createDataRepresentation(this->FitsSource->getOutputPort(0), viewCube);
    auto reprProxyOutline = drepOutline->getProxy();
    vtkSMPropertyHelper(reprProxyOutline, "Representation").Set("Outline");
    vtkSMPropertyHelper(reprProxyOutline, "LineWidth").Set(1);
    double red[3] = { 1.0, 0.0, 0.0 };
    vtkSMPropertyHelper(reprProxyOutline, "AmbientColor").Set(red, 3);
    reprProxyOutline->UpdateVTKObjects();

    auto fitsInfo = this->FitsSource->getOutputPort(0)->getDataInformation();
    auto fitsImageInfo = fitsInfo->GetPointDataInformation()->GetArrayInformation("FITSImage");
    double dataRange[2];
    bounds[0] = { 0 };
    fitsInfo->GetBounds(bounds);
    fitsImageInfo->GetComponentRange(0, dataRange);
    dataMin = dataRange[0];
    dataMax = dataRange[1];

    // get header from server to client, adapting
    // vtkSMTooltipSelectionPipeline::ConnectPVMoveSelectionToClient(
    auto dataMover = vtk::TakeSmartPointer(pxm->NewProxy("misc", "DataMover"));
    vtkSMPropertyHelper(dataMover, "Producer").Set(this->FitsSource->getProxy());
    vtkSMPropertyHelper(dataMover, "PortNumber").Set(static_cast<int>(1));
    vtkSMPropertyHelper(dataMover, "SkipEmptyDataSets").Set(1);
    dataMover->UpdateVTKObjects();
    dataMover->InvokeCommand("Execute");

    auto dataMover2 = vtkPVDataMover::SafeDownCast(dataMover->GetClientSideObject());
    for (int table = 0; table < dataMover2->GetNumberOfDataSets(); ++table) {
        vtkTable *headerTable = vtkTable::SafeDownCast(dataMover2->GetDataSetAtIndex(table));
        for (vtkIdType i = 0; i < headerTable->GetNumberOfRows(); i++) {
            headerMap.insert(QString::fromStdString(headerTable->GetValue(i, 0).ToString()),
                             QString::fromStdString(headerTable->GetValue(i, 1).ToString()));
        }
    }

    QString fh = createFitsHeader(headerMap);

    auto legendActorCube = vtkSmartPointer<vtkLegendScaleActor>::New();
    legendActorCube->LegendVisibilityOff();
    legendActorCube->setFitsHeader(fh.toStdString());

    auto rw = viewCube->getViewProxy()->GetRenderWindow()->GetRenderers();
    vtkRenderer::SafeDownCast(rw->GetItemAsObject(1))->AddActor(legendActorCube);

    auto legendActorSlice = vtkSmartPointer<vtkLegendScaleActor>::New();
    legendActorSlice->LegendVisibilityOff();
    legendActorSlice->setFitsHeader(fh.toStdString());

    auto rwSlice = viewSlice->getViewProxy()->GetRenderWindow()->GetRenderers();
    rwSlice->GetFirstRenderer()->AddActor(legendActorSlice);
    vtkRenderer::SafeDownCast(rwSlice->GetItemAsObject(1))->AddActor(legendActorSlice);

    rms = readRMSFromHeader(headerMap);
    lowerBound = 3 * rms;
    upperBound = dataMax;

    // Contour Filter
    contourFilter = builder->createFilter("filters", "Contour", this->FitsSource);
    auto reprSurface = builder->createDataRepresentation(contourFilter->getOutputPort(0), viewCube);
    reprProxySurface = reprSurface->getProxy();
    vtkSMPVRepresentationProxy::SetScalarColoring(reprProxySurface, nullptr, 0);
    vtkSMPropertyHelper(reprProxySurface, "Representation").Set("Surface");
    reprProxySurface->UpdateVTKObjects();
    ui->thresholdText->setText(QString::number(lowerBound, 'f', 4));
    setThreshold(lowerBound);
    vtkSMPropertyHelper(reprProxySurface, "Ambient").Set(0.5);
    vtkSMPropertyHelper(reprProxySurface, "Diffuse").Set(0.5);
    vtkSMPropertyHelper(reprProxySurface, "AmbientColor").Set(red, 3);
    setVolumeRenderingOpacity(1);
    reprProxySurface->UpdateVTKObjects();

    // Slice
    drepSlice = builder->createDataRepresentation(this->FitsSource->getOutputPort(0), viewSlice);
    auto reprProxySlice = drepSlice->getProxy();
    vtkSMPropertyHelper(reprProxySlice, "Representation").Set("Slice");
    vtkSMPVRepresentationProxy::SetScalarColoring(reprProxySlice, "FITSImage",
                                                  vtkDataObject::POINT);

    // Slice
    drepSliceCube = builder->createDataRepresentation(this->FitsSource->getOutputPort(0), viewCube);
    auto reprProxySliceCube = drepSliceCube->getProxy();
    vtkSMPropertyHelper(reprProxySliceCube, "Representation").Set("Slice");
    vtkSMPVRepresentationProxy::SetScalarColoring(reprProxySliceCube, "FITSImage",
                                                  vtkDataObject::POINT);
    vtkSMPropertyHelper helper(reprProxySliceCube, "ColorArrayName");
    const char *arrayName = helper.GetInputArrayNameToProcess();
    vtkNew<vtkSMTransferFunctionManager> mgr;

    lutProxy = vtkSMTransferFunctionProxy::SafeDownCast(
            mgr->GetColorTransferFunction(arrayName, reprProxySliceCube->GetSessionProxyManager()));

    auto presets = vtkSMTransferFunctionPresets::GetInstance();
    QActionGroup *lutGroup = new QActionGroup(this);
    for (int i = 0; i < presets->GetNumberOfPresets(); i++) {
        QString name = presets->GetPresetName(i).c_str();
        QAction *lut = new QAction(name);
        lut->setCheckable(true);
        if (presets->GetPresetName(i) == "Grayscale")
            lut->setChecked(true);

        lutGroup->addAction(lut);

        connect(lut, &QAction::triggered, this, [=]() { changeColorMap(name); });
    }

    ui->menuColor_Map->addActions(lutGroup->actions());
    changeColorMap("Grayscale", drepSlice->getProxy());

    ui->lowerBoundText->setText(QString::number(lowerBound, 'f', 4));
    ui->upperBoundText->setText(QString::number(upperBound, 'f', 4));
    ui->minCubeText->setText(QString::number(dataMin, 'f', 4));
    ui->maxCubeText->setText(QString::number(dataMax, 'f', 4));
    ui->rmsCubeText->setText(QString::number(rms, 'f', 4));

    sliceFilter = pqApplicationCore::instance()->getObjectBuilder()->createFilter(
            "filters", "ExtractGrid", this->FitsSource);
    contourFilter2D = pqApplicationCore::instance()->getObjectBuilder()->createFilter(
            "filters", "Contour", sliceFilter);

    ui->sliceSlider->setRange(1, bounds[5] + 1);
    ui->sliceSpinBox->setRange(1, bounds[5] + 1);

    setSliceDatacube(1);

    auto interactorStyle = vtkSmartPointer<vtkInteractorStyleImageCustom>::New();
    interactorStyle->SetCoordsCallback([this](std::string str) { showStatusBarMessage(str); });
    interactorStyle->SetLayerFitsReaderFunc(fh.toStdString());
    interactorStyle->SetPixelZCompFunc([this]() -> int { return currentSlice; });

    viewSlice->getViewProxy()->GetRenderWindow()->GetInteractor()->SetInteractorStyle(
            interactorStyle);

    viewCube->resetDisplay();
    viewCube->render();
    viewSlice->resetDisplay();
    viewSlice->render();

    showMaximized();
    activateWindow();
}

vtkWindowCube::~vtkWindowCube()
{
    this->FitsSource = NULL;

    if (parentWindow) {
        parentWindow->removeActorFromRenderer(contoursActorForParent);
    }

    delete ui;
}

QString vtkWindowCube::createFitsHeader(const QMap<QString, QString> &headerMap)
{
    fitsfile *fptr;
    // char* filename=;
    int status = 0;

    QString headerFile = QDir::homePath()
                                 .append(QDir::separator())
                                 .append("VisIVODesktopTemp")
                                 .append(QDir::separator())
                                 .append(this->filename.c_str());
    qDebug() << headerFile;

    fits_create_file(&fptr, ("!" + headerFile).toStdString().c_str(), &status);
    // write a keyword
    int datatype;

    fits_update_key_log(fptr, "SIMPLE", TRUE, "", &status);

    foreach (const auto &key, headerMap.keys()) {
        const auto &value = headerMap[key];
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

double vtkWindowCube::readRMSFromHeader(const QMap<QString, QString> &headerMap)
{
    if (headerMap.contains("RMS")) {
        return headerMap.value("RMS").toDouble();
    }

    int n = headerMap.value("MSn").toInt();
    if (n <= 0) {
        qDebug() << Q_FUNC_INFO
                 << "Missing required header keywords to calculate RMS from partial values";
        return 0.0;
    }

    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        QString keyword("MS");
        keyword.append(QString::number(i));
        double val = headerMap.value(keyword).toDouble();
        sum += val;
    }

    return sqrt(sum / n);
}

void vtkWindowCube::showStatusBarMessage(const std::string &msg)
{
    ui->statusBar->showMessage(QString::fromStdString(msg));
}

void vtkWindowCube::updateVelocityText()
{
    double initSlice = headerMap.value("CRVAL3").toDouble()
            - (headerMap.value("CDELT3").toDouble() * (headerMap.value("CRPIX3").toDouble() - 1));
    double velocity = initSlice + headerMap.value("CDELT3").toDouble() * currentSlice;

    if (headerMap.value("CUNIT3").startsWith("m")) {
        // Return value in km/s
        velocity /= 1000;
    }
    ui->velocityText->setText(QString::number(velocity).append(" Km/s"));
}

void vtkWindowCube::setThreshold(double threshold)
{
    vtkSMProxy *filterProxy = contourFilter->getProxy();
    pqSMAdaptor::setElementProperty(filterProxy->GetProperty("ContourValues"), threshold);
    filterProxy->UpdateVTKObjects();
    contourFilter->updatePipeline();
    viewCube->render();
}

void vtkWindowCube::showContours()
{

    double min = ui->lowerBoundText->text().toDouble();
    double max = ui->upperBoundText->text().toDouble();

    removeContours();
    contourFilter2D = pqApplicationCore::instance()->getObjectBuilder()->createFilter(
            "filters", "Contour", sliceFilter);

    double val;
    vtkSMPropertyHelper(contourFilter2D->getProxy(), "ContourValues").Set(0);

    contourFilter2D->getProxy()->UpdateVTKObjects();
    contourFilter2D->updatePipeline();

    if (ui->levelText->text().toInt() == 1) {
        vtkSMPropertyHelper(contourFilter2D->getProxy(), "ContourValues").Set(min);
    } else {
        for (int i = 0; i < ui->levelText->text().toInt(); ++i) {
            val = min + i * (max - min) / (ui->levelText->text().toInt() - 1);
            vtkSMPropertyHelper(contourFilter2D->getProxy(), "ContourValues").Set(i, val);
        }
    }

    contourFilter2D->getProxy()->UpdateVTKObjects();
    contourFilter2D->updatePipeline();

    auto reprContourSurface =
            pqApplicationCore::instance()->getObjectBuilder()->createDataRepresentation(
                    contourFilter2D->getOutputPort(0), viewSlice);
    auto reprProxyContourSurface = reprContourSurface->getProxy();

    vtkSMPropertyHelper(reprProxyContourSurface, "Representation").Set("Surface");
    vtkSMPVRepresentationProxy *proxy =
            vtkSMPVRepresentationProxy::SafeDownCast(reprProxyContourSurface);
    vtkSMProperty *separateProperty = proxy->GetProperty("UseSeparateColorMap");
    vtkSMPropertyHelper(separateProperty).Set(1);
    vtkSMPVRepresentationProxy::SetScalarColoring(reprProxyContourSurface, "FITSImage",
                                                  vtkDataObject::POINT);

    reprProxyContourSurface->UpdateVTKObjects();
    viewSlice->render();

    /*
     if (parentWindow) {
     double sky_coord_gal[2];
     AstroUtils::xy2sky(fitsReader->GetFileName(), 0, 0, sky_coord_gal, WCS_GALACTIC);

     double coord[3];
     AstroUtils::sky2xy(parentWindow->getFitsReader()->GetFileName(), sky_coord_gal[0],
     sky_coord_gal[1], coord);

     double angle = 0;
     double x1 = coord[0];
     double y1 = coord[1];

     AstroUtils::xy2sky(fitsReader->GetFileName(), 0, 100, sky_coord_gal, WCS_GALACTIC);
     AstroUtils::sky2xy(parentWindow->getFitsReader()->GetFileName(), sky_coord_gal[0],
     sky_coord_gal[1], coord);

     if (x1 != coord[0]) {
     double m = fabs((coord[1] - y1) / (coord[0] - x1));
     angle = 90 - atan(m) * 180 / M_PI;
     }

     double bounds[6];
     fitsReader->GetOutput()->GetBounds(bounds);

     double scaledPixel = AstroUtils::arcsecPixel(fitsReader->GetFileName())
     / AstroUtils::arcsecPixel(parentWindow->getFitsReader()->GetFileName());

     auto transform = vtkSmartPointer<vtkTransform>::New();
     transform->Translate(0, 0, -1 * sliceViewer->GetSlice());
     transform->Translate(bounds[0], bounds[2], 0);
     transform->RotateWXYZ(angle, 0, 0, 1);
     transform->Translate(-bounds[0], -bounds[2], 0);

     auto mapperForParent = vtkSmartPointer<vtkPolyDataMapper>::New();
     mapperForParent->ShallowCopy(mapper);

     contoursActorForParent->ShallowCopy(contoursActor);
     contoursActorForParent->SetMapper(mapperForParent);
     contoursActorForParent->SetScale(scaledPixel, scaledPixel, 1);
     contoursActorForParent->SetPosition(x1, y1, 1);
     contoursActorForParent->SetUserTransform(transform);
     parentWindow->addActorToRenderer(contoursActorForParent);
     }

     */
}

void vtkWindowCube::removeContours()
{
    /*
     ui->qVtkSlice->renderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(contoursActor);
     ui->qVtkSlice->renderWindow()->GetInteractor()->Render();

     if (parentWindow) {
     parentWindow->removeActorFromRenderer(contoursActorForParent);
     }
     */
    if (contourFilter2D) {
        pqApplicationCore::instance()->getObjectBuilder()->destroy(contourFilter2D);
        contourFilter2D = nullptr;
        viewSlice->render();
    }
}

void vtkWindowCube::calculateAndShowMomentMap(int order)
{
    if (parentWindow) {
        auto moment = vtkSmartPointer<vtkFitsReader>::New();
        moment->SetFileName(fitsReader->GetFileName());
        moment->isMoment3D = true;
        moment->setMomentOrder(order);
        parentWindow->addLayerImage(moment);
        parentWindow->raise();
    }
}

void vtkWindowCube::resetCamera()
{
    auto renderer = ui->qVtkCube->renderWindow()->GetRenderers()->GetFirstRenderer();
    auto camera = renderer->GetActiveCamera();
    camera->SetViewUp(0, 1, 0);
    camera->SetPosition(initialCameraPosition);
    camera->SetFocalPoint(initialCameraFocalPoint);
    renderer->ResetCamera();
}

void vtkWindowCube::setCameraAzimuth(double az)
{
    resetCamera();
    auto renderer = ui->qVtkCube->renderWindow()->GetRenderers()->GetFirstRenderer();
    renderer->GetActiveCamera()->Azimuth(az);
    ui->qVtkCube->renderWindow()->GetInteractor()->Render();
}

void vtkWindowCube::setCameraElevation(double el)
{
    resetCamera();
    auto renderer = ui->qVtkCube->renderWindow()->GetRenderers()->GetFirstRenderer();
    renderer->GetActiveCamera()->Elevation(el);
    ui->qVtkCube->renderWindow()->GetInteractor()->Render();
}

void vtkWindowCube::on_sliceSlider_valueChanged(int value)
{
    /*
     if (ui->contourCheckBox->isChecked()) {
     removeContours();
     }
     */
    qDebug() << "on_sliceSlider_valueChanged " << value;
    currentSlice = value - 1;
    setSliceDatacube(value);
    updateVelocityText();
    if (ui->sliceSpinBox->value() != value)
        ui->sliceSpinBox->setValue(value);
}

void vtkWindowCube::on_sliceSpinBox_valueChanged(int value)
{
    qDebug() << "on_sliceSpinBox_valueChanged " << value << " slid: " << ui->sliceSlider->value();
    // setSliceDatacube(value);
    if (ui->sliceSlider->value() != value) {
        ui->sliceSlider->setValue(value);
        ui->sliceSlider->repaint();
    }
}

void vtkWindowCube::setSliceDatacube(int value)
{
    qDebug() << "setslice " << value;

    /*
     XY_PLANE = VTK_XY_PLANE,
     YZ_PLANE = VTK_YZ_PLANE,
     XZ_PLANE = VTK_XZ_PLANE
     */

    currentSlice = value - 1;

    vtkSMProxy *reprProxySlice = drepSlice->getProxy();
    vtkSMPropertyHelper(reprProxySlice, "Slice").Set(currentSlice);
    vtkSMPropertyHelper(reprProxySlice, "SliceMode").Set(VTK_XY_PLANE);
    reprProxySlice->UpdateVTKObjects();

    vtkSMProxy *reprProxySliceCube = drepSliceCube->getProxy();
    vtkSMPropertyHelper(reprProxySliceCube, "Slice").Set(currentSlice);
    vtkSMPropertyHelper(reprProxySliceCube, "SliceMode").Set(VTK_XY_PLANE);
    reprProxySliceCube->UpdateVTKObjects();

    if (sliceFilter) {
        auto sliceProxy = sliceFilter->getProxy();
        int selectedSlice[] = { (int)bounds[0], (int)bounds[1], (int)bounds[2],
                                (int)bounds[3], (currentSlice), (currentSlice) };

        vtkSMPropertyHelper(sliceProxy, "VOI").Set(selectedSlice, 6);
        sliceProxy->UpdateVTKObjects();
        sliceFilter->updatePipeline();
        auto dataInformation = sliceFilter->getOutputPort(0)->getDataInformation();
        auto fitsImageInfo = dataInformation->GetPointDataInformation()->GetArrayInformation(0);
        if (fitsImageInfo) {
            double dataRange[2];
            fitsImageInfo->GetComponentRange(0, dataRange);
            ui->minSliceText->setText(QString::number(dataRange[0]));
            ui->maxSliceText->setText(QString::number(dataRange[1]));

            lutProxy->RescaleTransferFunction(lutProxy, dataRange[0], dataRange[1], true);
        }
    }

    // pxm->UnRegisterProxy(slice);
    // controller->UnRegisterProxy(slice);
    viewSlice->render();
    viewCube->render();
    viewSlice->resetDisplay();
    updateVelocityText();

    if (ui->contourCheckBox->isChecked()) {
        showContours();
    }
}

void vtkWindowCube::on_actionFront_triggered()
{
    setCameraAzimuth(0);
}

void vtkWindowCube::on_actionBack_triggered()
{
    setCameraAzimuth(-180);
}

void vtkWindowCube::on_actionTop_triggered()
{
    setCameraElevation(90);
}

void vtkWindowCube::on_actionRight_triggered()
{
    setCameraAzimuth(90);
}

void vtkWindowCube::on_actionBottom_triggered()
{
    setCameraElevation(-90);
}

void vtkWindowCube::on_actionLeft_triggered()
{
    setCameraAzimuth(-90);
}

void vtkWindowCube::on_thresholdText_editingFinished()
{

    double threshold = ui->thresholdText->text().toDouble();
    // Clamp threshold
    threshold = fmin(fmax(threshold, lowerBound), upperBound);
    ui->thresholdText->setText(QString::number(threshold, 'f', 4));

    int tickPosition = 100 * (threshold - lowerBound) / (upperBound - lowerBound);
    ui->thresholdSlider->setValue(tickPosition);

    setThreshold(threshold);
}

void vtkWindowCube::on_thresholdSlider_sliderReleased()
{

    double threshold =
            (ui->thresholdSlider->value() * (upperBound - lowerBound) / 100) + lowerBound;
    ui->thresholdText->setText(QString::number(threshold, 'f', 4));
    setThreshold(threshold);
}

void vtkWindowCube::on_contourCheckBox_toggled(bool checked)
{
    if (checked) {
        showContours();
    } else {
        removeContours();
    }
}

void vtkWindowCube::on_levelText_editingFinished()
{
    if (ui->contourCheckBox->isChecked()) {
        showContours();
    }
}

void vtkWindowCube::on_lowerBoundText_editingFinished()
{
    if (ui->contourCheckBox->isChecked()) {
        showContours();
    }
}

void vtkWindowCube::on_upperBoundText_editingFinished()
{
    if (ui->contourCheckBox->isChecked()) {
        showContours();
    }
}

void vtkWindowCube::on_actionCalculate_order_0_triggered()
{
    calculateAndShowMomentMap(0);
}

void vtkWindowCube::on_actionCalculate_order_1_triggered()
{
    calculateAndShowMomentMap(1);
}

void vtkWindowCube::on_action0_triggered()
{
    ui->action25->setChecked(false);
    ui->action50->setChecked(false);
    ui->action100->setChecked(false);

    setVolumeRenderingOpacity(0);
}

void vtkWindowCube::on_action25_triggered()
{
    ui->action0->setChecked(false);
    ui->action50->setChecked(false);
    ui->action100->setChecked(false);

    setVolumeRenderingOpacity(0.25);
}

void vtkWindowCube::on_action50_triggered()
{
    ui->action0->setChecked(false);
    ui->action25->setChecked(false);
    ui->action100->setChecked(false);

    setVolumeRenderingOpacity(0.5);
}

void vtkWindowCube::on_action100_triggered()
{
    ui->action0->setChecked(false);
    ui->action25->setChecked(false);
    ui->action50->setChecked(false);

    setVolumeRenderingOpacity(1);
}

void vtkWindowCube::setVolumeRenderingOpacity(double opacity)
{
    vtkSMPropertyHelper(reprProxySurface, "Opacity").Set(opacity);
    reprProxySurface->UpdateVTKObjects();
    viewCube->render();
}

void vtkWindowCube::changeColorMap(QString name)
{
    changeColorMap(name, drepSlice->getProxy());
}

void vtkWindowCube::changeColorMap(QString name, vtkSMProxy *proxy)
{
    if (vtkSMProperty *lutProperty = proxy->GetProperty("LookupTable")) {
        int rescaleMode =
                vtkSMPropertyHelper(lutProxy, "AutomaticRescaleRangeMode", true).GetAsInt();

        auto presets = vtkSMTransferFunctionPresets::GetInstance();

        lutProxy->ApplyPreset(presets->GetFirstPresetWithName(name.toLocal8Bit().data()),
                              (bool)rescaleMode);
        double range[2] = { 0 };
        lutProxy->GetRange(range);

        vtkSMPropertyHelper(lutProperty).Set(lutProxy);
        bool extend = rescaleMode == vtkSMTransferFunctionManager::GROW_ON_APPLY;
        bool force = false;

        vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRange(proxy, extend, force);

        proxy->UpdateVTKObjects();
        viewCube->render();
        viewSlice->render();
        viewSlice->resetDisplay();
    }
}
