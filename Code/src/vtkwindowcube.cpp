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

#include "vtkPVDataInformation.h"
#include "vtkPVArrayInformation.h"
#include "vtkPVDataSetAttributesInformation.h"
#include "vtkPVRenderView.h"

#include "vtkSMTransferFunctionProxy.h"
#include "vtkSMTransferFunctionManager.h"
#include "vtkSMTransferFunctionPresets.h"
#include "vtkPVDataMover.h"

#include "vtkSMTooltipSelectionPipeline.h"

#include "vtkImageSliceRepresentation.h"

#include "mainwindow.h"
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
vtkWindowCube::vtkWindowCube(QPointer<pqPipelineSource> fitsSource) : ui(new Ui::vtkWindowCube)
{
    MainWindow *w = &Singleton<MainWindow>::Instance();
    
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("paraview");
    
    // temp hack, if those two widget are in place
    ui->qVtkCube->hide();
    ui->qVtkSlice->hide();
    
    ui->action100->setChecked(true);
    this->FitsSource=fitsSource;
    
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
    vtkSMSessionProxyManager *pxm = w->server->proxyManager();
    
    
    pqActiveObjects::instance().setActiveSource(this->FitsSource);
    pqObjectBuilder *builder = pqApplicationCore::instance()->getObjectBuilder();
    
    // Outline
    auto drepOutline = builder->createDataRepresentation(this->FitsSource->getOutputPort(0), viewCube);
    auto reprProxyOutline = drepOutline->getProxy();
    vtkSMPropertyHelper(reprProxyOutline, "Representation").Set("Outline");
    vtkSMPropertyHelper(reprProxyOutline, "LineWidth").Set(1);
    double red[3] = { 1.0, 0.0, 0.0 };
    vtkSMPropertyHelper(reprProxyOutline, "AmbientColor").Set(red, 3);
    reprProxyOutline->UpdateVTKObjects();
    
    auto fitsInfo = this->FitsSource->getOutputPort(0)->getDataInformation();
    
    
    auto fitsImageInfo = fitsInfo->GetPointDataInformation()->GetArrayInformation("FITSImage");
    double dataRange[2];
    double bounds[6] = { 0 };
    fitsInfo->GetBounds(bounds);
    fitsImageInfo->GetComponentRange(0, dataRange);
    dataMin = dataRange[0];
    dataMax = dataRange[1];
    
    //get header from server to client, adapting vtkSMTooltipSelectionPipeline::ConnectPVMoveSelectionToClient(
    auto dataMover = vtk::TakeSmartPointer(pxm->NewProxy("misc", "DataMover"));
    vtkSMPropertyHelper(dataMover, "Producer").Set(this->FitsSource->getProxy());
    vtkSMPropertyHelper(dataMover, "PortNumber").Set(static_cast<int>(1));
    vtkSMPropertyHelper(dataMover, "SkipEmptyDataSets").Set(1);
    dataMover->UpdateVTKObjects();
    dataMover->InvokeCommand("Execute");
    
    auto dataMover2 = vtkPVDataMover::SafeDownCast(dataMover->GetClientSideObject());
    vtkTable *headerTable= vtkTable::SafeDownCast(dataMover2->GetDataSetAtIndex(0));
    
    for (vtkIdType i = 0; i < headerTable->GetNumberOfRows(); i++)
    {
        headerMap.insert(QString::fromStdString(headerTable->GetValue(i, 0).ToString()), QString::fromStdString(headerTable->GetValue(i, 1).ToString()));
    }
    
    
    auto calc = builder->createFilter("filters", "PythonCalculator", this->FitsSource);
    if (calc) {
        auto calcProxy = calc->getProxy();
        vtkSMPropertyHelper(calcProxy, "Expression").Set("sqrt(mean(FITSImage**2))");
        vtkSMPropertyHelper(calcProxy, "ArrayName").Set("RMS");
        vtkSMPropertyHelper(calcProxy, "CopyArrays").Set(0);
        calcProxy->UpdateVTKObjects();
        calc->updatePipeline();
        auto dataInformation = calc->getOutputPort(0)->getDataInformation();
        auto arrayInformation =
        dataInformation->GetPointDataInformation()->GetArrayInformation("RMS");
        double range[2];
        arrayInformation->GetComponentRange(0, range);
        rms = range[0];
    }
    
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
    vtkSMPVRepresentationProxy::SetScalarColoring(reprProxySlice, "FITSImage", vtkDataObject::POINT);
    
    // Slice
    drepSliceCube = builder->createDataRepresentation(this->FitsSource->getOutputPort(0), viewCube);
    auto reprProxySliceCube = drepSliceCube->getProxy();
    vtkSMPropertyHelper(reprProxySliceCube, "Representation").Set("Slice");
    vtkSMPVRepresentationProxy::SetScalarColoring(reprProxySliceCube, "FITSImage", vtkDataObject::POINT);
    vtkSMPropertyHelper helper(reprProxySliceCube, "ColorArrayName");
    const char* arrayName = helper.GetInputArrayNameToProcess();
    vtkNew<vtkSMTransferFunctionManager> mgr;
    
    lutProxy = vtkSMTransferFunctionProxy::SafeDownCast(
                                                        mgr->GetColorTransferFunction(arrayName, reprProxySliceCube->GetSessionProxyManager()));
    
    auto presets = vtkSMTransferFunctionPresets::GetInstance();
    QActionGroup *lutGroup = new QActionGroup(this);
    for (int i=0; i< presets->GetNumberOfPresets();i++)
    {
        QString name= presets->GetPresetName(i).c_str();
        QAction *lut = new QAction(name);
        lut->setCheckable (true);
        if (presets->GetPresetName(i) == "X Ray")
            lut->setChecked(true);
        
        lutGroup->addAction(lut);
        
        connect(lut, &QAction::triggered, this,[=](){
            changeSliceColorMap(name);
        });
    }
    
    ui->menuColor_Map->addActions(lutGroup->actions());
    changeSliceColorMap("X Ray");
    
    ui->lowerBoundText->setText(QString::number(lowerBound, 'f', 4));
    ui->upperBoundText->setText(QString::number(upperBound, 'f', 4));
    ui->minCubeText->setText(QString::number(dataMin, 'f', 4));
    ui->maxCubeText->setText(QString::number(dataMax, 'f', 4));
    ui->rmsCubeText->setText(QString::number(rms, 'f', 4));
    
    on_sliceSpinBox_valueChanged(1);
    ui->sliceSlider->setRange(1, bounds[5] + 1);
    ui->sliceSpinBox->setRange(1, bounds[5] + 1);
    
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

void vtkWindowCube::showStatusBarMessage(const std::string &msg)
{
    ui->statusBar->showMessage(QString::fromStdString(msg));
}

void vtkWindowCube::updateVelocityText()
{
    double initSlice = headerMap.value("CRVAL3").toDouble() - (headerMap.value("CDELT3").toDouble() * (headerMap.value("CRPIX3").toDouble() - 1));
    double velocity = initSlice + headerMap.value("CDELT3").toDouble() * (currentSlice-1);
    
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
    auto plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(0, 0, sliceViewer->GetSlice());
    plane->SetNormal(0, 0, 1);
    
    auto cutter = vtkSmartPointer<vtkCutter>::New();
    cutter->SetCutFunction(plane);
    cutter->SetInputData(fitsReader->GetOutput());
    cutter->Update();
    
    int level = ui->levelText->text().toInt();
    double min = ui->lowerBoundText->text().toDouble();
    double max = ui->upperBoundText->text().toDouble();
    
    auto contoursFilter = vtkSmartPointer<vtkContourFilter>::New();
    contoursFilter->GenerateValues(level, min, max);
    contoursFilter->SetInputConnection(cutter->GetOutputPort());
    
    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(contoursFilter->GetOutputPort());
    mapper->SetScalarRange(min, max);
    mapper->ScalarVisibilityOn();
    mapper->SetScalarModeToUsePointData();
    mapper->SetColorModeToMapScalars();
    
    contoursActor->SetMapper(mapper);
    ui->qVtkSlice->renderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(contoursActor);
    ui->qVtkSlice->renderWindow()->GetInteractor()->Render();
    
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
}

void vtkWindowCube::removeContours()
{
    ui->qVtkSlice->renderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(contoursActor);
    ui->qVtkSlice->renderWindow()->GetInteractor()->Render();
    
    if (parentWindow) {
        parentWindow->removeActorFromRenderer(contoursActorForParent);
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
    currentSlice = value - 1;
    ui->sliceSpinBox->setValue(currentSlice);
    updateVelocityText();
    
}

void vtkWindowCube::on_sliceSlider_sliderReleased()
{
    ui->sliceSpinBox->setValue(ui->sliceSlider->value());
}

void vtkWindowCube::on_sliceSpinBox_valueChanged(int value)
{
    /*
         XY_PLANE = VTK_XY_PLANE,
         YZ_PLANE = VTK_YZ_PLANE,
         XZ_PLANE = VTK_XZ_PLANE
     */
    
    currentSlice = value;
    
    vtkSMProxy *reprProxySlice = drepSlice->getProxy();
    vtkSMPropertyHelper(reprProxySlice, "Slice").Set(currentSlice-1);
    vtkSMPropertyHelper(reprProxySlice, "SliceMode").Set(VTK_XY_PLANE);
    reprProxySlice->UpdateVTKObjects();
    
    vtkSMProxy *reprProxySliceCube = drepSliceCube->getProxy();
    vtkSMPropertyHelper(reprProxySliceCube, "Slice").Set(currentSlice-1);
    vtkSMPropertyHelper(reprProxySliceCube, "SliceMode").Set(VTK_XY_PLANE);
    reprProxySliceCube->UpdateVTKObjects();
    
    auto slice = pqApplicationCore::instance()->getObjectBuilder()->createFilter("filters", "Cut", this->FitsSource);
    if (slice) {
        auto sliceProxy = slice->getProxy();
        vtkSMProxy* cutFunction = vtkSMPropertyHelper(sliceProxy, "CutFunction").GetAsProxy();
        double normal[] = { 0, 0, 1 };
        vtkSMPropertyHelper(cutFunction, "Normal").Set(normal, 3);
        double origin[] = { 0, 0, (double)currentSlice-1 };
        vtkSMPropertyHelper(cutFunction, "Origin").Set(origin, 3);
        cutFunction->UpdateVTKObjects();
        sliceProxy->UpdateVTKObjects();
        slice->updatePipeline();

        auto dataInformation = slice->getOutputPort(0)->getDataInformation();
        auto fitsImageInfo = dataInformation->GetPointDataInformation()->GetArrayInformation(0);
        if (fitsImageInfo)
        {
            double dataRange[2];
        fitsImageInfo->GetComponentRange(0, dataRange);
        ui->minSliceText->setText(QString::number(dataRange[0]));
        ui->maxSliceText->setText(QString::number(dataRange[1]));
   
        }
    }
    //controller->UnRegisterProxy(slice);
    viewSlice->render();
    viewCube->render();
    viewSlice->resetDisplay();
    updateVelocityText();
        
    /*
     ui->sliceSlider->setValue(value);
     if (ui->contourCheckBox->isChecked()) {
     showContours();
     }
     */
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

void vtkWindowCube::changeSliceColorMap(QString name)
{
    //here need to uncheck all the other item in menu
    
    
    auto reprProxySliceCube = drepSliceCube->getProxy();
    
    if (vtkSMProperty* lutProperty = reprProxySliceCube->GetProperty("LookupTable"))
    {
        int rescaleMode =
        vtkSMPropertyHelper(lutProxy, "AutomaticRescaleRangeMode", true).GetAsInt();
        
        auto presets = vtkSMTransferFunctionPresets::GetInstance();
        
        lutProxy->ApplyPreset(presets->GetFirstPresetWithName(name.toLocal8Bit().data()), (bool)rescaleMode);
        
        vtkSMPropertyHelper(lutProperty).Set(lutProxy);
        bool extend = rescaleMode == vtkSMTransferFunctionManager::GROW_ON_APPLY;
        bool force = false;
        vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRange(reprProxySliceCube, extend, force);
        reprProxySliceCube->UpdateVTKObjects();
        viewCube->render();
        viewSlice->render();
        viewSlice->resetDisplay();
        
        
    }
    
}
