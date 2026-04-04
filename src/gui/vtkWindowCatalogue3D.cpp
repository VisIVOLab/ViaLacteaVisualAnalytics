#include "vtkWindowCatalogue3D.h"
#include "ui_vtkWindowCatalogue3D.h"

#include "Catalogue3DTableModel.h"
#include "Catalogue3DParser.h"

// ── VTK ──────────────────────────────────────────────────────────────────────
#include <QVTKOpenGLNativeWidget.h>
#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkAxesActor.h>
#include <vtkBillboardTextActor3D.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCubeSource.h>
#include <vtkCubeAxesActor.h>
#include <vtkCoordinate.h>
#include <vtkFloatArray.h>
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
#include <QComboBox>
#include <QDebug>
#include <QDockWidget>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QSlider>
#include <QTableView>
#include <QTextEdit>
#include <QVBoxLayout>

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

    this->comboGeometry = new QComboBox(this);
    this->comboGeometry->addItems({ u"Ellipsoid"_s, u"Sphere"_s, u"Point"_s, u"Cross"_s });
    this->comboSizeMode = new QComboBox(this);
    this->comboSizeMode->addItems({ u"Fixed"_s, u"Major axis"_s, u"LLS"_s, u"Flux"_s });
    this->comboFrame = new QComboBox(this);
    this->comboFrame->addItems({ u"FK5 / J2000"_s, u"Galactic (future)"_s });
    this->comboFrame->setCurrentIndex(0);
    this->comboFrame->setEnabled(false);
    this->chkShowAxes = new QCheckBox(u"Show axes"_s, this);
    this->chkShowAxes->setChecked(true);
    this->chkShowBoundingBox = new QCheckBox(u"Show bounding box"_s, this);
    this->chkShowBoundingBox->setChecked(false);
    this->chkShowShells = new QCheckBox(u"Show redshift shells"_s, this);
    this->chkShowShells->setChecked(false);

    auto *geometryGroup = new QGroupBox(u"Geometry"_s, this);
    auto *geometryLayout = new QFormLayout(geometryGroup);
    geometryLayout->addRow(u"Shape"_s, this->comboGeometry);
    auto *sizeGroup = new QGroupBox(u"Size"_s, this);
    auto *sizeLayout = new QFormLayout(sizeGroup);
    sizeLayout->addRow(u"Scale"_s, ui->sliderScale);
    auto *sceneGroup = new QGroupBox(u"Scene"_s, this);
    auto *sceneLayout = new QFormLayout(sceneGroup);
    sceneLayout->addRow(this->chkShowAxes);
    sceneLayout->addRow(this->chkShowBoundingBox);
    sceneLayout->addRow(this->chkShowShells);
    sceneLayout->addRow(u"Frame"_s, this->comboFrame);
    auto *vizLayout = qobject_cast<QVBoxLayout *>(ui->groupViz->layout());
    if (vizLayout) {
        while (vizLayout->count() > 0) {
            vizLayout->takeAt(0);
        }
        vizLayout->addWidget(geometryGroup);
        vizLayout->addWidget(sizeGroup);
        vizLayout->addWidget(sceneGroup);
        vizLayout->addWidget(ui->chkLabels);
        vizLayout->addWidget(ui->chkMorphColors);
    }
    ui->labelStatus->setToolTip(
            u"3D positions use RA/Dec + redshift or distance. Source geometry is parametric."_s);

    // ── Parse catalogue ──────────────────────────────────────────────────────
    const auto parsed = Catalogue3DParser::parseFile(filepath);
    if (!parsed.valid) {
        QMessageBox::critical(this, u"Catalogue Error"_s, parsed.errorMessage);
        // Window still opens – empty scene with error shown in status.
        ui->labelStatus->setText(u"Load error: "_s + parsed.errorMessage);
    } else {
        entries = parsed.entries;
        rawHeaders = parsed.headers;
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
    this->ensureCatalogueDock();
    this->populateMappingControls();
    this->refreshCatalogueTable();

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
    QObject::connect(this->comboGeometry, &QComboBox::currentIndexChanged, this,
                     [this](int) { this->updateGeometryMode(); });
    QObject::connect(this->comboSizeMode, &QComboBox::currentIndexChanged, this,
                     [this](int) { this->updateSizeMode(); });
    QObject::connect(this->chkShowAxes, &QCheckBox::toggled, this,
                     &vtkWindowCatalogue3D::toggleSceneAxes);
    QObject::connect(this->chkShowBoundingBox, &QCheckBox::toggled, this,
                     &vtkWindowCatalogue3D::toggleBoundingBox);
    QObject::connect(this->chkShowShells, &QCheckBox::toggled, this,
                     &vtkWindowCatalogue3D::toggleShells);
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
    ui->vtk->setEnableTouchEventProcessing(false);
    qInfo() << "[catalogue3d] widget type:" << ui->vtk->metaObject()->className()
            << "touch-processing disabled to match vtkWindowCube";

    renderer->SetBackground(0.06, 0.06, 0.12);
    renderWindow->AddRenderer(renderer);

    // Trackball camera – standard 3D navigation, matching the cube viewer approach:
    // use the stock VTK style and keep catalogue logic in separate observers.
    auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    ui->vtk->interactor()->SetInteractorStyle(style);
    qInfo() << "[catalogue3d] installed interactor style:" << style->GetClassName();

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
    inter->AddObserver(vtkCommand::LeftButtonDoubleClickEvent, cb);
    qInfo() << "[catalogue3d] observer registration complete;"
            << "hover observer is separate from camera interaction";
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

    this->valueLut->SetNumberOfTableValues(256);
    this->valueLut->SetHueRange(0.67, 0.0);
    this->valueLut->SetSaturationRange(0.85, 0.95);
    this->valueLut->SetValueRange(0.9, 1.0);
    this->valueLut->Build();
}

