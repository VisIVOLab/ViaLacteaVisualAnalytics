#include "vtkWindowCatalogue3D.h"
#include "ui_vtkWindowCatalogue3D.h"

#include "Catalogue3DParser.h"

// ── VTK ──────────────────────────────────────────────────────────────────────
#include <QVTKOpenGLNativeWidget.h>
#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkBillboardTextActor3D.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCubeAxesActor.h>
#include <vtkCoordinate.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkGlyph3D.h>
#include <vtkIntArray.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkLookupTable.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSphereSource.h>
#include <vtkTextProperty.h>

// ── Qt ───────────────────────────────────────────────────────────────────────
#include <QCheckBox>
#include <QDebug>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QSlider>

// ── std ───────────────────────────────────────────────────────────────────────
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

using namespace Qt::StringLiterals;

// ── Anonymous helpers ─────────────────────────────────────────────────────────
namespace {

// Fixed palette for recognised morphology classes (RGB, 0-1).
// Unknown classes are assigned from the same cycle by index modulo.
const std::array<std::array<double, 3>, 10> kPalette = { {
        { 0.58, 0.18, 0.92 }, // 0 – halo        – purple
        { 1.00, 0.50, 0.00 }, // 1 – relic        – orange
        { 0.00, 0.82, 0.90 }, // 2 – mini-halo    – cyan
        { 0.70, 0.90, 0.10 }, // 3 – double relic – lime
        { 0.90, 0.25, 0.25 }, // 4 – phoenix      – red
        { 0.20, 0.75, 0.35 }, // 5 – extended     – green
        { 0.95, 0.80, 0.00 }, // 6 – diffuse      – gold
        { 0.30, 0.55, 0.95 }, // 7 – candidate    – blue
        { 0.80, 0.45, 0.80 }, // 8 – unclear      – pink
        { 0.70, 0.70, 0.70 }, // 9 – other/fallback – gray
} };

// Canonical mapping: morphology keyword → palette index.
const QMap<QString, int> kKnownMorphologies = {
        { QStringLiteral("halo"),         0 },
        { QStringLiteral("relic"),        1 },
        { QStringLiteral("mini-halo"),    2 },
        { QStringLiteral("minihalo"),     2 },
        { QStringLiteral("mini halo"),    2 },
        { QStringLiteral("double relic"), 3 },
        { QStringLiteral("doublerelic"),  3 },
        { QStringLiteral("phoenix"),      4 },
        { QStringLiteral("extended"),     5 },
        { QStringLiteral("diffuse"),      6 },
        { QStringLiteral("candidate"),    7 },
        { QStringLiteral("unclear"),      8 },
        { QStringLiteral("other"),        9 },
};

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// Construction / destruction
// ─────────────────────────────────────────────────────────────────────────────

vtkWindowCatalogue3D::vtkWindowCatalogue3D(const QString &filepath, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::vtkWindowCatalogue3D)
    , filepath(filepath)
{
    ui->setupUi(this);

    // ── Parse catalogue ──────────────────────────────────────────────────────
    const auto parsed = Catalogue3DParser::parseFile(filepath);
    if (!parsed.valid) {
        QMessageBox::critical(this, u"Catalogue Error"_s, parsed.errorMessage);
        // Window still opens – empty scene with error shown in status.
        ui->labelStatus->setText(u"Load error: "_s + parsed.errorMessage);
    } else {
        entries = parsed.entries;
        if (parsed.skippedEntries > 0) {
            ui->labelStatus->setText(
                    QStringLiteral("Loaded %1 sources (%2 rows skipped).")
                            .arg(entries.size())
                            .arg(parsed.skippedEntries));
        } else {
            ui->labelStatus->setText(
                    QStringLiteral("Loaded %1 sources.").arg(entries.size()));
        }
    }

    setWindowTitle(QStringLiteral("3D Catalogue — %1  (%2 sources)")
                           .arg(QFileInfo(filepath).fileName())
                           .arg(entries.size()));

    // ── VTK scene ────────────────────────────────────────────────────────────
    setupRenderer();
    if (!entries.empty()) {
        buildScene();
        buildLabels();
    }

    // ── Connect UI controls ───────────────────────────────────────────────────
    QObject::connect(ui->chkLabels, &QCheckBox::toggled, this,
                     &vtkWindowCatalogue3D::toggleLabels);
    QObject::connect(ui->chkMorphColors, &QCheckBox::toggled, this,
                     &vtkWindowCatalogue3D::toggleMorphColors);
    QObject::connect(ui->sliderScale, &QSlider::valueChanged, this,
                     &vtkWindowCatalogue3D::scaleChanged);

    QObject::connect(ui->actionShowLabels, &QAction::toggled, ui->chkLabels,
                     &QCheckBox::setChecked);
    QObject::connect(ui->chkLabels, &QCheckBox::toggled, ui->actionShowLabels,
                     &QAction::setChecked);
    QObject::connect(ui->actionColorByMorphology, &QAction::toggled, ui->chkMorphColors,
                     &QCheckBox::setChecked);
    QObject::connect(ui->chkMorphColors, &QCheckBox::toggled, ui->actionColorByMorphology,
                     &QAction::setChecked);
    QObject::connect(ui->actionResetCamera, &QAction::triggered, this,
                     &vtkWindowCatalogue3D::resetCamera);
}

vtkWindowCatalogue3D::~vtkWindowCatalogue3D()
{
    delete ui;
}

// ─────────────────────────────────────────────────────────────────────────────
// Setup
// ─────────────────────────────────────────────────────────────────────────────

void vtkWindowCatalogue3D::setupRenderer()
{
    // Attach render window to the Qt VTK widget.
    ui->vtk->setRenderWindow(renderWindow.Get());

    renderer->SetBackground(0.06, 0.06, 0.12);
    renderWindow->AddRenderer(renderer);

    // Trackball camera – standard 3D navigation.
    auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    ui->vtk->interactor()->SetInteractorStyle(style);

    // Orientation axes widget (bottom-left corner, non-interactive).
    vtkNew<vtkAxesActor> axesActor;
    axesWidget->SetOrientationMarker(axesActor);
    axesWidget->SetInteractor(ui->vtk->interactor());
    axesWidget->SetViewport(0.0, 0.0, 0.14, 0.14);
    axesWidget->EnabledOn();
    axesWidget->InteractiveOff();

    // ── Mouse observers ───────────────────────────────────────────────────────
    auto cb = vtkSmartPointer<vtkCallbackCommand>::New();
    cb->SetCallback([](vtkObject *, unsigned long eid, void *clientData, void *) {
        static_cast<vtkWindowCatalogue3D *>(clientData)->onMouseEvent(eid);
    });
    cb->SetClientData(this);

    auto *inter = ui->vtk->interactor();
    inter->AddObserver(vtkCommand::MouseMoveEvent,         cb);
    inter->AddObserver(vtkCommand::LeftButtonPressEvent,   cb);
    inter->AddObserver(vtkCommand::LeftButtonReleaseEvent, cb);
}

// ─────────────────────────────────────────────────────────────────────────────
// Scene construction
// ─────────────────────────────────────────────────────────────────────────────

/*static*/
std::array<double, 3> vtkWindowCatalogue3D::morphologyColor(const QString &morph,
                                                             int fallbackIndex)
{
    if (kKnownMorphologies.contains(morph))
        return kPalette[kKnownMorphologies[morph]];
    return kPalette[fallbackIndex % static_cast<int>(kPalette.size())];
}

void vtkWindowCatalogue3D::buildMorphologyLut()
{
    // Collect ordered unique morphologies preserving first-seen order.
    for (const auto &e : entries) {
        if (!morphologyIndexOf.contains(e.morphology)) {
            morphologyIndexOf[e.morphology] = morphologyNames.size();
            morphologyNames.append(e.morphology);
        }
    }

    const int n = morphologyNames.size();
    morphologyLut->SetNumberOfTableValues(n);
    // Range: use [-0.5, n-0.5] so each integer index falls in its own bin.
    morphologyLut->SetTableRange(-0.5, n - 0.5);

    for (int i = 0; i < n; ++i) {
        const auto [r, g, b] = morphologyColor(morphologyNames.at(i), i);
        morphologyLut->SetTableValue(i, r, g, b, 1.0);
    }
    morphologyLut->Build();
}

void vtkWindowCatalogue3D::buildScene()
{
    buildMorphologyLut();

    // ── Populate point cloud ──────────────────────────────────────────────────
    sourcePoints->SetNumberOfPoints(static_cast<vtkIdType>(entries.size()));

    morphScalars->SetName("Morphology");
    morphScalars->SetNumberOfValues(static_cast<vtkIdType>(entries.size()));

    // Compute bounding box to derive a sensible default glyph radius.
    double xMin = std::numeric_limits<double>::max();
    double yMin = xMin, zMin = xMin;
    double xMax = std::numeric_limits<double>::lowest();
    double yMax = xMax, zMax = xMax;

    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
        const auto &e = entries[i];
        sourcePoints->SetPoint(i, e.sceneX, e.sceneY, e.sceneZ);
        morphScalars->SetValue(i, morphologyIndexOf.value(e.morphology, 0));

        xMin = std::min(xMin, e.sceneX); xMax = std::max(xMax, e.sceneX);
        yMin = std::min(yMin, e.sceneY); yMax = std::max(yMax, e.sceneY);
        zMin = std::min(zMin, e.sceneZ); zMax = std::max(zMax, e.sceneZ);
    }

