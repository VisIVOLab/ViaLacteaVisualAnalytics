#include <pybind11/embed.h>

#include "ui_lutcustomize.h"
#include "ui_vtkwindowcube.h"
#include "vtkwindowcube.h"

#include "interactors/vtkinteractorstyledrawarrow.h"
#include "interactors/vtkinteractorstyleimagecustom.h"
#include "interactors/vtkinteractorstyleprofile.h"

#include "astroutils.h"
#include "FilterFITSDialog.h"
#include "lutcustomize.h"
#include "luteditor.h"
#include "profilewindow.h"
#include "vtkfitsreader.h"
#include "vtkfitsreader2.h"
#include "vtkFITSWriter.h"
#include "vtklegendscaleactorwcs.h"
#include "vtkwindow_new.h"
#include "vtkwindowpv.h"

#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCaptionActor2D.h>
#include <vtkCutter.h>
#include <vtkFlyingEdges2D.h>
#include <vtkFlyingEdges3D.h>
#include <vtkFrustumSource.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageActor.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkImageSliceMapper.h>
#include <vtkLODActor.h>
#include <vtkLookupTable.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkOutlineFilter.h>
#include <vtkPlane.h>
#include <vtkPlanes.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkResliceImageViewer.h>
#include <vtkScalarBarActor.h>
#include <vtkTextActor.h>
#include <vtkTransform.h>

#include <QDebug>

#include <cmath>
#include <string>

#include "visivomenu.h"

namespace py = pybind11;

