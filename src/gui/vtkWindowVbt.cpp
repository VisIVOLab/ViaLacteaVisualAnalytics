#include "vtkWindowVbt.h"

#include <QVTKOpenGLNativeWidget.h>

#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkCellArray.h>
#include <vtkCubeAxesActor.h>
#include <vtkDoubleArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkLookupTable.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkScalarBarActor.h>
#include <vtkTextProperty.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <limits>

using namespace Qt::StringLiterals;

vtkWindowVbt::vtkWindowVbt(const VbtTableData &table, QWidget *parent)
    : QMainWindow(parent)
    , table(table)
{
    this->setupUi();
    this->setupRenderer();
    this->buildPointCloud();
    this->updateColorMapping();
    this->updateSummary();
}

void vtkWindowVbt::setupUi()
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowTitle(QStringLiteral("VBT Point Viewer — %1").arg(this->table.header.headerPath));
    this->resize(1280, 800);

    auto *central = new QWidget(this);
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(6);

    this->vtkWidget = new QVTKOpenGLNativeWidget(central);
    this->vtkWidget->setMinimumSize(700, 500);
    layout->addWidget(this->vtkWidget, 1);

    auto *panel = new QWidget(central);
    panel->setMinimumWidth(260);
    panel->setMaximumWidth(300);
    auto *panelLayout = new QVBoxLayout(panel);

    auto *displayGroup = new QGroupBox(u"Display"_s, panel);
    auto *displayLayout = new QFormLayout(displayGroup);
    this->comboRenderMode = new QComboBox(displayGroup);
    this->comboRenderMode->addItems({ u"Points"_s, u"Particle Ray-Casting"_s });
    this->comboColorField = new QComboBox(displayGroup);
    this->comboColorField->addItem(u"Solid color"_s, QString());
    for (int field = 0; field < this->table.header.fieldNames.size(); ++field) {
        if (field == this->table.xIndex || field == this->table.yIndex || field == this->table.zIndex) {
            continue;
        }
        this->comboColorField->addItem(this->table.header.fieldNames.at(field), field);
    }
    if (this->comboColorField->count() > 1) {
        this->comboColorField->setCurrentIndex(1);
    }
    this->comboColormap = new QComboBox(displayGroup);
    for (const auto &name : ColorMaps::GetColorMapNames()) {
        this->comboColormap->addItem(QString::fromStdString(name));
    }
    this->comboColormap->setCurrentText(QString::fromStdString(ColorMaps::DefaultColorMap));
    this->comboBackground = new QComboBox(displayGroup);
    this->comboBackground->addItems({ u"Black"_s, u"Dark gray"_s, u"White"_s });
    this->sliderPointSize = new QSlider(Qt::Horizontal, displayGroup);
    this->sliderPointSize->setRange(1, 12);
    this->sliderPointSize->setValue(3);
    this->sliderRayIntensity = new QSlider(Qt::Horizontal, displayGroup);
    this->sliderRayIntensity->setRange(1, 100);
    this->sliderRayIntensity->setValue(35);
    this->checkShowBox = new QCheckBox(u"Show box"_s, displayGroup);
    this->checkShowBox->setChecked(true);
    this->checkShowLut = new QCheckBox(u"Show LUT"_s, displayGroup);
    this->checkShowLut->setChecked(true);
    this->checkShowOrientation = new QCheckBox(u"Show orientation"_s, displayGroup);
    this->checkShowOrientation->setChecked(true);
    this->spinRangeMin = new QDoubleSpinBox(displayGroup);
    this->spinRangeMax = new QDoubleSpinBox(displayGroup);
    this->spinRangeMin->setDecimals(6);
    this->spinRangeMax->setDecimals(6);
    this->spinRangeMin->setRange(-1e30, 1e30);
    this->spinRangeMax->setRange(-1e30, 1e30);
    this->buttonAutoscale = new QPushButton(u"Autoscale"_s, displayGroup);
    this->buttonResetView = new QPushButton(u"Reset View"_s, displayGroup);
    displayLayout->addRow(u"Mode"_s, this->comboRenderMode);
    displayLayout->addRow(u"Color"_s, this->comboColorField);
    displayLayout->addRow(u"Colormap"_s, this->comboColormap);
    displayLayout->addRow(u"Range min"_s, this->spinRangeMin);
    displayLayout->addRow(u"Range max"_s, this->spinRangeMax);
    displayLayout->addRow(u"Background"_s, this->comboBackground);
    displayLayout->addRow(u"Point size"_s, this->sliderPointSize);
    displayLayout->addRow(u"Ray intensity"_s, this->sliderRayIntensity);
    displayLayout->addRow(this->checkShowBox);
    displayLayout->addRow(this->checkShowLut);
    displayLayout->addRow(this->checkShowOrientation);
    displayLayout->addRow(this->buttonAutoscale);
    displayLayout->addRow(this->buttonResetView);
    panelLayout->addWidget(displayGroup);

    auto *summaryGroup = new QGroupBox(u"Summary"_s, panel);
    auto *summaryLayout = new QVBoxLayout(summaryGroup);
    this->summaryLabel = new QLabel(summaryGroup);
    this->summaryLabel->setWordWrap(true);
    summaryLayout->addWidget(this->summaryLabel);
    panelLayout->addWidget(summaryGroup);
    panelLayout->addStretch(1);
    layout->addWidget(panel);
    this->setCentralWidget(central);

    this->metadataDock = new QDockWidget(u"VBT Metadata"_s, this);
    this->metadataDock->setObjectName(u"VbtMetadataDock"_s);
    auto *dockWidget = new QWidget(this->metadataDock);
    auto *dockLayout = new QVBoxLayout(dockWidget);
    this->metadataLabel = new QLabel(dockWidget);
    this->metadataLabel->setWordWrap(true);
    this->fieldList = new QListWidget(dockWidget);
    for (const QString &fieldName : this->table.header.fieldNames) {
        this->fieldList->addItem(fieldName);
    }
    dockLayout->addWidget(this->metadataLabel);
    dockLayout->addWidget(this->fieldList, 1);
    this->metadataDock->setWidget(dockWidget);
    this->addDockWidget(Qt::BottomDockWidgetArea, this->metadataDock);

    QObject::connect(this->comboRenderMode, &QComboBox::currentIndexChanged, this,
                     [this](int) { this->updateRenderMode(); });
    QObject::connect(this->comboColorField, &QComboBox::currentIndexChanged, this,
                     [this](int) { this->updateColorMapping(); });
    QObject::connect(this->comboColormap, &QComboBox::currentIndexChanged, this,
                     [this](int) { this->updateColorMapping(); });
    QObject::connect(this->comboBackground, &QComboBox::currentIndexChanged, this,
                     [this](int) { this->applyBackground(); });
    QObject::connect(this->sliderPointSize, &QSlider::valueChanged, this, [this](int value) {
        if (this->actor) {
            this->actor->GetProperty()->SetPointSize(value);
            if (this->gaussianMapper) {
                this->gaussianMapper->SetScaleFactor(static_cast<double>(value));
            }
            this->renderWindow->Render();
            this->updateSummary();
        }
    });
    QObject::connect(this->sliderRayIntensity, &QSlider::valueChanged, this, [this](int) {
        this->updateRenderMode();
    });
    QObject::connect(this->checkShowBox, &QCheckBox::toggled, this, [this](bool checked) {
        this->boxActor->SetVisibility(checked ? 1 : 0);
        this->renderWindow->Render();
    });
    QObject::connect(this->checkShowLut, &QCheckBox::toggled, this, [this](bool) {
        this->updateScalarBar();
        this->renderWindow->Render();
    });
    QObject::connect(this->checkShowOrientation, &QCheckBox::toggled, this, [this](bool checked) {
        this->axesWidget->SetEnabled(checked ? 1 : 0);
        this->renderWindow->Render();
    });
    QObject::connect(this->buttonAutoscale, &QPushButton::clicked, this, [this]() {
        this->setScalarRange(this->dataScalarMin, this->dataScalarMax);
        this->updateColorMapping();
    });
    QObject::connect(this->buttonResetView, &QPushButton::clicked, this, [this]() {
        this->resetView();
    });
    QObject::connect(this->spinRangeMin, &QDoubleSpinBox::valueChanged, this, [this](double) {
        if (this->updatingRangeControls) {
            return;
        }
        this->setScalarRange(this->spinRangeMin->value(), this->spinRangeMax->value());
        this->updateColorMapping();
    });
    QObject::connect(this->spinRangeMax, &QDoubleSpinBox::valueChanged, this, [this](double) {
        if (this->updatingRangeControls) {
            return;
        }
        this->setScalarRange(this->spinRangeMin->value(), this->spinRangeMax->value());
        this->updateColorMapping();
    });
}