void vtkWindowCatalogue3D::buildScene()
{
    buildMorphologyLut();
    this->mappedPositions.resize(entries.size());

    // ── Populate point cloud ──────────────────────────────────────────────────
    sourcePoints->SetNumberOfPoints(static_cast<vtkIdType>(entries.size()));

    morphScalars->SetName("Morphology");
    morphScalars->SetNumberOfValues(static_cast<vtkIdType>(entries.size()));

    // Compute bounding box to derive a sensible default glyph radius.
    double xMin = std::numeric_limits<double>::max();
    double yMin = xMin, zMin = xMin;
    double xMax = std::numeric_limits<double>::lowest();
    double yMax = xMax, zMax = xMax;

    maxDistanceMpc = 1.0;
    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
        const auto &e = entries[i];
        morphScalars->SetValue(i, morphologyIndexOf.value(e.morphology, 0));

        xMin = std::min(xMin, e.sceneX); xMax = std::max(xMax, e.sceneX);
        yMin = std::min(yMin, e.sceneY); yMax = std::max(yMax, e.sceneY);
        zMin = std::min(zMin, e.sceneZ); zMax = std::max(zMax, e.sceneZ);
        maxDistanceMpc = std::max(maxDistanceMpc, e.distanceMpc);
    }

    const double diag = std::sqrt((xMax - xMin) * (xMax - xMin)
                                  + (yMax - yMin) * (yMax - yMin)
                                  + (zMax - zMin) * (zMax - zMin));
    // Default glyph radius ≈ 1.2 % of diagonal so fields remain readable.
    defaultGlyphRadius = std::max(0.5, diag * 0.012);
    sizeNormalizationValue = 1.0;

    // Vertices cell array so vtkGlyph3D recognises each point.
    vtkNew<vtkCellArray> verts;
    for (vtkIdType i = 0; i < static_cast<vtkIdType>(entries.size()); ++i)
        verts->InsertNextCell(1, &i);

    sourcesPolyData->SetPoints(sourcePoints);
    sourcesPolyData->SetVerts(verts);
    sourcesPolyData->GetPointData()->SetScalars(morphScalars);
    this->glyphScaleVectors->SetName("ScaleVector");
    this->glyphScaleVectors->SetNumberOfComponents(3);
    this->glyphScaleVectors->SetNumberOfTuples(static_cast<vtkIdType>(entries.size()));
    sourcesPolyData->GetPointData()->SetVectors(this->glyphScaleVectors);

    // ── Glyph pipeline ────────────────────────────────────────────────────────
    sphereSource->SetRadius(0.5); // unit sphere; scaled by SetScaleFactor
    sphereSource->SetPhiResolution(10);
    sphereSource->SetThetaResolution(10);
    pointCubeSource->SetXLength(1.0);
    pointCubeSource->SetYLength(1.0);
    pointCubeSource->SetZLength(1.0);
    vtkNew<vtkPoints> crossPoints;
    vtkNew<vtkCellArray> crossCells;
    const std::array<std::array<double, 3>, 6> crossVertices{ { { -0.5, 0.0, 0.0 },
                                                                 { 0.5, 0.0, 0.0 },
                                                                 { 0.0, -0.5, 0.0 },
                                                                 { 0.0, 0.5, 0.0 },
                                                                 { 0.0, 0.0, -0.5 },
                                                                 { 0.0, 0.0, 0.5 } } };
    for (const auto &v : crossVertices) {
        crossPoints->InsertNextPoint(v[0], v[1], v[2]);
    }
    const std::array<std::array<vtkIdType, 2>, 3> crossLines{ { { 0, 1 }, { 2, 3 }, { 4, 5 } } };
    for (const auto &line : crossLines) {
        crossCells->InsertNextCell(2);
        crossCells->InsertCellPoint(line[0]);
        crossCells->InsertCellPoint(line[1]);
    }
    vtkNew<vtkPolyData> crossData;
    crossData->SetPoints(crossPoints);
    crossData->SetLines(crossCells);
    this->crossSource->RemoveAllInputs();
    this->crossSource->AddInputData(crossData);

    glyphs->SetInputData(sourcesPolyData);
    glyphs->SetSourceConnection(sphereSource->GetOutputPort());
    glyphs->SetScaleModeToScaleByVectorComponents();
    glyphs->SetVectorModeToUseVector();
    glyphs->OrientOff();
    glyphs->SetColorModeToColorByScalar();  // pass morphology index to mapper
    glyphs->SetScaleFactor(1.0);

    const int nMorph = morphologyNames.size();
    glyphMapper->SetInputConnection(glyphs->GetOutputPort());
    glyphMapper->SetLookupTable(morphologyLut);
    glyphMapper->SetScalarRange(-0.5, nMorph - 0.5);
    glyphMapper->SetColorModeToMapScalars();
    glyphMapper->ScalarVisibilityOn();

    glyphActor->SetMapper(glyphMapper);
    renderer->AddActor(glyphActor);
    this->pointsMapper->SetInputData(sourcesPolyData);
    this->pointsMapper->SetLookupTable(this->morphologyLut);
    this->pointsMapper->SetScalarRange(-0.5, nMorph - 0.5);
    this->pointsMapper->ScalarVisibilityOn();
    this->pointsActor->SetMapper(this->pointsMapper);
    this->pointsActor->GetProperty()->SetPointSize(4.0);
    this->pointsActor->GetProperty()->SetColor(0.78, 0.82, 0.95);
    this->pointsActor->VisibilityOff();
    renderer->AddActor(this->pointsActor);

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
    cubeAxesActor->SetVisibility(this->chkShowBoundingBox && this->chkShowBoundingBox->isChecked() ? 1 : 0);

    this->buildShells();
    this->updatePointPositions();
    this->updateColorMapping();
    this->updateGlyphPipeline();

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