vtkWindowCube::vtkWindowCube(QWidget *parent, const QString &filepath, int ScaleFactor, QString velocityUnit)
: QMainWindow(parent),
ui(new Ui::vtkWindowCube),
filepath(filepath),
ScaleFactor(ScaleFactor),
parentWindow(qobject_cast<vtkwindow_new *>(parent)),
currentSlice(0),
velocityUnit(velocityUnit){
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(filepath);
    
    
    readFitsHeader();
    
    auto wcsGroup = new QActionGroup(this);
    auto wcsItem = new QAction("Galactic", wcsGroup);
    wcsItem->setCheckable(true);
    wcsItem->setChecked(true);
    wcsGroup->addAction(wcsItem);
    connect(wcsItem, &QAction::triggered, this, [=]() { changeLegendWCS(WCS_GALACTIC); });
    
    wcsItem = new QAction("FK5", wcsGroup);
    wcsItem->setCheckable(true);
    wcsGroup->addAction(wcsItem);
    connect(wcsItem, &QAction::triggered, this, [=]() { changeLegendWCS(WCS_J2000); });
    
    wcsItem = new QAction("FK4", wcsGroup);
    wcsItem->setCheckable(true);
    wcsGroup->addAction(wcsItem);
    connect(wcsItem, &QAction::triggered, this, [=]() { changeLegendWCS(WCS_B1950); });
    
    wcsItem = new QAction("Ecliptic", wcsGroup);
    wcsItem->setCheckable(true);
    wcsGroup->addAction(wcsItem);
    connect(wcsItem, &QAction::triggered, this, [=]() { changeLegendWCS(WCS_ECLIPTIC); });
    
    // ui->menuWCS->addActions(wcsGroup->actions());
    
    readerCube = vtkSmartPointer<vtkFitsReader2>::New();
    readerCube->SetFileName(filepath.toStdString().c_str());
    readerCube->SetScaleFactor(ScaleFactor);
    readerCube->Update();
    
    float rms = readerCube->GetRMS();
    double bounds[6];
    readerCube->GetBounds(bounds);
    float rangeCube[2];
    readerCube->GetValueRange(rangeCube);
    lowerBound = 3 * rms;
    upperBound = rangeCube[1];
    
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
    
    // Outline
    auto outlineFilter = vtkSmartPointer<vtkOutlineFilter>::New();
    outlineFilter->SetInputConnection(readerCube->GetOutputPort());
    auto outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    outlineMapper->SetInputConnection(outlineFilter->GetOutputPort());
    outlineMapper->ScalarVisibilityOff();
    auto outlineActor = vtkSmartPointer<vtkActor>::New();
    outlineActor->SetMapper(outlineMapper);
    rendererCube->AddActor(outlineActor);
    
    // Isosurface
    isosurface = vtkSmartPointer<vtkFlyingEdges3D>::New();
    isosurface->SetInputConnection(readerCube->GetOutputPort());
    isosurface->ComputeNormalsOn();
    isosurface->SetValue(0, lowerBound);
    auto isosurfaceMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    isosurfaceMapper->SetInputConnection(isosurface->GetOutputPort());
    isosurfaceMapper->ScalarVisibilityOff();
    auto isosurfaceActor = vtkSmartPointer<vtkActor>::New();
    isosurfaceActor->SetMapper(isosurfaceMapper);
    isosurfaceActor->GetProperty()->SetColor(1.0, 0.5, 1.0);
    rendererCube->AddActor(isosurfaceActor);
    
    // Plane
    auto planes = vtkSmartPointer<vtkPlanes>::New();
    planes->SetBounds(bounds[0], bounds[1], bounds[2], bounds[3], 0, 1);
    auto frustumSource = vtkSmartPointer<vtkFrustumSource>::New();
    frustumSource->ShowLinesOff();
    frustumSource->SetPlanes(planes);
    auto planeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    planeMapper->SetInputConnection(frustumSource->GetOutputPort());
    planeActor = vtkSmartPointer<vtkActor>::New();
    planeActor->SetMapper(planeMapper);
    rendererCube->AddActor(planeActor);
    
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
    legendActorCube = vtkSmartPointer<vtkLegendScaleActorWCS>::New();
    legendActorCube->LegendVisibilityOff();
    legendActorCube->setFitsFile(readerCube->GetFileName());
    rendererCube->AddActor(legendActorCube);
    
    rendererCube->GetActiveCamera()->GetPosition(initialCameraPosition);
    rendererCube->GetActiveCamera()->GetFocalPoint(initialCameraFocalPoint);
    
    rendererCube->ResetCamera();
    renWinCube->GetInteractor()->Render();
    //  End datacube pipeline
    
    // Start slice pipeline
    readerSlice = vtkSmartPointer<vtkFitsReader2>::New();
    readerSlice->SetFileName(filepath.toStdString().c_str());
    readerSlice->SliceModeOn();
    readerSlice->SetSlice(0);
    readerSlice->Update();
    
    float rangeSlice[2];
    readerSlice->GetValueRange(rangeSlice);
    
    auto renWinSlice = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->qVtkSlice->setDefaultCursor(Qt::ArrowCursor);
    ui->qVtkSlice->setRenderWindow(renWinSlice);
    
    // Setup context menu to toggle slice/moment renderers
    QActionGroup *grp = new QActionGroup(this);
    grp->addAction(ui->actionShowSlice);
    grp->addAction(ui->actionShowMomentMap);
    connect(ui->actionShowSlice, &QAction::triggered, this, [=]() { changeSliceView(0); });
    connect(ui->actionShowMomentMap, &QAction::triggered, this, [=]() {
        if (moment.Get() == nullptr) {
            calculateAndShowMomentMap(0);
        } else {
            changeSliceView(1);
        }
    });
    
    rendererSlice = vtkSmartPointer<vtkRenderer>::New();
    rendererSlice->SetBackground(0.21, 0.23, 0.25);
    rendererSlice->GlobalWarningDisplayOff();
    
    rendererMoment = vtkSmartPointer<vtkRenderer>::New();
    rendererMoment->SetBackground(0.21, 0.23, 0.25);
    rendererMoment->GlobalWarningDisplayOff();
    
    legendActorMoment = vtkSmartPointer<vtkLegendScaleActorWCS>::New();
    legendActorMoment->LegendVisibilityOff();
    legendActorMoment->setFitsFile(readerSlice->GetFileName());
    rendererMoment->AddActor(legendActorMoment);
    
    momViewer = vtkSmartPointer<vtkResliceImageViewer>::New();
    momViewer->SetRenderer(rendererMoment);
    momViewer->SetRenderWindow(renWinSlice);
    momViewer->GetImageActor()->InterpolateOff();
    
    // Show cube slices by default
    currentVisOnSlicePanel = 0;
    changeSliceView(currentVisOnSlicePanel);
    
    lutName = "Gray";
    lutSlice = vtkSmartPointer<vtkLookupTable>::New();
    lutSlice->SetTableRange(rangeSlice[0], rangeSlice[1]);
    SelectLookTable(lutName, lutSlice);
    
    sliceViewer = vtkSmartPointer<vtkResliceImageViewer>::New();
    sliceViewer->SetInputData(readerSlice->GetOutput());
    sliceViewer->GetWindowLevel()->SetOutputFormatToRGB();
    sliceViewer->GetWindowLevel()->SetLookupTable(lutSlice);
    sliceViewer->GetImageActor()->InterpolateOff();
    sliceViewer->SetRenderWindow(renWinSlice);
    sliceViewer->SetRenderer(rendererSlice);
    ui->sliceSlider->setRange(1, readerSlice->GetNumberOfSlices());
    ui->sliceSpinBox->setRange(1, readerSlice->GetNumberOfSlices());
    
    // Added to renderers when the contour checkbox is checked
    contoursActor = vtkSmartPointer<vtkLODActor>::New();
    contoursActor->GetProperty()->SetLineWidth(1);
    contoursActorForParent = vtkSmartPointer<vtkLODActor>::New();
    contoursActorForParent->GetProperty()->SetLineWidth(1);
    
    legendActorSlice = vtkSmartPointer<vtkLegendScaleActorWCS>::New();
    legendActorSlice->LegendVisibilityOff();
    legendActorSlice->setFitsFile(readerSlice->GetFileName());
    rendererSlice->AddActor(legendActorSlice);
    
    setInteractorStyleImage();
    
    rendererSlice->ResetCamera();
    renWinSlice->GetInteractor()->Render();
    
    // Show FITS Stats
    ui->lineCubeMin->setText(QString::number(readerCube->GetValueRange()[0], 'f', 4));
    ui->lineCubeMax->setText(QString::number(readerCube->GetValueRange()[1], 'f', 4));
    ui->lineCubeMean->setText(QString::number(readerCube->GetMean(), 'f', 4));
    ui->lineCubeRMS->setText(QString::number(readerCube->GetRMS(), 'f', 4));
    ui->lineImgMin->setText(QString::number(readerSlice->GetValueRange()[0], 'f', 4));
    ui->lineImgMax->setText(QString::number(readerSlice->GetValueRange()[1], 'f', 4));
    ui->lineImgMean->setText(QString::number(readerSlice->GetMean(), 'f', 4));
    ui->lineImgRMS->setText(QString::number(readerSlice->GetRMS(), 'f', 4));
    
    // Set legend WCS
    QString ctype1 = fitsHeader.value("CTYPE1").toUpper();
    if (ctype1.startsWith("GL")) {
        legendActorCube->setWCS(WCS_GALACTIC);
        legendActorSlice->setWCS(WCS_GALACTIC);
        legendActorMoment->setWCS(WCS_GALACTIC);
        // ui->menuWCS->actions().at(1)->setChecked(true);
    } else if (ctype1.startsWith("RA")) {
        // FK5
        legendActorCube->setWCS(WCS_J2000);
        legendActorSlice->setWCS(WCS_J2000);
        legendActorMoment->setWCS(WCS_J2000);
        // ui->menuWCS->actions().at(2)->setChecked(true);
    }
    
    bunit = fitsHeader.value("BUNIT");
    bunit.replace("'", QString());
    bunit.replace("\"", QString());
    bunit = bunit.simplified();
    
    if (!bunit.isEmpty()) {
        ui->thresholdGroupBox->setTitle(ui->thresholdGroupBox->title() + " (" + bunit + ")");
        ui->contourGroupBox->setTitle(ui->contourGroupBox->title() + " (" + bunit + ")");
    }
    
    currentSlice = 0;
    updateSliceDatacube();
    
    visivoMenu = new VisIVOMenu();
    visivoMenu->configureCubeWindowMenu();
    this->layout()->setMenuBar(visivoMenu);
}