void vtkWindowVbt::setupRenderer()
{
    this->vtkWidget->setRenderWindow(this->renderWindow.Get());
    this->vtkWidget->setEnableTouchEventProcessing(false);
    this->renderer->SetBackground(0.06, 0.06, 0.1);
    this->renderWindow->AddRenderer(this->renderer);

    auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    this->vtkWidget->interactor()->SetInteractorStyle(style);

    vtkNew<vtkAxesActor> axesActor;
    this->axesWidget->SetOrientationMarker(axesActor);
    this->axesWidget->SetInteractor(this->vtkWidget->interactor());
    this->axesWidget->SetViewport(0.0, 0.0, 0.14, 0.14);
    this->axesWidget->EnabledOn();
    this->axesWidget->InteractiveOff();

    this->scalarBar->SetLookupTable(this->scalarLut.Get());
    this->scalarBar->SetMaximumWidthInPixels(100);
    this->scalarBar->SetPosition(0.88, 0.1);
    this->scalarBar->GetTitleTextProperty()->SetColor(1.0, 1.0, 1.0);
    this->scalarBar->GetLabelTextProperty()->SetColor(1.0, 1.0, 1.0);
    this->scalarBar->VisibilityOff();
    this->renderer->AddViewProp(this->scalarBar.Get());

    this->boxActor->SetCamera(this->renderer->GetActiveCamera());
    this->boxActor->SetXTitle("X");
    this->boxActor->SetYTitle("Y");
    this->boxActor->SetZTitle("Z");
    this->boxActor->SetFlyModeToOuterEdges();
    this->boxActor->DrawXGridlinesOff();
    this->boxActor->DrawYGridlinesOff();
    this->boxActor->DrawZGridlinesOff();
    for (int axis = 0; axis < 3; ++axis) {
        this->boxActor->GetTitleTextProperty(axis)->SetColor(0.85, 0.85, 0.85);
        this->boxActor->GetLabelTextProperty(axis)->SetColor(0.75, 0.75, 0.75);
    }
    this->renderer->AddActor(this->boxActor.Get());

    this->gaussianMapper->SetInputData(this->polyData.Get());
    this->gaussianMapper->SetScaleArray("VbtScalar");
    this->gaussianMapper->SetOpacityArray("VbtScalar");
    this->gaussianMapper->SetScaleFunction(this->gaussianScaleFunction.Get());
    this->gaussianMapper->SetScalarOpacityFunction(this->gaussianOpacityFunction.Get());
    this->gaussianMapper->SetScaleFactor(static_cast<double>(this->sliderPointSize->value()));
    this->gaussianMapper->SetEmissive(true);
    this->gaussianMapper->SetBoundScale(3.0f);
    this->gaussianMapper->SetLookupTable(this->scalarLut.Get());
}