void vtkWindowCatalogue3D::buildShells()
{
    for (const auto &actor : this->shellActors) {
        this->renderer->RemoveActor(actor);
    }
    this->shellActors.clear();
    if (entries.empty()) {
        return;
    }

    const std::array<double, 3> shellFractions{ { 0.33, 0.66, 1.0 } };
    for (double fraction : shellFractions) {
        vtkNew<vtkSphereSource> shellSource;
        shellSource->SetRadius(std::max(1.0, this->maxDistanceMpc * fraction));
        shellSource->SetPhiResolution(48);
        shellSource->SetThetaResolution(48);
        vtkNew<vtkPolyDataMapper> shellMapper;
        shellMapper->SetInputConnection(shellSource->GetOutputPort());
        auto shellActor = vtkSmartPointer<vtkActor>::New();
        shellActor->SetMapper(shellMapper);
        shellActor->GetProperty()->SetRepresentationToWireframe();
        shellActor->GetProperty()->SetColor(0.42, 0.5, 0.66);
        shellActor->GetProperty()->SetOpacity(0.2);
        shellActor->VisibilityOff();
        this->renderer->AddActor(shellActor);
        this->shellActors.push_back(shellActor);
    }
}

QStringList vtkWindowCatalogue3D::numericRawFieldNames() const
{
    QStringList items;
    for (const int index : this->numericRawFieldIndices) {
        if (index >= 0 && index < this->rawHeaders.size()) {
            items.append(this->rawHeaders.at(index));
        }
    }
    return items;
}

bool vtkWindowCatalogue3D::rawFieldValue(const Catalogue3DEntry &entry, int fieldIndex,
                                         double &value) const
{
    if (fieldIndex < 0 || fieldIndex >= entry.rawFieldValues.size()) {
        return false;
    }
    return Catalogue3DParser::detail::maybeNumeric(entry.rawFieldValues.at(fieldIndex), value);
}

double vtkWindowCatalogue3D::mappedAxisValue(const Catalogue3DEntry &entry, const QString &mappingKey) const
{
    if (mappingKey == QStringLiteral("computed:x")) {
        return entry.sceneX;
    }
    if (mappingKey == QStringLiteral("computed:y")) {
        return entry.sceneY;
    }
    if (mappingKey == QStringLiteral("computed:z")) {
        return entry.sceneZ;
    }
    if (mappingKey.startsWith(QStringLiteral("raw:"))) {
        bool okIndex = false;
        const int index = mappingKey.mid(4).toInt(&okIndex);
        double value = 0.0;
        if (okIndex && this->rawFieldValue(entry, index, value)) {
            return value;
        }
    }
    return 0.0;
}

double vtkWindowCatalogue3D::mappedSizeValue(const Catalogue3DEntry &entry, bool &ok) const
{
    ok = true;
    if (this->mapSizeKey == QStringLiteral("builtin:fixed")) {
        return 1.0;
    }
    if (this->mapSizeKey == QStringLiteral("builtin:major")) {
        if (entry.majorAxisArcmin > 0.0) {
            return entry.majorAxisArcmin;
        }
        ok = false;
        return 1.0;
    }
    if (this->mapSizeKey == QStringLiteral("builtin:lls")) {
        if (entry.llsMajorKpc > 0.0) {
            return entry.llsMajorKpc;
        }
        ok = false;
        return 1.0;
    }
    if (this->mapSizeKey == QStringLiteral("builtin:flux")) {
        if (entry.fluxMJy > 0.0) {
            return entry.fluxMJy;
        }
        ok = false;
        return 1.0;
    }
    if (this->mapSizeKey.startsWith(QStringLiteral("raw:"))) {
        bool okIndex = false;
        const int index = this->mapSizeKey.mid(4).toInt(&okIndex);
        double value = 0.0;
        if (okIndex && this->rawFieldValue(entry, index, value) && value > 0.0) {
            return value;
        }
        ok = false;
        return 1.0;
    }
    ok = false;
    return 1.0;
}

double vtkWindowCatalogue3D::mappedColorValue(const Catalogue3DEntry &entry, bool &ok) const
{
    ok = true;
    if (this->mapColorKey.startsWith(QStringLiteral("raw:"))) {
        bool okIndex = false;
        const int index = this->mapColorKey.mid(4).toInt(&okIndex);
        double value = 0.0;
        if (okIndex && this->rawFieldValue(entry, index, value)) {
            return value;
        }
        ok = false;
        return 0.0;
    }
    ok = false;
    return 0.0;
}

QString vtkWindowCatalogue3D::fallbackDetailsText(const Catalogue3DEntry &entry) const
{
    QString text;
    for (int i = 0; i < this->rawHeaders.size() && i < entry.rawFieldValues.size(); ++i) {
        const QString value = entry.rawFieldValues.at(i).trimmed();
        if (value.isEmpty()) {
            continue;
        }
        text += QStringLiteral("%1: %2\n").arg(this->rawHeaders.at(i), value);
    }
    return text.trimmed();
}