vtkWindowCube::~vtkWindowCube()
{
    setInteractorStyleImage();
    
    if (parentWindow) {
        parentWindow->removeActor(contoursActorForParent);
    }
    
    if (profileWin) {
        profileWin->deleteLater();
    }
    
    delete ui;
}

void vtkWindowCube::changeSliceView(int mode)
{
    switch (mode) {
        case 0:
            currentVisOnSlicePanel = 0;
            ui->qVtkSlice->renderWindow()->RemoveRenderer(rendererMoment);
            ui->qVtkSlice->renderWindow()->AddRenderer(rendererSlice);
            
            ui->labelImg->setText("Slice");
            ui->lineImgMin->setText(QString::number(readerSlice->GetValueRange()[0], 'f', 4));
            ui->lineImgMax->setText(QString::number(readerSlice->GetValueRange()[1], 'f', 4));
            ui->lineImgMean->setText(QString::number(readerSlice->GetMean(), 'f', 4));
            ui->lineImgRMS->setText(QString::number(readerSlice->GetRMS(), 'f', 4));
            
            if (lcustom) {
                lcustom->configureFits3D();
            }
            break;
        case 1:
            currentVisOnSlicePanel = 1;
            ui->qVtkSlice->renderWindow()->RemoveRenderer(rendererSlice);
            ui->qVtkSlice->renderWindow()->AddRenderer(rendererMoment);
            ui->actionShowMomentMap->setChecked(true);
            
            ui->labelImg->setText("Moment");
            ui->lineImgMin->setText(QString::number(moment->GetMin(), 'f', 4));
            ui->lineImgMax->setText(QString::number(moment->GetMax(), 'f', 4));
            ui->lineImgMean->setText(QString::number(moment->GetMedia(), 'f', 4));
            ui->lineImgRMS->setText(QString::number(moment->GetRMS(), 'f', 4));
            
            if (lcustom) {
                lcustom->configureMoment();
            }
            break;
    }
    
    ui->qVtkSlice->renderWindow()->GetRenderers()->GetFirstRenderer()->ResetCamera();
    ui->qVtkSlice->renderWindow()->Render();
}