void vtkWindowVbt::buildPointCloud()
{
    const std::size_t rowCount = static_cast<std::size_t>(this->table.header.rowCount);
    this->points->SetNumberOfPoints(static_cast<vtkIdType>(rowCount));
    this->pointScalars->SetName("VbtScalar");
    this->pointScalars->SetNumberOfTuples(static_cast<vtkIdType>(rowCount));

    vtkNew<vtkCellArray> verts;
    for (vtkIdType row = 0; row < static_cast<vtkIdType>(rowCount); ++row) {
        const double x = this->table.columns[static_cast<std::size_t>(this->table.xIndex)][static_cast<std::size_t>(row)];
        const double y = this->table.columns[static_cast<std::size_t>(this->table.yIndex)][static_cast<std::size_t>(row)];
        const double z = this->table.columns[static_cast<std::size_t>(this->table.zIndex)][static_cast<std::size_t>(row)];
        this->points->SetPoint(row, x, y, z);
        this->pointScalars->SetValue(row, 0.0);
        verts->InsertNextCell(1, &row);
    }

    this->polyData->SetPoints(this->points.Get());
    this->polyData->SetVerts(verts);
    this->polyData->GetPointData()->SetScalars(this->pointScalars.Get());

    this->scalarLut->SetNumberOfTableValues(256);
    this->scalarLut->SetHueRange(0.67, 0.0);
    this->scalarLut->SetSaturationRange(0.85, 0.95);
    this->scalarLut->SetValueRange(0.9, 1.0);
    this->scalarLut->Build();

    this->mapper->SetInputData(this->polyData.Get());
    this->actor->SetMapper(this->mapper.Get());
    this->actor->GetProperty()->SetRepresentationToPoints();
    this->actor->GetProperty()->SetPointSize(this->sliderPointSize->value());
    this->actor->GetProperty()->SetColor(0.8, 0.85, 1.0);
    this->renderer->AddActor(this->actor.Get());
    this->updateBoundsContext();
    this->updateRenderMode();
    this->resetView();
}