void vtkWindowCatalogue3D::updatePointPositions()
{
    if (this->entries.empty()) {
        return;
    }

    const auto validatedAxisKey = [this](const QString &requestedKey, const QString &fallbackKey) {
        if (!requestedKey.startsWith(QStringLiteral("raw:"))) {
            return requestedKey;
        }
        int validCount = 0;
        double minValue = std::numeric_limits<double>::max();
        double maxValue = std::numeric_limits<double>::lowest();
        for (const auto &entry : this->entries) {
            const double value = this->mappedAxisValue(entry, requestedKey);
            if (!std::isfinite(value)) {
                continue;
            }
            ++validCount;
            minValue = std::min(minValue, value);
            maxValue = std::max(maxValue, value);
        }
        if (validCount < 3 || !(minValue < maxValue)) {
            qWarning() << "[catalogue3d] invalid axis mapping" << requestedKey
                       << "-> fallback to" << fallbackKey;
            return fallbackKey;
        }
        return requestedKey;
    };

    const QString resolvedXKey = validatedAxisKey(this->mapXKey, QStringLiteral("computed:x"));
    const QString resolvedYKey = validatedAxisKey(this->mapYKey, QStringLiteral("computed:y"));
    const QString resolvedZKey = validatedAxisKey(this->mapZKey, QStringLiteral("computed:z"));

    double xMin = std::numeric_limits<double>::max();
    double yMin = xMin;
    double zMin = xMin;
    double xMax = std::numeric_limits<double>::lowest();
    double yMax = xMax;
    double zMax = xMax;

    for (vtkIdType i = 0; i < static_cast<vtkIdType>(this->entries.size()); ++i) {
        const auto &entry = this->entries[static_cast<std::size_t>(i)];
        const double x = this->mappedAxisValue(entry, resolvedXKey);
        const double y = this->mappedAxisValue(entry, resolvedYKey);
        const double z = this->mappedAxisValue(entry, resolvedZKey);
        this->mappedPositions[static_cast<std::size_t>(i)] = { x, y, z };
        this->sourcePoints->SetPoint(i, x, y, z);
        xMin = std::min(xMin, x); xMax = std::max(xMax, x);
        yMin = std::min(yMin, y); yMax = std::max(yMax, y);
        zMin = std::min(zMin, z); zMax = std::max(zMax, z);
        if (i < static_cast<vtkIdType>(this->labelActors.size())) {
            this->labelActors[static_cast<std::size_t>(i)]->SetPosition(x, y, z);
        }
    }
    this->sourcePoints->Modified();
    this->sourcesPolyData->Modified();
    const double bounds[6] = { xMin, xMax, yMin, yMax, zMin, zMax };
    this->cubeAxesActor->SetBounds(bounds);
    this->renderer->ResetCameraClippingRange();
    this->updateHighlightSphere(this->hoverSphere.Get(), this->hoverActor.Get(), this->hoveredIndex, 1.35);
    this->updateHighlightSphere(this->selectSphere.Get(), this->selectActor.Get(), this->selectedIndex, 1.18);
    qInfo() << "[catalogue3d] axis mapping updated"
            << "X=" << this->mapXKey << "resolved=" << resolvedXKey
            << "Y=" << this->mapYKey << "resolved=" << resolvedYKey
            << "Z=" << this->mapZKey << "resolved=" << resolvedZKey;
}

void vtkWindowCatalogue3D::updateColorMapping()
{
    if (this->entries.empty()) {
        return;
    }

    if (this->mapColorKey == QStringLiteral("builtin:morphology")) {
        for (vtkIdType i = 0; i < static_cast<vtkIdType>(this->entries.size()); ++i) {
            const auto &entry = this->entries[static_cast<std::size_t>(i)];
            this->morphScalars->SetValue(i, this->morphologyIndexOf.value(entry.morphology, 0));
        }
        this->morphScalars->Modified();
        this->sourcesPolyData->GetPointData()->SetScalars(this->morphScalars);
        this->glyphMapper->SetLookupTable(this->morphologyLut);
        this->glyphMapper->SetScalarRange(-0.5, this->morphologyNames.size() - 0.5);
        this->pointsMapper->SetLookupTable(this->morphologyLut);
        this->pointsMapper->SetScalarRange(-0.5, this->morphologyNames.size() - 0.5);
        qInfo() << "[catalogue3d] color mapping: morphology";
        return;
    }

    double minValue = std::numeric_limits<double>::max();
    double maxValue = std::numeric_limits<double>::lowest();
    bool anyValid = false;
    for (vtkIdType i = 0; i < static_cast<vtkIdType>(this->entries.size()); ++i) {
        bool ok = false;
        const double value = this->mappedColorValue(this->entries[static_cast<std::size_t>(i)], ok);
        if (!ok || !std::isfinite(value)) {
            this->morphScalars->SetValue(i, 0.0);
            continue;
        }
        this->morphScalars->SetValue(i, value);
        minValue = std::min(minValue, value);
        maxValue = std::max(maxValue, value);
        anyValid = true;
    }

    if (!anyValid || !(minValue < maxValue)) {
        qWarning() << "[catalogue3d] invalid color mapping" << this->mapColorKey
                   << "-> fallback to morphology";
        this->mapColorKey = QStringLiteral("builtin:morphology");
        if (this->comboMapColor) {
            QSignalBlocker blocker(this->comboMapColor);
            this->comboMapColor->setCurrentIndex(0);
        }
        this->updateColorMapping();
        return;
    }

    this->morphScalars->Modified();
    this->sourcesPolyData->GetPointData()->SetScalars(this->morphScalars);
    this->valueLut->SetTableRange(minValue, maxValue);
    this->valueLut->Build();
    this->glyphMapper->SetLookupTable(this->valueLut);
    this->glyphMapper->SetScalarRange(minValue, maxValue);
    this->pointsMapper->SetLookupTable(this->valueLut);
    this->pointsMapper->SetScalarRange(minValue, maxValue);
    qInfo() << "[catalogue3d] color mapping:" << this->mapColorKey
            << "range:" << minValue << maxValue;
}