void vtkWindowCube::setInteractorStyleImage()
{
    vtkNew<vtkInteractorStyleImageCustom> interactorStyle;
    interactorStyle->SetCoordsCallback([this](std::string str) { showStatusBarMessage(str); });
    interactorStyle->SetReader(readerSlice);
    ui->qVtkSlice->renderWindow()->GetInteractor()->SetInteractorStyle(interactorStyle);
    ui->qVtkSlice->renderWindow()->Render();
}

void vtkWindowCube::setInteractorStyleProfile(bool liveMode)
{
    vtkNew<vtkInteractorStyleProfile> interactorStyle;
    interactorStyle->SetCoordsCallback([this](std::string str) { showStatusBarMessage(str); });
    interactorStyle->SetProfileCallback(
                                        [this](double x, double y, bool live) { extractSpectrum(x, y, live); });
    interactorStyle->SetReader(readerSlice);
    interactorStyle->SetLiveMode(liveMode);
    ui->qVtkSlice->renderWindow()->GetInteractor()->SetInteractorStyle(interactorStyle);
    ui->qVtkSlice->renderWindow()->Render();
}

void vtkWindowCube::setInteractorStyleDrawLine()
{
    vtkNew<vtkInteractorStyleDrawArrow> interactorStyle;
    interactorStyle->SetAbortCallback([this]() { setInteractorStyleImage(); });
    interactorStyle->SetCallback([this](float x1, float y1, float x2, float y2) {
        setInteractorStyleImage();
        generatePositionVelocityPlot(x1, y1, x2, y2);
    });
    ui->qVtkSlice->renderWindow()->GetInteractor()->SetInteractorStyle(interactorStyle);
    ui->qVtkSlice->renderWindow()->Render();
    showStatusBarMessage("Press ENTER to confirm your selection, press ESC to abort.");
}

int vtkWindowCube::readFitsHeader()
{
    this->fitsHeader.clear();
    
    fitsfile *fptr;
    int status = 0;
    if (fits_open_data(&fptr, this->filepath.toUtf8().data(), READONLY, &status)) {
        fits_report_error(stderr, status);
        return 1;
    }
    
    // Get number of keys in header
    int nKeys = 0;
    if (fits_get_hdrspace(fptr, &nKeys, 0, &status)) {
        fits_report_error(stderr, status);
        return 2;
    }
    
    // Get header keys and values
    char name[80];
    char value[80];
    for (int i = 1; i <= nKeys; ++i) {
        if (fits_read_keyn(fptr, i, name, value, 0, &status)) {
            fits_report_error(stderr, status);
            return 3;
        }
        
        // Skip empty names, HISTORY and COMMENT keyworks
        QString s_name(QString::fromUtf8(name));
        QString s_value(QString::fromUtf8(value));
        if (s_name.isEmpty() || (s_name.compare("HISTORY", Qt::CaseInsensitive) == 0)
            || (s_name.compare("COMMENT", Qt::CaseInsensitive) == 0)) {
            continue;
        }
        
        this->fitsHeader.insert(s_name, s_value);
    }
    
    fits_close_file(fptr, &status);
    return 0;
}