void vtkWindowVbt::updateRenderMode()
{
    if (!this->actor) {
        return;
    }
    const bool rayCasting = this->comboRenderMode && this->comboRenderMode->currentIndex() == 1;
    if (rayCasting) {
        this->gaussianMapper->SetScaleFactor(static_cast<double>(this->sliderPointSize ? this->sliderPointSize->value() : 3));
        const double intensity = (this->sliderRayIntensity ? this->sliderRayIntensity->value() : 35) / 100.0;
        this->actor->SetMapper(this->gaussianMapper.Get());
        this->actor->GetProperty()->SetOpacity(std::clamp(0.25 + intensity * 0.75, 0.05, 1.0));
    } else {
        this->actor->SetMapper(this->mapper.Get());
        this->actor->GetProperty()->SetOpacity(1.0);
        this->actor->GetProperty()->SetPointSize(this->sliderPointSize ? this->sliderPointSize->value() : 3);
    }
    this->renderWindow->Render();
    this->updateSummary();
}

QString vtkWindowVbt::activeColorFieldName() const
{
    if (!this->comboColorField) {
        return QStringLiteral("solid");
    }
    return this->comboColorField->currentIndex() <= 0
            ? QStringLiteral("solid")
            : this->comboColorField->currentText();
}

QString vtkWindowVbt::activeColormapName() const
{
    return this->comboColormap ? this->comboColormap->currentText()
                               : QString::fromStdString(ColorMaps::DefaultColorMap);
}

void vtkWindowVbt::setScalarRange(double minValue, double maxValue)
{
    if (!(minValue < maxValue)) {
        minValue = this->dataScalarMin;
        maxValue = this->dataScalarMax;
        if (!(minValue < maxValue)) {
            maxValue = minValue + 1.0;
        }
    }
    this->activeScalarMin = minValue;
    this->activeScalarMax = maxValue;
    this->updatingRangeControls = true;
    if (this->spinRangeMin) {
        this->spinRangeMin->setValue(minValue);
    }
    if (this->spinRangeMax) {
        this->spinRangeMax->setValue(maxValue);
    }
    this->updatingRangeControls = false;
}

void vtkWindowVbt::updateColorMapping()
{
    if (!this->comboColorField) {
        return;
    }

    const QVariant data = this->comboColorField->currentData();
    if (!data.isValid() || data.toString().isEmpty()) {
        this->mapper->ScalarVisibilityOff();
        this->gaussianMapper->ScalarVisibilityOff();
        this->actor->GetProperty()->SetColor(0.8, 0.85, 1.0);
        this->updateScalarBar();
        this->updateRenderMode();
        this->renderWindow->Render();
        this->updateSummary();
        return;
    }

    bool okIndex = false;
    const int fieldIndex = data.toInt(&okIndex);
    if (!okIndex || fieldIndex < 0 || fieldIndex >= static_cast<int>(this->table.columns.size())) {
        this->mapper->ScalarVisibilityOff();
        this->gaussianMapper->ScalarVisibilityOff();
        this->updateScalarBar();
        this->updateRenderMode();
        this->renderWindow->Render();
        this->updateSummary();
        return;
    }

    double minValue = std::numeric_limits<double>::max();
    double maxValue = std::numeric_limits<double>::lowest();
    const auto &column = this->table.columns[static_cast<std::size_t>(fieldIndex)];
    for (vtkIdType row = 0; row < static_cast<vtkIdType>(column.size()); ++row) {
        const double value = column[static_cast<std::size_t>(row)];
        this->pointScalars->SetValue(row, value);
        minValue = std::min(minValue, value);
        maxValue = std::max(maxValue, value);
    }
    if (!(minValue < maxValue)) {
        maxValue = minValue + 1.0;
    }
    this->dataScalarMin = minValue;
    this->dataScalarMax = maxValue;
    if (!(this->activeScalarMin < this->activeScalarMax)
        || this->activeScalarMin < minValue || this->activeScalarMax > maxValue) {
        this->setScalarRange(minValue, maxValue);
    }
    this->pointScalars->Modified();
    this->polyData->GetPointData()->SetScalars(this->pointScalars.Get());
    ColorMaps::SetColorMap(this->scalarLut.Get(), this->activeColormapName().toStdString());
    this->scalarLut->SetTableRange(this->activeScalarMin, this->activeScalarMax);
    this->scalarLut->Build();
    this->mapper->SetLookupTable(this->scalarLut.Get());
    this->mapper->SetScalarRange(this->activeScalarMin, this->activeScalarMax);
    this->mapper->ScalarVisibilityOn();
    this->gaussianMapper->SetLookupTable(this->scalarLut.Get());
    this->gaussianMapper->SetScalarRange(this->activeScalarMin, this->activeScalarMax);
    this->gaussianMapper->ScalarVisibilityOn();

    this->gaussianScaleFunction->RemoveAllPoints();
    this->gaussianOpacityFunction->RemoveAllPoints();
    const double range = this->activeScalarMax - this->activeScalarMin;
    const double p20 = this->activeScalarMin + range * 0.20;
    const double p60 = this->activeScalarMin + range * 0.60;
    const double p90 = this->activeScalarMin + range * 0.90;
    const double intensity = (this->sliderRayIntensity ? this->sliderRayIntensity->value() : 35) / 100.0;
    this->gaussianScaleFunction->AddPoint(this->activeScalarMin, 0.6);
    this->gaussianScaleFunction->AddPoint(p60, 1.0);
    this->gaussianScaleFunction->AddPoint(this->activeScalarMax, 1.6);
    this->gaussianOpacityFunction->AddPoint(this->activeScalarMin, 0.0);
    this->gaussianOpacityFunction->AddPoint(p20, 0.02 * intensity);
    this->gaussianOpacityFunction->AddPoint(p60, 0.10 * intensity);
    this->gaussianOpacityFunction->AddPoint(p90, 0.30 * intensity);
    this->gaussianOpacityFunction->AddPoint(this->activeScalarMax, 0.65 * intensity);

    this->updateScalarBar();
    this->updateRenderMode();
    this->renderWindow->Render();
    this->updateSummary();
}