    const double diag = std::sqrt((xMax - xMin) * (xMax - xMin)
                                  + (yMax - yMin) * (yMax - yMin)
                                  + (zMax - zMin) * (zMax - zMin));
    // Default glyph radius ≈ 1.5 % of diagonal so single sources remain visible.
    defaultGlyphRadius = std::max(1.0, diag * 0.015);

    // Vertices cell array so vtkGlyph3D recognises each point.
    vtkNew<vtkCellArray> verts;
    for (vtkIdType i = 0; i < static_cast<vtkIdType>(entries.size()); ++i)
        verts->InsertNextCell(1, &i);

    sourcesPolyData->SetPoints(sourcePoints);
    sourcesPolyData->SetVerts(verts);
    sourcesPolyData->GetPointData()->SetScalars(morphScalars);

    // ── Glyph pipeline ────────────────────────────────────────────────────────
    sphereSource->SetRadius(0.5); // unit sphere; scaled by SetScaleFactor
    sphereSource->SetPhiResolution(10);
    sphereSource->SetThetaResolution(10);

    glyphs->SetInputData(sourcesPolyData);
    glyphs->SetSourceConnection(sphereSource->GetOutputPort());
    glyphs->SetScaleModeToDataScalingOff(); // uniform size from SetScaleFactor
    glyphs->SetColorModeToColorByScalar();  // pass morphology index to mapper
    glyphs->SetScaleFactor(defaultGlyphRadius);