void vtkWindowCube::showStatusBarMessage(const std::string &msg)
{
    ui->statusBar->showMessage(QString::fromStdString(msg));
}

void vtkWindowCube::updateSliceDatacube()
{
    planeActor->SetPosition(0, 0, currentSlice);
    readerSlice->SetSlice(currentSlice);
    readerSlice->Update();
    
    float range[2];
    readerSlice->GetValueRange(range);
    
    auto lutSlice = vtkSmartPointer<vtkLookupTable>::New();
    lutSlice->SetTableRange(range[0], range[1]);
    SelectLookTable(lutName, lutSlice);
    sliceViewer->SetInputData(readerSlice->GetOutput());
    sliceViewer->GetWindowLevel()->SetLookupTable(lutSlice);
    if (lcustom && currentVisOnSlicePanel == 0)
        lcustom->configureFits3D();
    
    sliceViewer->GetRenderer()->ResetCamera();
    sliceViewer->Render();
    ui->qVtkCube->renderWindow()->GetInteractor()->Render();
    
    if (currentVisOnSlicePanel == 0) {
        // Update stats widget only if needed
        ui->lineImgMin->setText(QString::number(readerSlice->GetValueRange()[0], 'f', 4));
        ui->lineImgMax->setText(QString::number(readerSlice->GetValueRange()[1], 'f', 4));
        ui->lineImgMean->setText(QString::number(readerSlice->GetMean(), 'f', 4));
        ui->lineImgRMS->setText(QString::number(readerSlice->GetRMS(), 'f', 4));
    }
    
    if (parentWindow && ui->contourCheckBox->isChecked()) {
        removeContours();
        showContours();
    }
}

void vtkWindowCube::updateVelocityText()
{
    double crval3 = fitsHeader.value("CRVAL3").toDouble();
    double cdelt3 = fitsHeader.value("CDELT3").toDouble();
    double crpix3 = fitsHeader.value("CRPIX3").toDouble();
    
    double initSlice = crval3 - (cdelt3 * (crpix3 - 1));
    double value = initSlice + cdelt3 * currentSlice;
    
    QString cunit3 = fitsHeader.value("CUNIT3");
    cunit3.replace("'", QString());
    cunit3.replace("\"", QString());
    cunit3 = cunit3.simplified();
    
    if (cunit3.startsWith("m")) {
        // Return value in km/s
        value /= 1000;
    } else if (cunit3.startsWith("Hz")) {
        // Convert to velocity
        double restfrq = fitsHeader.value("RESTFRQ").toDouble();
        double c = 299792.458;
        value = (restfrq - value) / restfrq * c;
    }
    ui->velocityText->setText(QString::number(value).append(" Km/s"));
}

void vtkWindowCube::setThreshold(double threshold)
{
    isosurface->SetValue(0, threshold);
    ui->qVtkCube->renderWindow()->GetInteractor()->Render();
}