void vtkWindowVbt::updateScalarBar()
{
    const bool visible = this->mapper->GetScalarVisibility()
            && this->checkShowLut && this->checkShowLut->isChecked();
    this->scalarBar->SetVisibility(visible ? 1 : 0);
    if (visible) {
        this->scalarBar->SetTitle(this->activeColorFieldName().toUtf8().constData());
        const bool darkBackground = !this->comboBackground || this->comboBackground->currentText() != u"White"_s;
        const double textColor = darkBackground ? 1.0 : 0.1;
        this->scalarBar->GetTitleTextProperty()->SetColor(textColor, textColor, textColor);
        this->scalarBar->GetLabelTextProperty()->SetColor(textColor, textColor, textColor);
    }
}

void vtkWindowVbt::updateBoundsContext()
{
    double bounds[6] = { 0., 0., 0., 0., 0., 0. };
    this->polyData->GetBounds(bounds);
    this->boxActor->SetBounds(bounds);
    this->boxActor->SetVisibility(this->checkShowBox && this->checkShowBox->isChecked() ? 1 : 0);
}

void vtkWindowVbt::applyBackground()
{
    if (!this->comboBackground) {
        return;
    }
    const QString background = this->comboBackground->currentText();
    if (background == u"White"_s) {
        this->renderer->SetBackground(0.98, 0.98, 0.98);
    } else if (background == u"Dark gray"_s) {
        this->renderer->SetBackground(0.16, 0.17, 0.19);
    } else {
        this->renderer->SetBackground(0.06, 0.06, 0.1);
    }
    this->updateScalarBar();
    this->renderWindow->Render();
}

void vtkWindowVbt::resetView()
{
    this->renderer->ResetCamera();
    this->renderer->ResetCameraClippingRange();
    this->renderWindow->Render();
}

void vtkWindowVbt::updateSummary()
{
    if (this->summaryLabel) {
        this->summaryLabel->setText(
                QStringLiteral("Rows: %1\nFields: %2\nScalar type: %3\nMode: %4\nActive color: %5\nColormap: %6\nRange: [%7, %8]\nPoint size: %9\nRay intensity: %10")
                        .arg(this->table.header.rowCount)
                        .arg(this->table.header.fieldCount)
                        .arg(VbtTableLoader::scalarTypeName(this->table.header.scalarType))
                        .arg(this->comboRenderMode ? this->comboRenderMode->currentText() : QStringLiteral("Points"))
                        .arg(this->activeColorFieldName())
                        .arg(this->activeColormapName())
                        .arg(this->activeScalarMin, 0, 'g', 6)
                        .arg(this->activeScalarMax, 0, 'g', 6)
                        .arg(this->sliderPointSize ? this->sliderPointSize->value() : 0)
                        .arg(this->sliderRayIntensity ? this->sliderRayIntensity->value() : 0));
    }
    if (this->metadataLabel) {
        this->metadataLabel->setText(
                QStringLiteral("Header: %1\nBinary: %2\nEndian: %3\nFields: %4\nRows: %5")
                        .arg(this->table.header.headerPath)
                        .arg(this->table.header.binaryPath)
                        .arg(VbtTableLoader::endianName(this->table.header.endian))
                        .arg(this->table.header.fieldCount)
                        .arg(this->table.header.rowCount));
    }
}
