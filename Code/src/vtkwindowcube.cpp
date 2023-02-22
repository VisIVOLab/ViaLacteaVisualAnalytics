#include "vtkwindowcube.h"
#include "ui_lutcustomize.h"
#include "ui_vtkwindowcube.h"

#include "interactors/vtkinteractorstyleimagecustom.h"

#include "astroutils.h"
#include "fitsimagestatisiticinfo.h"
#include "lutcustomize.h"
#include "luteditor.h"
#include "vtkfitsreader.h"
#include "vtkfitsreader2.h"
#include "vtklegendscaleactorwcs.h"
#include "vtkwindow_new.h"

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

vtkWindowCube::vtkWindowCube(QWidget *parent, const QString &filepath, int ScaleFactor,
                             QString velocityUnit)
    : QMainWindow(parent),
      ui(new Ui::vtkWindowCube),
      filepath(filepath),
      ScaleFactor(ScaleFactor),
      parentWindow(qobject_cast<vtkwindow_new *>(parent)),
      currentSlice(0),
      velocityUnit(velocityUnit)
{
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

    ui->menuWCS->addActions(wcsGroup->actions());

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

    // Create FITS Stats Widget
    fitsStatsWidget = new FitsImageStatisiticInfo(readerCube, readerSlice, this);
    on_actionShowStats_triggered();

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
    connect(ui->actionShowMomentMap, &QAction::triggered, this, [=]() { changeSliceView(1); });

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

    // Show cube slices by default
    currentVisOnSlicePanel = 0;
    renWinSlice->AddRenderer(rendererSlice);

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

    auto interactorStyle = vtkSmartPointer<vtkInteractorStyleImageCustom>::New();
    interactorStyle->SetCoordsCallback([this](std::string str) { showStatusBarMessage(str); });
    interactorStyle->SetReader(readerSlice);
    ui->qVtkSlice->renderWindow()->GetInteractor()->SetInteractorStyle(interactorStyle);

    rendererSlice->ResetCamera();
    renWinSlice->GetInteractor()->Render();

    // Set legend WCS
    QString ctype1 = fitsHeader.value("CTYPE1").toUpper();
    if (ctype1.startsWith("GL")) {
        legendActorCube->setWCS(WCS_GALACTIC);
        legendActorSlice->setWCS(WCS_GALACTIC);
        legendActorMoment->setWCS(WCS_GALACTIC);
        ui->menuWCS->actions().at(1)->setChecked(true);
    } else if (ctype1.startsWith("RA")) {
        // FK5
        legendActorCube->setWCS(WCS_J2000);
        legendActorSlice->setWCS(WCS_J2000);
        legendActorMoment->setWCS(WCS_J2000);
        ui->menuWCS->actions().at(2)->setChecked(true);
    }

    ui->thresholdGroupBox->setTitle(ui->thresholdGroupBox->title() + " ("
                                    + fitsHeader.value("BUNIT") + ")");
    ui->contourGroupBox->setTitle(ui->contourGroupBox->title() + " (" + fitsHeader.value("BUNIT")
                                  + ")");
}

vtkWindowCube::~vtkWindowCube()
{
    if (parentWindow) {
        parentWindow->removeActor(contoursActorForParent);
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
        ui->actionShowSlice->setChecked(true);
        if (lcustom) {
            lcustom->configureFits3D();
        }
        break;
    case 1:
        currentVisOnSlicePanel = 1;
        ui->qVtkSlice->renderWindow()->RemoveRenderer(rendererSlice);
        ui->qVtkSlice->renderWindow()->AddRenderer(rendererMoment);
        ui->actionShowMomentMap->setChecked(true);
        if (lcustom) {
            lcustom->configureMoment();
        }
        break;
    }

    ui->qVtkSlice->renderWindow()->GetRenderers()->GetFirstRenderer()->ResetCamera();
    ui->qVtkSlice->renderWindow()->Render();
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
        if ((strlen(name) == 0) || (strcasecmp(name, "HISTORY") == 0)
            || (strcasecmp(name, "COMMENT") == 0)) {
            continue;
        }

        this->fitsHeader.insert(QString::fromUtf8(name), QString::fromUtf8(value));
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

    fitsStatsWidget->updateSliceStats();

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
    double velocity = initSlice + cdelt3 * currentSlice;

    if (fitsHeader.value("CUNIT3").startsWith("m")) {
        // Return value in km/s
        velocity /= 1000;
    }
    ui->velocityText->setText(QString::number(velocity).append(" Km/s"));
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

    if (parentWindow) {
        parentWindow->addLayerImage(moment);
    } else {
        parentWindow = new vtkwindow_new(nullptr, moment, 0, 0, false);
        parentWindow->showMaximized();
    }

    momViewer->SetInputData(moment->GetOutput());
    momViewer->Render();
    changeSliceView(1);
    this->activateWindow();
    this->raise();
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

void vtkWindowCube::on_actionShowStats_triggered()
{
    if (!dock) {
        dock = new QDockWidget(this);
        dock->setWidget(fitsStatsWidget);
        dock->setAllowedAreas(Qt::RightDockWidgetArea);
    }

    addDockWidget(Qt::RightDockWidgetArea, dock);
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
    if (scalarBar != 0) {
        ui->qVtkSlice->renderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(scalarBar);
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
        legendActorSlice->RightAxisVisibilityOff();
        scalarBar->SetPosition(0.80, 0.10);
        scalarBar->SetLookupTable(lut);
        auto renderer = ui->qVtkSlice->renderWindow()->GetRenderers()->GetFirstRenderer();
        renderer->AddActor(scalarBar);
        scalarBar->SetVisibility(checked);
    } else {
        legendActorSlice->RightAxisVisibilityOn();
    }
    ui->qVtkSlice->update();
    ui->qVtkSlice->renderWindow()->GetInteractor()->Render();
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
    ui->qVtkSlice->update();
    ui->qVtkSlice->renderWindow()->GetInteractor()->Render();
}