    const int nMorph = morphologyNames.size();
    glyphMapper->SetInputConnection(glyphs->GetOutputPort());
    glyphMapper->SetLookupTable(morphologyLut);
    glyphMapper->SetScalarRange(-0.5, nMorph - 0.5);
    glyphMapper->SetColorModeToMapScalars();
    glyphMapper->ScalarVisibilityOn();

    glyphActor->SetMapper(glyphMapper);
    renderer->AddActor(glyphActor);

    // ── Hover highlight (yellow wireframe) ────────────────────────────────────
    hoverSphere->SetRadius(defaultGlyphRadius * 1.35);
    hoverSphere->SetPhiResolution(12);
    hoverSphere->SetThetaResolution(12);

    hoverMapper->SetInputConnection(hoverSphere->GetOutputPort());
    hoverActor->SetMapper(hoverMapper);
    hoverActor->GetProperty()->SetRepresentationToWireframe();
    hoverActor->GetProperty()->SetColor(1.0, 0.95, 0.0); // yellow
    hoverActor->GetProperty()->SetLineWidth(1.8);
    hoverActor->VisibilityOff();
    renderer->AddActor(hoverActor);

    // ── Selection highlight (bright white wireframe) ───────────────────────────
    selectSphere->SetRadius(defaultGlyphRadius * 1.18);
    selectSphere->SetPhiResolution(12);
    selectSphere->SetThetaResolution(12);

    selectMapper->SetInputConnection(selectSphere->GetOutputPort());
    selectActor->SetMapper(selectMapper);
    selectActor->GetProperty()->SetRepresentationToWireframe();
    selectActor->GetProperty()->SetColor(1.0, 1.0, 1.0); // white
    selectActor->GetProperty()->SetLineWidth(2.2);
    selectActor->VisibilityOff();
    renderer->AddActor(selectActor);