void vtkWindowCube::showContours()
{
    int level = ui->levelText->text().toInt();
    double min = ui->lowerBoundText->text().toDouble();
    double max = ui->upperBoundText->text().toDouble();
    
    auto contoursFilter = vtkSmartPointer<vtkFlyingEdges2D>::New();
    contoursFilter->GenerateValues(level, min, max);
    contoursFilter->SetInputConnection(readerSlice->GetOutputPort());
    
    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(contoursFilter->GetOutputPort());
    mapper->SetScalarRange(min, max);
    mapper->ScalarVisibilityOn();
    mapper->SetScalarModeToUsePointData();
    mapper->SetColorModeToMapScalars();
    
    contoursActor->SetMapper(mapper);
    rendererSlice->AddActor(contoursActor);
    ui->qVtkSlice->renderWindow()->Render();
    
    if (parentWindow) {
        double sky_coord_gal[2];
        AstroUtils::xy2sky(filepath.toStdString(), 0, 0, sky_coord_gal, WCS_GALACTIC);
        
        double coord[3];
        AstroUtils::sky2xy(parentWindow->getFitsImage()->GetFileName(), sky_coord_gal[0],
                           sky_coord_gal[1], coord);
        
        double angle = 0;
        double x1 = coord[0];
        double y1 = coord[1];
        
        AstroUtils::xy2sky(filepath.toStdString(), 0, 100, sky_coord_gal, WCS_GALACTIC);
        AstroUtils::sky2xy(parentWindow->getFitsImage()->GetFileName(), sky_coord_gal[0],
                           sky_coord_gal[1], coord);
        
        if (x1 != coord[0]) {
            double m = fabs((coord[1] - y1) / (coord[0] - x1));
            angle = 90 - atan(m) * 180 / M_PI;
        }
        
        double bounds[6];
        readerCube->GetBounds(bounds);
        
        double scaledPixel = AstroUtils::arcsecPixel(filepath.toStdString())
        / AstroUtils::arcsecPixel(parentWindow->getFitsImage()->GetFileName());
        
        auto transform = vtkSmartPointer<vtkTransform>::New();
        transform->Translate(0, 0, -1 * sliceViewer->GetSlice());
        transform->Translate(bounds[0], bounds[2], 0);
        transform->RotateWXYZ(angle, 0, 0, 1);
        transform->Translate(-bounds[0], -bounds[2], 0);
        
        auto mapperForParent = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapperForParent->ShallowCopy(mapper);
        
        contoursActorForParent = vtkSmartPointer<vtkActor>::New();
        contoursActorForParent->SetMapper(mapperForParent);
        contoursActorForParent->SetScale(scaledPixel, scaledPixel, 1);
        contoursActorForParent->SetUserTransform(transform);
        contoursActorForParent->SetPosition(x1, y1, 1);
        parentWindow->addActor(contoursActorForParent);
        parentWindow->updateScene();
    }
}

void vtkWindowCube::removeContours()
{
    rendererSlice->RemoveActor(contoursActor);
    ui->qVtkSlice->renderWindow()->Render();
    
    if (parentWindow) {
        parentWindow->removeActor(contoursActorForParent);
    }
}

void vtkWindowCube::calculateAndShowMomentMap(int order)
{
    moment = vtkSmartPointer<vtkFitsReader>::New();
    moment->SetFileName(filepath.toStdString());
    moment->isMoment3D = true;
    moment->setMomentOrder(order);
    
    if (parentWindow && parentWindow->isVisible()) {
        parentWindow->addLayerImage(moment);
    } else {
        parentWindow = new vtkwindow_new(nullptr, moment, 0, 0, false);
        parentWindow->show();
    }
    
    momViewer->SetInputData(moment->GetOutput());
    momViewer->Render();
    changeSliceView(1);
    this->raise();
    this->activateWindow();
    this->saveMomentToFile();
}

void vtkWindowCube::saveMomentToFile()
{
    const QDir outDir = QDir::home().absoluteFilePath("VisIVODesktopTemp");
    const QString fn = QFileInfo(this->filepath).baseName() + "_moment"
    + QString::number(this->moment->getMomentOrder()) + ".fits";
    const QString filepath = outDir.absoluteFilePath(fn);
    if (QFile::exists(filepath)) {
        // Do not overwrite existing file!
        return;
    }
    
    vtkNew<vtkFITSWriter> writer;
    writer->SetInputData(this->moment->GetOutput());
    writer->SetFileName(filepath.toStdString().c_str());
    writer->Write();
    AstroUtils::setMomentFITSHeader(this->filepath.toStdString(), filepath.toStdString(),
                                    this->moment->getMomentOrder());
}