void vtkWindowCatalogue3D::updateGlyphPipeline()
{
    if (entries.empty()) {
        return;
    }

    this->pointsActor->VisibilityOff();
    this->glyphActor->VisibilityOn();
    switch (this->geometryMode) {
    case GeometryMode::Ellipsoid:
    case GeometryMode::Sphere:
        this->glyphs->SetSourceConnection(this->sphereSource->GetOutputPort());
        break;
    case GeometryMode::Cross:
        this->glyphs->SetSourceConnection(this->crossSource->GetOutputPort());
        break;
    case GeometryMode::Point:
        this->glyphActor->VisibilityOff();
        this->pointsActor->VisibilityOn();
        break;
    }

    this->updateGlyphScales();
    this->renderWindow->Render();
}

void vtkWindowCatalogue3D::updateGlyphScales()
{
    if (entries.empty()) {
        return;
    }

    sizeNormalizationValue = 1.0;
    bool anyValid = false;
    for (const auto &entry : entries) {
        bool ok = false;
        const double value = this->mappedSizeValue(entry, ok);
        if (!ok || !std::isfinite(value) || value <= 0.0) {
            continue;
        }
        sizeNormalizationValue = std::max(sizeNormalizationValue, value);
        anyValid = true;
    }
    if (!anyValid) {
        qWarning() << "[catalogue3d] invalid size mapping" << this->mapSizeKey
                   << "-> fallback to fixed size";
        this->mapSizeKey = QStringLiteral("builtin:fixed");
        if (this->comboMapSize) {
            QSignalBlocker blocker(this->comboMapSize);
            this->comboMapSize->setCurrentIndex(0);
        }
        this->sizeMode = SizeMode::Fixed;
        sizeNormalizationValue = 1.0;
    }

    const double sliderScale = ui->sliderScale->value() / 100.0;
    double minVisual = std::numeric_limits<double>::max();
    double maxVisual = 0.0;
    for (vtkIdType i = 0; i < static_cast<vtkIdType>(entries.size()); ++i) {
        const Catalogue3DEntry &entry = entries[static_cast<std::size_t>(i)];
        bool ok = false;
        const double sourceValue = this->mappedSizeValue(entry, ok);
        const double normalized = ok && sourceValue > 0.0
                ? std::clamp(sourceValue / sizeNormalizationValue, 0.15, 1.6)
                : 0.35;
        double major = defaultGlyphRadius * sliderScale * normalized;
        double minor = major;
        double depth = major;

        if (this->geometryMode == GeometryMode::Ellipsoid) {
            if (this->mapSizeKey == QStringLiteral("builtin:major") && entry.majorAxisArcmin > 0.0) {
                major = defaultGlyphRadius * sliderScale * std::max(0.15, entry.majorAxisArcmin / sizeNormalizationValue);
                const double minorNorm = entry.minorAxisArcmin > 0.0
                        ? std::max(0.12, entry.minorAxisArcmin / sizeNormalizationValue)
                        : std::max(0.12, entry.majorAxisArcmin / sizeNormalizationValue * 0.65);
                minor = defaultGlyphRadius * sliderScale * minorNorm;
                depth = minor;
            } else if (this->mapSizeKey == QStringLiteral("builtin:lls") && entry.llsMajorKpc > 0.0) {
                major = defaultGlyphRadius * sliderScale * std::max(0.15, entry.llsMajorKpc / sizeNormalizationValue);
                const double minorNorm = entry.llsMinorKpc > 0.0
                        ? std::max(0.12, entry.llsMinorKpc / sizeNormalizationValue)
                        : std::max(0.12, entry.llsMajorKpc / sizeNormalizationValue * 0.7);
                minor = defaultGlyphRadius * sliderScale * minorNorm;
                depth = minor;
            }
        }

        if (this->geometryMode == GeometryMode::Sphere || this->geometryMode == GeometryMode::Point
            || this->geometryMode == GeometryMode::Cross) {
            major = minor = depth = defaultGlyphRadius * sliderScale * normalized;
        }

        this->glyphScaleVectors->SetTuple3(i, minor, major, depth);
        minVisual = std::min(minVisual, std::min({ minor, major, depth }));
        maxVisual = std::max(maxVisual, std::max({ minor, major, depth }));
    }
    this->glyphScaleVectors->Modified();
    this->glyphs->Modified();
    this->pointsActor->GetProperty()->SetPointSize(std::clamp(4.0 * (ui->sliderScale->value() / 100.0), 3.0, 12.0));

    updateHighlightSphere(hoverSphere.Get(), hoverActor.Get(), hoveredIndex, 1.45);
    updateHighlightSphere(selectSphere.Get(), selectActor.Get(), selectedIndex, 1.28);
    qInfo() << "[catalogue3d] size mapping:" << this->mapSizeKey
            << "visual range:" << minVisual << maxVisual;
}

// ─────────────────────────────────────────────────────────────────────────────
// Interaction
// ─────────────────────────────────────────────────────────────────────────────

void vtkWindowCatalogue3D::onMouseEvent(unsigned long eid)
{
    int x = 0, y = 0;
    ui->vtk->interactor()->GetEventPosition(x, y);

    if (eid == vtkCommand::LeftButtonDoubleClickEvent) {
        const int idx = entries.empty() ? -1 : pickNearestSource(x, y);
        qInfo() << "[catalogue3d] double-click observer fired, picked source:" << idx;
        if (idx >= 0) {
            setSelectedSource(idx);
            centerOnSource(idx);
        }
        return;
    }

    if (eid == vtkCommand::LeftButtonPressEvent) {
        leftButtonDown = true;
        pressX = x;
        pressY = y;
        qInfo() << "[catalogue3d] left press observed; camera remains owned by VTK style";
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
            qInfo() << "[catalogue3d] click selection hit index:" << idx;
            setSelectedSource(idx == selectedIndex ? -1 : idx);
        } else if (dx * dx + dy * dy <= clickDragThresholdPx * clickDragThresholdPx) {
            qInfo() << "[catalogue3d] click selection miss on empty scene";
            setSelectedSource(-1);
        }
        return;
    }

    // MouseMoveEvent: no hover activation/highlighting. Camera drag remains owned by VTK.
    if (leftButtonDown) {
        qInfo() << "[catalogue3d] drag mouse move observed -> camera interaction path";
    }
}