    // ── Bounding-box coordinate axes ──────────────────────────────────────────
    const double bounds[6] = { xMin, xMax, yMin, yMax, zMin, zMax };
    cubeAxesActor->SetBounds(bounds);
    cubeAxesActor->SetCamera(renderer->GetActiveCamera());
    cubeAxesActor->SetXTitle("X (Mpc)");
    cubeAxesActor->SetYTitle("Y (Mpc)");
    cubeAxesActor->SetZTitle("Z (Mpc)");
    cubeAxesActor->SetXUnits("");
    cubeAxesActor->SetYUnits("");
    cubeAxesActor->SetZUnits("");
    // Axis colours: red / green / blue
    for (int axis = 0; axis < 3; ++axis) {
        cubeAxesActor->GetTitleTextProperty(axis)->SetColor(
                axis == 0 ? 1.0 : 0.5, axis == 1 ? 1.0 : 0.5, axis == 2 ? 1.0 : 0.5);
        cubeAxesActor->GetLabelTextProperty(axis)->SetColor(0.8, 0.8, 0.8);
    }
    cubeAxesActor->SetFlyModeToOuterEdges();
    cubeAxesActor->DrawXGridlinesOff();
    cubeAxesActor->DrawYGridlinesOff();
    cubeAxesActor->DrawZGridlinesOff();
    renderer->AddActor(cubeAxesActor);

    renderer->ResetCamera();
    renderWindow->Render();
}