void vtkWindowCube::generatePositionVelocityPlot(float x1, float y1, float x2, float y2)
{
    try {
        std::string fin = this->filepath.toStdString();
        py::list line;
        line.append(std::make_tuple(x1, y1));
        line.append(std::make_tuple(x2, y2));
        std::string outDir = QDir::home().absoluteFilePath("VisIVODesktopTemp").toStdString();
        
        py::module_ pvplot = py::module_::import("pvplot");
        std::string fout = pvplot.attr("extract_pv_plot")(fin, this->currentSlice, line, outDir)
            .cast<std::string>();
        auto win = new vtkWindowPV(QString::fromStdString(fout), this->filepath, x1, y1, x2, y2);
        win->show();
    } catch (const std::exception &e) {
        qDebug() << Q_FUNC_INFO << "Error" << e.what();
        QMessageBox::critical(this, "Error", e.what());
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
    //
    auto interactorStyle = vtkSmartPointer<vtkInteractorStyleImageCustom>::New();
    interactorStyle->SetCoordsCallback([this](std::string str) { showStatusBarMessage(str); });
    interactorStyle->SetReader(readerSlice);
    ui->qVtkSlice->renderWindow()->GetInteractor()->SetInteractorStyle(interactorStyle);
    //
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
    currentSlice = value - 1;
    updateVelocityText();
    updateSliceDatacube();
}

void vtkWindowCube::on_sliceSlider_sliderReleased()
{
    ui->sliceSpinBox->setValue(ui->sliceSlider->value());
}

void vtkWindowCube::on_sliceSpinBox_valueChanged(int value)
{
    ui->sliceSlider->setValue(value);
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

void vtkWindowCube::on_actionCalculate_order_2_triggered()
{
    calculateAndShowMomentMap(2);
}

void vtkWindowCube::on_actionCalculate_order_6_triggered()
{
    calculateAndShowMomentMap(6);
}

void vtkWindowCube::on_actionCalculate_order_8_triggered()
{
    calculateAndShowMomentMap(8);
}

void vtkWindowCube::on_actionCalculate_order_10_triggered()
{
    calculateAndShowMomentMap(10);
}

void vtkWindowCube::changeLegendWCS(int wcs)
{
    legendActorCube->setWCS(wcs);
    legendActorSlice->setWCS(wcs);
    legendActorMoment->setWCS(wcs);
    ui->qVtkCube->renderWindow()->Render();
    ui->qVtkSlice->renderWindow()->Render();
}

void vtkWindowCube::on_actionSlice_Lookup_Table_triggered()
{
    if (!lcustom)
        lcustom = new LutCustomize(this);
    lcustom->setLut(lutName);
    QString selected_scale = "Linear";
    if (lutSlice->GetScale() != 0)
        selected_scale = "Log";
    lcustom->setScaling(selected_scale);
    
    if (currentVisOnSlicePanel == 0) {
        lcustom->configureFits3D();
    } else {
        lcustom->configureMoment();
    }
    
    lcustom->show();
}

void vtkWindowCube::showColorbar(bool checked, double min, double max)
{
    auto renderer = currentVisOnSlicePanel == 0 ? rendererSlice : rendererMoment;
    auto legend = currentVisOnSlicePanel == 0 ? legendActorSlice : legendActorMoment;
    
    if (scalarBar != 0) {
        renderer->RemoveActor(scalarBar);
        scalarBar = nullptr;
    }
    
    if (checked) {
        vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
        SelectLookTable(lcustom->ui->lutComboBox->currentText().toStdString().c_str(), lut);
        lut->SetTableRange(min, max);
        scalarBar = vtkScalarBarActor::New();
        scalarBar->SetLabelFormat("%.3g");
        scalarBar->SetOrientationToVertical();
        scalarBar->UnconstrainedFontSizeOn();
        scalarBar->SetMaximumWidthInPixels(80);
        legend->RightAxisVisibilityOff();
        scalarBar->SetPosition(0.80, 0.10);
        scalarBar->SetLookupTable(lut);
        renderer->AddActor(scalarBar);
        scalarBar->SetVisibility(checked);
    } else {
        legend->RightAxisVisibilityOn();
    }
    
    ui->qVtkSlice->renderWindow()->Render();
}

void vtkWindowCube::changeFitsScale(std::string palette, std::string scale, float min, float max)
{
    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
    
    QString myscale = scale.c_str();
    lutName = QString::fromUtf8(palette.c_str());
    lut->SetTableRange(min, max);
    if (myscale == "Linear")
        lut->SetScaleToLinear();
    else if (myscale == "Log")
        lut->SetScaleToLog10();
    
    SelectLookTable(palette.c_str(), lut);
    if (currentVisOnSlicePanel == 0)
        sliceViewer->GetWindowLevel()->SetLookupTable(lut);
    else
        momViewer->GetWindowLevel()->SetLookupTable(lut);
    
    if (scalarBar) {
        showColorbar(true, min, max);
    }
    
    ui->qVtkSlice->renderWindow()->Render();
}

void vtkWindowCube::on_actionExtract_spectrum_triggered()
{
    setInteractorStyleProfile();
}

void vtkWindowCube::extractSpectrum(double x, double y, bool live)
{
    if (!profileWin) {
        int naxis3 = fitsHeader.value("NAXIS3").toInt();
        double crval3 = fitsHeader.value("CRVAL3").toDouble();
        double cdelt3 = fitsHeader.value("CDELT3").toDouble();
        double crpix3 = fitsHeader.value("CRPIX3").toDouble();
        QString cunit3 = fitsHeader.value("CUNIT3");
        cunit3.replace("'", QString());
        cunit3.replace("\"", QString());
        cunit3 = cunit3.simplified();
        
        profileWin = new ProfileWindow(bunit);
        if (cunit3.contains("Hz")) {
            double restfrq = fitsHeader.value("RESTFRQ").toDouble();
            profileWin->setupSpectrumPlot(naxis3, crval3, cdelt3, crpix3, restfrq);
        } else {
            profileWin->setupSpectrumPlot(naxis3, crval3, cdelt3, crpix3, cunit3);
        }
        profileWin->show();
        
        connect(profileWin, &ProfileWindow::liveUpdateStateChanged, this, [this](int status) {
            if (status) {
                setInteractorStyleProfile(status);
            } else {
                setInteractorStyleImage();
            }
        });
        
        connect(profileWin, &ProfileWindow::destroyed, this,
                &vtkWindowCube::setInteractorStyleImage);
    }
    
    if (!live) {
        setInteractorStyleImage();
        profileWin->setLiveProfileFlag(live);
    }
    
    double nulval = 0;
    auto vectors = AstroUtils::extractSpectrum(filepath.toStdString().c_str(), x, y, nulval);
    profileWin->plotSpectrum(vectors.first, vectors.second, x, y, nulval);
    profileWin->raise();
}

void vtkWindowCube::on_actionPV_triggered()
{
    setInteractorStyleDrawLine();
}

void vtkWindowCube::on_actionFilter_triggered()
{
    FilterFITSDialog d(this->filepath, this);
    d.exec();
}

void vtkWindowCube::initializeMenuConnections()
{
    connect(visivoMenu, &VisIVOMenu::cameraFrontTriggered, this, &vtkWindowCube::on_actionFront_triggered);
    connect(visivoMenu, &VisIVOMenu::cameraBackTriggered, this, &vtkWindowCube::on_actionBack_triggered);
    connect(visivoMenu, &VisIVOMenu::cameraTopTriggered, this, &vtkWindowCube::on_actionTop_triggered);
    connect(visivoMenu, &VisIVOMenu::cameraBottomTriggered, this, &vtkWindowCube::on_actionBottom_triggered);
    connect(visivoMenu, &VisIVOMenu::cameraLeftTriggered, this, &vtkWindowCube::on_actionLeft_triggered);
    connect(visivoMenu, &VisIVOMenu::cameraRightTriggered, this, &vtkWindowCube::on_actionRight_triggered);
}


void vtkWindowCube::changeEvent(QEvent *e)
{
    if(e->type() == QEvent::ActivationChange && this->isActiveWindow()) {
        visivoMenu->configureCubeWindowMenu();
        initializeMenuConnections(); // Chiamata solo quando l'oggetto è attivato
        /*
         connect(visivoMenu, &VisIVOMenu::cameraFrontTriggered, this, &vtkWindowCube::on_actionFront_triggered);
         connect(visivoMenu, &VisIVOMenu::cameraBackTriggered, this, &vtkWindowCube::on_actionBack_triggered);
         connect(visivoMenu, &VisIVOMenu::cameraTopTriggered, this,
         &vtkWindowCube::on_actionTop_triggered);
         connect(visivoMenu, &VisIVOMenu::cameraBottomTriggered, this, &vtkWindowCube::on_actionBottom_triggered);
         connect(visivoMenu, &VisIVOMenu::cameraLeftTriggered, this, &vtkWindowCube::on_actionLeft_triggered);
         connect(visivoMenu, &VisIVOMenu::cameraRightTriggered, this, &vtkWindowCube::on_actionRight_triggered);
         */
        
    }
}