int vtkWindowCatalogue3D::pickNearestSource(int displayX, int displayY) const
{
    vtkNew<vtkCoordinate> coord;
    coord->SetCoordinateSystemToWorld();

    double bestDist2 = hoverThresholdPx * hoverThresholdPx;
    int bestIdx = -1;

    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
        const auto &position = this->mappedPositions[static_cast<std::size_t>(i)];
        coord->SetValue(position[0], position[1], position[2]);
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
    const auto &position = this->mappedPositions[static_cast<std::size_t>(idx)];
    double tuple[3] = { defaultGlyphRadius, defaultGlyphRadius, defaultGlyphRadius };
    this->glyphScaleVectors->GetTuple(idx, tuple);
    const double r = std::max({ tuple[0], tuple[1], tuple[2] }) * radiusFactor;
    sphere->SetCenter(position[0], position[1], position[2]);
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
    if (hoveredIndex >= 0) {
        hoveredIndex = -1;
        hoverActor->VisibilityOff();
    }
    updateHighlightSphere(selectSphere.Get(), selectActor.Get(), idx, 1.18);
    updateInfoPanel();
    updateDockSelectionDetails();
    qInfo() << "[catalogue3d] selected source index:" << idx;
    emit this->sourceSelectionChanged(idx);
    renderWindow->Render();
}

// ─────────────────────────────────────────────────────────────────────────────
// Info panel
// ─────────────────────────────────────────────────────────────────────────────