void vtkWindowCatalogue3D::buildLabels()
{
    for (const auto &e : entries) {
        auto actor = vtkSmartPointer<vtkBillboardTextActor3D>::New();
        actor->SetInput(e.name.toUtf8().constData());
        actor->SetPosition(e.sceneX, e.sceneY, e.sceneZ);
        actor->GetTextProperty()->SetFontSize(11);
        actor->GetTextProperty()->SetColor(1.0, 1.0, 1.0);
        actor->GetTextProperty()->ShadowOn();
        actor->GetTextProperty()->SetShadowOffset(1, -1);
        actor->SetVisibility(0); // hidden until user enables
        labelActors.push_back(actor);
        renderer->AddActor(actor);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Interaction
// ─────────────────────────────────────────────────────────────────────────────

void vtkWindowCatalogue3D::onMouseEvent(unsigned long eid)
{
    int x = 0, y = 0;
    ui->vtk->interactor()->GetEventPosition(x, y);

    if (eid == vtkCommand::LeftButtonPressEvent) {
        leftButtonDown = true;
        pressX = x;
        pressY = y;
        // Let the interactor style handle camera rotation — no early return needed.
        return;
    }

    if (eid == vtkCommand::LeftButtonReleaseEvent) {
        leftButtonDown = false;
        // Treat as a click only when the pointer hasn't moved significantly.
        const int dx = x - pressX;
        const int dy = y - pressY;
        if (dx * dx + dy * dy <= clickDragThresholdPx * clickDragThresholdPx
            && !entries.empty()) {
            const int idx = pickNearestSource(x, y);
            setSelectedSource(idx == selectedIndex ? -1 : idx);
        }
        return;
    }

    // MouseMoveEvent: update hover only when no button is held (pure hover).
    if (!leftButtonDown && !entries.empty())
        setHoveredSource(pickNearestSource(x, y));
}

int vtkWindowCatalogue3D::pickNearestSource(int displayX, int displayY) const
{
    vtkNew<vtkCoordinate> coord;
    coord->SetCoordinateSystemToWorld();

    double bestDist2 = hoverThresholdPx * hoverThresholdPx;
    int bestIdx = -1;

    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
        const auto &e = entries[i];
        coord->SetValue(e.sceneX, e.sceneY, e.sceneZ);
        // GetComputedDisplayValue returns int pixel coords [x, y].
        const int *dp = coord->GetComputedDisplayValue(renderer.Get());
        const double dx = dp[0] - displayX;
        const double dy = dp[1] - displayY;
        const double dist2 = dx * dx + dy * dy;
        if (dist2 < bestDist2) {
            bestDist2 = dist2;
            bestIdx = i;
        }
    }
    return bestIdx;
}

void vtkWindowCatalogue3D::updateHighlightSphere(vtkSphereSource *sphere, vtkActor *actor,
                                                  int idx, double radiusFactor)
{
    if (idx < 0) {
        actor->VisibilityOff();
        return;
    }
    const auto &e = entries[idx];
    const double r = defaultGlyphRadius
            * (ui->sliderScale->value() / 100.0)
            * radiusFactor;
    sphere->SetCenter(e.sceneX, e.sceneY, e.sceneZ);
    sphere->SetRadius(r);
    sphere->Update();
    actor->VisibilityOn();
}

void vtkWindowCatalogue3D::setHoveredSource(int idx)
{
    if (idx == hoveredIndex)
        return;
    hoveredIndex = idx;
    updateHighlightSphere(hoverSphere.Get(), hoverActor.Get(), idx, 1.35);
    renderWindow->Render();
}

void vtkWindowCatalogue3D::setSelectedSource(int idx)
{
    selectedIndex = idx;
    updateHighlightSphere(selectSphere.Get(), selectActor.Get(), idx, 1.18);
    updateInfoPanel();
    renderWindow->Render();
}

// ─────────────────────────────────────────────────────────────────────────────
// Info panel
// ─────────────────────────────────────────────────────────────────────────────

void vtkWindowCatalogue3D::updateInfoPanel()
{
    if (selectedIndex < 0) {
        ui->labelInfo->setText(u"No source selected"_s);
        return;
    }

    const Catalogue3DEntry &e = entries[selectedIndex];
    QString html;
    html += QStringLiteral("<b>%1</b>").arg(e.name.toHtmlEscaped());
    html += QStringLiteral("<br>RA: %1°").arg(e.raDeg, 0, 'f', 4);
    html += QStringLiteral("<br>Dec: %1°").arg(e.decDeg, 0, 'f', 4);
    if (e.redshift > 0.0)
        html += QStringLiteral("<br>z: %1").arg(e.redshift, 0, 'f', 4);
    if (e.distanceMpc > 0.0)
        html += QStringLiteral("<br>d: %1 Mpc").arg(e.distanceMpc, 0, 'f', 1);
    html += QStringLiteral("<br>Type: <i>%1</i>").arg(e.morphology.toHtmlEscaped());
    if (e.majorAxisArcmin > 0.0) {
        if (e.minorAxisArcmin > 0.0)
            html += QStringLiteral("<br>Size: %1′ × %2′")
                            .arg(e.majorAxisArcmin, 0, 'f', 1)
                            .arg(e.minorAxisArcmin, 0, 'f', 1);
        else
            html += QStringLiteral("<br>Size: %1′").arg(e.majorAxisArcmin, 0, 'f', 1);
    }
    if (e.fluxMJy > 0.0)
        html += QStringLiteral("<br>Flux: %1 mJy").arg(e.fluxMJy, 0, 'f', 2);

    ui->labelInfo->setText(html);
}

// ─────────────────────────────────────────────────────────────────────────────
// Slots
// ─────────────────────────────────────────────────────────────────────────────

void vtkWindowCatalogue3D::toggleLabels(bool checked)
{
    for (auto &actor : labelActors)
        actor->SetVisibility(checked ? 1 : 0);
    if (!entries.empty())
        renderWindow->Render();
}

void vtkWindowCatalogue3D::toggleMorphColors(bool checked)
{
    glyphMapper->SetScalarVisibility(checked ? 1 : 0);
    if (!checked)
        glyphActor->GetProperty()->SetColor(0.75, 0.75, 0.85);
    if (!entries.empty())
        renderWindow->Render();
}

void vtkWindowCatalogue3D::scaleChanged(int value)
{
    if (entries.empty())
        return;
    const double scale = defaultGlyphRadius * (value / 100.0);
    glyphs->SetScaleFactor(scale);
    glyphs->Modified();

    // Keep highlight spheres consistent with current scale.
    updateHighlightSphere(hoverSphere.Get(),  hoverActor.Get(),  hoveredIndex,  1.35);
    updateHighlightSphere(selectSphere.Get(), selectActor.Get(), selectedIndex, 1.18);

    renderWindow->Render();
}

void vtkWindowCatalogue3D::resetCamera()
{
    renderer->ResetCamera();
    renderWindow->Render();
}