void vtkWindowCatalogue3D::updateInfoPanel()
{
    if (selectedIndex < 0) {
        ui->labelInfo->setText(u"No source selected"_s);
        qInfo() << "[catalogue3d] details panel updated from selection: none";
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
    if (e.llsMajorKpc > 0.0) {
        if (e.llsMinorKpc > 0.0)
            html += QStringLiteral("<br>LLS: %1 × %2 kpc")
                            .arg(e.llsMinorKpc, 0, 'f', 1)
                            .arg(e.llsMajorKpc, 0, 'f', 1);
        else
            html += QStringLiteral("<br>LLS: %1 kpc").arg(e.llsMajorKpc, 0, 'f', 1);
    }
    if (e.fluxMJy > 0.0)
        html += QStringLiteral("<br>Flux: %1 mJy").arg(e.fluxMJy, 0, 'f', 2);
    html += QStringLiteral(
            "<br><br><span style='color:#b0b8c8'>3D depth comes from RA/Dec + redshift or distance. Source shape is parametric.</span>");

    ui->labelInfo->setText(html);
    qInfo() << "[catalogue3d] details panel updated from selection:" << selectedIndex;
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
    pointsMapper->SetScalarVisibility(checked ? 1 : 0);
    if (!checked)
        glyphActor->GetProperty()->SetColor(0.75, 0.75, 0.85);
    if (!checked)
        pointsActor->GetProperty()->SetColor(0.75, 0.75, 0.85);
    if (checked) {
        this->updateColorMapping();
    }
    if (!entries.empty())
        renderWindow->Render();
}

void vtkWindowCatalogue3D::scaleChanged(int value)
{
    if (entries.empty())
        return;
    Q_UNUSED(value);
    this->updateGlyphScales();
    renderWindow->Render();
}

void vtkWindowCatalogue3D::resetCamera()
{
    renderer->ResetCamera();
    renderWindow->Render();
}

void vtkWindowCatalogue3D::updateGeometryMode()
{
    switch (this->comboGeometry ? this->comboGeometry->currentIndex() : 0) {
    case 1:
        this->geometryMode = GeometryMode::Sphere;
        break;
    case 2:
        this->geometryMode = GeometryMode::Point;
        break;
    case 3:
        this->geometryMode = GeometryMode::Cross;
        break;
    default:
        this->geometryMode = GeometryMode::Ellipsoid;
        break;
    }
    this->updateGlyphPipeline();
}

void vtkWindowCatalogue3D::updateSizeMode()
{
    this->mapSizeKey = this->comboSizeMode ? this->comboSizeMode->currentData().toString()
                                           : QStringLiteral("builtin:fixed");
    if (this->mapSizeKey == QStringLiteral("builtin:major")) {
        this->sizeMode = SizeMode::MajorAxis;
    } else if (this->mapSizeKey == QStringLiteral("builtin:lls")) {
        this->sizeMode = SizeMode::Lls;
    } else if (this->mapSizeKey == QStringLiteral("builtin:flux")) {
        this->sizeMode = SizeMode::Flux;
    } else {
        this->sizeMode = SizeMode::Fixed;
    }
    this->updateGlyphScales();
    this->renderWindow->Render();
}

void vtkWindowCatalogue3D::toggleSceneAxes(bool checked)
{
    this->axesWidget->SetEnabled(checked ? 1 : 0);
    this->renderWindow->Render();
}

void vtkWindowCatalogue3D::toggleBoundingBox(bool checked)
{
    this->cubeAxesActor->SetVisibility(checked ? 1 : 0);
    this->renderWindow->Render();
}

void vtkWindowCatalogue3D::toggleShells(bool checked)
{
    for (const auto &actor : this->shellActors) {
        actor->SetVisibility(checked ? 1 : 0);
    }
    this->renderWindow->Render();
}

void vtkWindowCatalogue3D::ensureCatalogueDock()
{
    if (this->catalogueDock) {
        return;
    }
    this->catalogueDock = new QDockWidget(u"Catalogue"_s, this);
    this->catalogueDock->setObjectName(u"Catalogue3DDock"_s);
    auto *dockWidget = new QWidget(this->catalogueDock);
    auto *dockLayout = new QVBoxLayout(dockWidget);
    dockLayout->setContentsMargins(6, 6, 6, 6);
    dockLayout->setSpacing(6);

    auto *mappingGroup = new QGroupBox(u"Mappings"_s, dockWidget);
    auto *mappingLayout = new QFormLayout(mappingGroup);
    mappingLayout->setContentsMargins(6, 6, 6, 6);
    this->comboMapX = new QComboBox(mappingGroup);
    this->comboMapY = new QComboBox(mappingGroup);
    this->comboMapZ = new QComboBox(mappingGroup);
    this->comboMapSize = this->comboSizeMode;
    this->comboMapColor = new QComboBox(mappingGroup);
    mappingLayout->addRow(u"X"_s, this->comboMapX);
    mappingLayout->addRow(u"Y"_s, this->comboMapY);
    mappingLayout->addRow(u"Z"_s, this->comboMapZ);
    mappingLayout->addRow(u"Size"_s, this->comboMapSize);
    mappingLayout->addRow(u"Color"_s, this->comboMapColor);
    dockLayout->addWidget(mappingGroup);

    this->catalogueTableView = new QTableView(dockWidget);
    this->catalogueTableModel = new Catalogue3DTableModel(this->catalogueDock);
    this->catalogueTableView->setModel(this->catalogueTableModel);
    this->catalogueTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->catalogueTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    this->catalogueTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->catalogueTableView->setAlternatingRowColors(true);
    this->catalogueTableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->catalogueTableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->catalogueTableView->verticalHeader()->setVisible(false);
    this->catalogueTableView->horizontalHeader()->setStretchLastSection(false);
    this->catalogueTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    dockLayout->addWidget(this->catalogueTableView, 1);

    this->catalogueDetailsView = new QTextEdit(dockWidget);
    this->catalogueDetailsView->setReadOnly(true);
    this->catalogueDetailsView->setPlaceholderText(u"No source selected"_s);
    this->catalogueDetailsView->setMaximumHeight(140);
    dockLayout->addWidget(this->catalogueDetailsView);

    this->catalogueDock->setWidget(dockWidget);
    this->addDockWidget(Qt::BottomDockWidgetArea, this->catalogueDock);
    QObject::connect(this, &vtkWindowCatalogue3D::sourceSelectionChanged, this,
                     &vtkWindowCatalogue3D::syncTableSelection);
    QObject::connect(this->catalogueTableView->selectionModel(), &QItemSelectionModel::currentRowChanged,
                     this, [this](const QModelIndex &current) {
        if (this->syncingTableSelection) {
            return;
        }
        this->setSelectedSource(current.isValid() ? current.row() : -1);
    });
    QObject::connect(this->catalogueTableView, &QTableView::doubleClicked, this,
                     [this](const QModelIndex &index) {
        if (index.isValid()) {
            this->setSelectedSource(index.row());
            this->centerOnSource(index.row());
        }
    });
    QObject::connect(this->comboMapX, &QComboBox::currentIndexChanged, this, [this](int) {
        if (!this->comboMapX || this->updatingMappingControls) {
            return;
        }
        this->mapXKey = this->comboMapX->currentData().toString();
        this->updatePointPositions();
        this->renderWindow->Render();
    });
    QObject::connect(this->comboMapY, &QComboBox::currentIndexChanged, this, [this](int) {
        if (!this->comboMapY || this->updatingMappingControls) {
            return;
        }
        this->mapYKey = this->comboMapY->currentData().toString();
        this->updatePointPositions();
        this->renderWindow->Render();
    });
    QObject::connect(this->comboMapZ, &QComboBox::currentIndexChanged, this, [this](int) {
        if (!this->comboMapZ || this->updatingMappingControls) {
            return;
        }
        this->mapZKey = this->comboMapZ->currentData().toString();
        this->updatePointPositions();
        this->renderWindow->Render();
    });
    QObject::connect(this->comboMapColor, &QComboBox::currentIndexChanged, this, [this](int) {
        if (!this->comboMapColor || this->updatingMappingControls) {
            return;
        }
        this->mapColorKey = this->comboMapColor->currentData().toString();
        this->updateColorMapping();
        this->renderWindow->Render();
    });
}

void vtkWindowCatalogue3D::refreshCatalogueTable()
{
    if (!this->catalogueTableModel) {
        return;
    }
    this->catalogueTableModel->setCatalogue(&this->entries, &this->rawHeaders);
    if (this->catalogueTableView) {
        this->catalogueTableView->resizeColumnsToContents();
    }
    this->updateDockSelectionDetails();
}

void vtkWindowCatalogue3D::populateMappingControls()
{
    if (!this->comboMapX || !this->comboMapY || !this->comboMapZ || !this->comboMapSize
        || !this->comboMapColor) {
        return;
    }

    this->numericRawFieldIndices.clear();
    for (int column = 0; column < this->rawHeaders.size(); ++column) {
        int validCount = 0;
        int nonEmptyCount = 0;
        for (const auto &entry : this->entries) {
            if (column >= entry.rawFieldValues.size()) {
                continue;
            }
            const QString raw = entry.rawFieldValues.at(column).trimmed();
            if (raw.isEmpty()) {
                continue;
            }
            ++nonEmptyCount;
            double value = 0.0;
            if (Catalogue3DParser::detail::maybeNumeric(raw, value)) {
                ++validCount;
            }
        }
        if (validCount >= 3 && validCount * 5 >= std::max(1, nonEmptyCount) * 4) {
            this->numericRawFieldIndices.push_back(column);
        }
    }

    this->updatingMappingControls = true;
    const auto fillAxisCombo = [this](QComboBox *combo, const QString &currentKey, const QString &defaultKey) {
        combo->clear();
        combo->addItem(u"Computed Cartesian X"_s, QStringLiteral("computed:x"));
        combo->addItem(u"Computed Cartesian Y"_s, QStringLiteral("computed:y"));
        combo->addItem(u"Computed Cartesian Z"_s, QStringLiteral("computed:z"));
        for (const int index : this->numericRawFieldIndices) {
            combo->addItem(QStringLiteral("raw:%1").arg(this->rawHeaders.value(index)),
                           QStringLiteral("raw:%1").arg(index));
        }
        int comboIndex = combo->findData(currentKey);
        if (comboIndex < 0) {
            comboIndex = combo->findData(defaultKey);
        }
        combo->setCurrentIndex(std::max(0, comboIndex));
    };
    fillAxisCombo(this->comboMapX, this->mapXKey, QStringLiteral("computed:x"));
    fillAxisCombo(this->comboMapY, this->mapYKey, QStringLiteral("computed:y"));
    fillAxisCombo(this->comboMapZ, this->mapZKey, QStringLiteral("computed:z"));

    this->comboMapSize->clear();
    this->comboMapSize->addItem(u"Fixed"_s, QStringLiteral("builtin:fixed"));
    this->comboMapSize->addItem(u"Major axis"_s, QStringLiteral("builtin:major"));
    this->comboMapSize->addItem(u"LLS"_s, QStringLiteral("builtin:lls"));
    this->comboMapSize->addItem(u"Flux"_s, QStringLiteral("builtin:flux"));
    for (const int index : this->numericRawFieldIndices) {
        this->comboMapSize->addItem(QStringLiteral("raw:%1").arg(this->rawHeaders.value(index)),
                                    QStringLiteral("raw:%1").arg(index));
    }
    int sizeIndex = this->comboMapSize->findData(this->mapSizeKey);
    this->comboMapSize->setCurrentIndex(sizeIndex >= 0 ? sizeIndex : 0);

    this->comboMapColor->clear();
    this->comboMapColor->addItem(u"Morphology color"_s, QStringLiteral("builtin:morphology"));
    for (const int index : this->numericRawFieldIndices) {
        this->comboMapColor->addItem(QStringLiteral("raw:%1").arg(this->rawHeaders.value(index)),
                                     QStringLiteral("raw:%1").arg(index));
    }
    int colorIndex = this->comboMapColor->findData(this->mapColorKey);
    this->comboMapColor->setCurrentIndex(colorIndex >= 0 ? colorIndex : 0);
    this->updatingMappingControls = false;
}

void vtkWindowCatalogue3D::updateDockSelectionDetails()
{
    if (!this->catalogueDetailsView) {
        return;
    }
    if (this->selectedIndex < 0 || this->selectedIndex >= static_cast<int>(this->entries.size())) {
        this->catalogueDetailsView->clear();
        qInfo() << "[catalogue3d] catalogue details view cleared";
        return;
    }
    const auto &entry = this->entries[static_cast<std::size_t>(this->selectedIndex)];
    QString details;
    details += QStringLiteral("Name: %1\n").arg(entry.name);
    details += QStringLiteral("RA: %1\n").arg(entry.raDeg, 0, 'f', 6);
    details += QStringLiteral("Dec: %1\n").arg(entry.decDeg, 0, 'f', 6);
    details += QStringLiteral("Distance: %1\n").arg(entry.distanceMpc, 0, 'f', 3);
    details += QStringLiteral("Morphology: %1\n").arg(entry.morphology);
    const QString rawDetails = this->fallbackDetailsText(entry);
    if (!rawDetails.isEmpty()) {
        details += QStringLiteral("\nRaw fields\n%1").arg(rawDetails);
    }
    this->catalogueDetailsView->setPlainText(details);
    qInfo() << "[catalogue3d] catalogue details view updated for selection:" << this->selectedIndex;
}

void vtkWindowCatalogue3D::syncTableSelection(int index)
{
    if (!this->catalogueTableView || !this->catalogueTableView->selectionModel() || !this->catalogueTableModel) {
        return;
    }
    this->syncingTableSelection = true;
    if (index < 0 || index >= this->catalogueTableModel->rowCount()) {
        this->catalogueTableView->clearSelection();
    } else {
        const QModelIndex modelIndex = this->catalogueTableModel->index(index, 0);
        this->catalogueTableView->selectionModel()->setCurrentIndex(
                modelIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        this->catalogueTableView->scrollTo(modelIndex, QAbstractItemView::PositionAtCenter);
    }
    this->syncingTableSelection = false;
}

void vtkWindowCatalogue3D::centerOnSource(int idx, double zoomFactor)
{
    if (idx < 0 || idx >= static_cast<int>(this->entries.size())) {
        return;
    }
    const auto &targetPosition = this->mappedPositions[static_cast<std::size_t>(idx)];
    auto *camera = this->renderer->GetActiveCamera();
    if (!camera) {
        return;
    }
    double focal[3];
    double cameraPosition[3];
    camera->GetFocalPoint(focal);
    camera->GetPosition(cameraPosition);
    const double dx = cameraPosition[0] - focal[0];
    const double dy = cameraPosition[1] - focal[1];
    const double dz = cameraPosition[2] - focal[2];
    camera->SetFocalPoint(targetPosition[0], targetPosition[1], targetPosition[2]);
    camera->SetPosition(targetPosition[0] + dx / zoomFactor,
                        targetPosition[1] + dy / zoomFactor,
                        targetPosition[2] + dz / zoomFactor);
    this->renderer->ResetCameraClippingRange();
    this->renderWindow->Render();
}
