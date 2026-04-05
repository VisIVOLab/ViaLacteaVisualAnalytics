#include "vtkWindowVbtVolume.h"

#include <QVTKOpenGLNativeWidget.h>

#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCubeAxesActor.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPointData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkScalarBarActor.h>
#include <vtkTextProperty.h>

#include <QComboBox>
#include <QDockWidget>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <cmath>
#include <QSysInfo>
#include <array>
#include <cstring>
#include <limits>

using namespace Qt::StringLiterals;

namespace {

template <typename T>
T maybeSwapValue(const char *ptr, bool swapBytes)
{
    std::array<unsigned char, sizeof(T)> bytes{};
    std::memcpy(bytes.data(), ptr, sizeof(T));
    if (swapBytes) {
        std::reverse(bytes.begin(), bytes.end());
    }
    T value{};
    std::memcpy(&value, bytes.data(), sizeof(T));
    return value;
}

}

double vtkWindowVbtVolume::percentileFromSortedSample(const std::vector<double> &values, double fraction)
{
    if (values.empty()) {
        return 0.0;
    }
    const double clamped = std::clamp(fraction, 0.0, 1.0);
    const double scaled = clamped * static_cast<double>(values.size() - 1);
    const std::size_t low = static_cast<std::size_t>(std::floor(scaled));
    const std::size_t high = static_cast<std::size_t>(std::ceil(scaled));
    if (low == high) {
        return values[low];
    }
    const double t = scaled - static_cast<double>(low);
    return values[low] * (1.0 - t) + values[high] * t;
}

vtkWindowVbtVolume::vtkWindowVbtVolume(const VbtTableData &table, QWidget *parent)
    : QMainWindow(parent)
    , table(table)
{
    qDebug().noquote()
            << QStringLiteral("[vbt-volume] header type=%1 fields=%2 dims=%3x%4x%5 spacing=%6,%7,%8 selected_field=%9")
                       .arg(VbtTableLoader::scalarTypeName(this->table.header.scalarType))
                       .arg(this->table.header.fieldCount)
                       .arg(this->table.header.dimensions[0])
                       .arg(this->table.header.dimensions[1])
                       .arg(this->table.header.dimensions[2])
                       .arg(this->table.header.spacing[0], 0, 'g', 6)
                       .arg(this->table.header.spacing[1], 0, 'g', 6)
                       .arg(this->table.header.spacing[2], 0, 'g', 6)
                       .arg(this->table.header.fieldNames.isEmpty() ? QStringLiteral("<none>")
                                                                    : this->table.header.fieldNames.first());
    this->setupUi();
    this->setupRenderer();
    this->rebuildVolumeData();
    this->updateSummary();
}

void vtkWindowVbtVolume::setupUi()
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowTitle(QStringLiteral("VBT Volume Viewer — %1").arg(this->table.header.headerPath));
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

    auto *displayGroup = new QGroupBox(u"Volume"_s, panel);
    auto *displayLayout = new QFormLayout(displayGroup);
    this->comboScalarField = new QComboBox(displayGroup);
    for (int field = 0; field < this->table.header.fieldNames.size(); ++field) {
        this->comboScalarField->addItem(this->table.header.fieldNames.at(field), field);
    }
    this->sliderOpacity = new QSlider(Qt::Horizontal, displayGroup);
    this->sliderOpacity->setRange(1, 100);
    this->sliderOpacity->setValue(35);
    displayLayout->addRow(u"Scalar"_s, this->comboScalarField);
    displayLayout->addRow(u"Opacity"_s, this->sliderOpacity);
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

    QObject::connect(this->comboScalarField, &QComboBox::currentIndexChanged, this,
                     [this](int) { this->rebuildVolumeData(); });
    QObject::connect(this->sliderOpacity, &QSlider::valueChanged, this,
                     [this](int) { this->rebuildVolumeData(); });
}

void vtkWindowVbtVolume::setupRenderer()
{
    this->vtkWidget->setRenderWindow(this->renderWindow.Get());
    this->vtkWidget->setEnableTouchEventProcessing(false);
    this->renderer->SetBackground(0.04, 0.04, 0.08);
    this->renderWindow->AddRenderer(this->renderer);

    auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    this->vtkWidget->interactor()->SetInteractorStyle(style);

    vtkNew<vtkAxesActor> axesActor;
    this->axesWidget->SetOrientationMarker(axesActor);
    this->axesWidget->SetInteractor(this->vtkWidget->interactor());
    this->axesWidget->SetViewport(0.0, 0.0, 0.14, 0.14);
    this->axesWidget->EnabledOn();
    this->axesWidget->InteractiveOff();

    this->volumeMapper->SetBlendModeToComposite();
    this->volumeMapper->SetRequestedRenderModeToGPU();
    this->volumeProperty->ShadeOff();
    this->volumeProperty->SetInterpolationTypeToLinear();
    this->volumeProperty->SetColor(this->colorTransfer.Get());
    this->volumeProperty->SetScalarOpacity(this->opacityTransfer.Get());
    this->volumeActor->SetMapper(this->volumeMapper.Get());
    this->volumeActor->SetProperty(this->volumeProperty.Get());
    this->renderer->AddVolume(this->volumeActor.Get());

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
    this->boxActor->SetVisibility(1);
    this->renderer->AddActor(this->boxActor.Get());

    this->scalarBar->SetLookupTable(this->colorTransfer.Get());
    this->scalarBar->SetMaximumWidthInPixels(100);
    this->scalarBar->SetPosition(0.88, 0.1);
    this->scalarBar->GetTitleTextProperty()->SetColor(1.0, 1.0, 1.0);
    this->scalarBar->GetLabelTextProperty()->SetColor(1.0, 1.0, 1.0);
    this->scalarBar->SetVisibility(1);
    this->renderer->AddViewProp(this->scalarBar.Get());

    qDebug().noquote()
            << QStringLiteral("[vbt-volume] renderer setup mapper=%1 volume_actor=1 box_actor=1 scalar_bar=1 props=%2")
                       .arg(this->volumeMapper->GetClassName())
                       .arg(this->renderer->GetViewProps()->GetNumberOfItems());
}

void vtkWindowVbtVolume::rebuildVolumeData()
{
    const int fieldIndex = this->comboScalarField ? this->comboScalarField->currentData().toInt() : 0;
    const std::size_t voxelCount = static_cast<std::size_t>(this->table.header.rowCount);
    const qsizetype scalarSize =
            this->table.header.scalarType == VbtScalarType::Float64 ? sizeof(double) : sizeof(float);
    const bool hostLittleEndian = QSysInfo::ByteOrder == QSysInfo::LittleEndian;
    const bool fileLittleEndian = this->table.header.endian == VbtEndian::Little;
    const bool swapBytes = hostLittleEndian != fileLittleEndian;

    this->currentImageData = vtkSmartPointer<vtkImageData>::New();
    this->currentImageData->SetDimensions(this->table.header.dimensions[0],
                                          this->table.header.dimensions[1],
                                          this->table.header.dimensions[2]);
    this->currentImageData->SetSpacing(this->table.header.spacing[0], this->table.header.spacing[1],
                                       this->table.header.spacing[2]);
    this->currentImageData->SetOrigin(0.0, 0.0, 0.0);
    this->currentImageData->AllocateScalars(VTK_FLOAT, 1);

    auto *scalars = static_cast<float *>(this->currentImageData->GetScalarPointer());
    const char *base = this->table.rawBinary.constData();
    const int fieldCount = this->table.header.fieldCount;
    double minValue = std::numeric_limits<double>::max();
    double maxValue = std::numeric_limits<double>::lowest();
    constexpr std::size_t targetSampleCount = 200000;
    const std::size_t sampleStride = std::max<std::size_t>(1, voxelCount / targetSampleCount);
    std::vector<double> sampledValues;
    sampledValues.reserve(std::min<std::size_t>(voxelCount, targetSampleCount));

    for (std::size_t voxel = 0; voxel < voxelCount; ++voxel) {
        const char *ptr = base + (static_cast<qint64>(voxel) * fieldCount + fieldIndex) * scalarSize;
        const double value = this->table.header.scalarType == VbtScalarType::Float64
                ? maybeSwapValue<double>(ptr, swapBytes)
                : maybeSwapValue<float>(ptr, swapBytes);
        scalars[voxel] = static_cast<float>(value);
        minValue = std::min(minValue, value);
        maxValue = std::max(maxValue, value);
        if ((voxel % sampleStride) == 0 && std::isfinite(value)) {
            sampledValues.push_back(value);
        }
    }

    if (!(minValue < maxValue)) {
        maxValue = minValue + 1.0;
    }

    std::sort(sampledValues.begin(), sampledValues.end());
    double displayMin = minValue;
    double displayMax = maxValue;
    if (sampledValues.size() >= 16) {
        displayMin = percentileFromSortedSample(sampledValues, 0.005);
        displayMax = percentileFromSortedSample(sampledValues, 0.995);
    }
    if (!(displayMin < displayMax) || !std::isfinite(displayMin) || !std::isfinite(displayMax)) {
        displayMin = minValue;
        displayMax = maxValue;
    }
    if (!(displayMin < displayMax)) {
        displayMax = displayMin + 1.0;
    }

    const double range = displayMax - displayMin;
    const double p10 = displayMin + range * 0.10;
    const double p30 = displayMin + range * 0.30;
    const double p60 = displayMin + range * 0.60;

    this->colorTransfer->RemoveAllPoints();
    this->colorTransfer->AddRGBPoint(displayMin, 0.02, 0.02, 0.04);
    this->colorTransfer->AddRGBPoint(p30, 0.05, 0.20, 0.65);
    this->colorTransfer->AddRGBPoint(p60, 0.90, 0.55, 0.10);
    this->colorTransfer->AddRGBPoint(displayMax, 1.00, 0.98, 0.92);

    const double opacityScale = (this->sliderOpacity ? this->sliderOpacity->value() : 35) / 100.0;
    this->opacityTransfer->RemoveAllPoints();
    this->opacityTransfer->AddPoint(displayMin, 0.0);
    this->opacityTransfer->AddPoint(p10, 0.01 * opacityScale);
    this->opacityTransfer->AddPoint(p30, 0.05 * opacityScale);
    this->opacityTransfer->AddPoint(p60, 0.2 * opacityScale);
    this->opacityTransfer->AddPoint(displayMax, 0.8 * opacityScale);

    this->volumeProperty->SetScalarOpacityUnitDistance(
            std::max(1e-6, (this->table.header.spacing[0] + this->table.header.spacing[1]
                            + this->table.header.spacing[2])
                                   / 3.0));

    this->volumeMapper->SetInputData(this->currentImageData);
    this->updateBoundsContext();
    this->updateScalarBar();
    this->resetView();

    double imageRange[2]{ 0.0, 0.0 };
    this->currentImageData->GetScalarRange(imageRange);
    double bounds[6]{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    this->currentImageData->GetBounds(bounds);
    qDebug().noquote()
            << QStringLiteral("[vbt-volume] image dims=%1x%2x%3 spacing=%4,%5,%6 origin=%7,%8,%9 scalar_type=%10 image_range=%11..%12 bounds=[%13,%14]x[%15,%16]x[%17,%18]")
                       .arg(this->table.header.dimensions[0])
                       .arg(this->table.header.dimensions[1])
                       .arg(this->table.header.dimensions[2])
                       .arg(this->table.header.spacing[0], 0, 'g', 6)
                       .arg(this->table.header.spacing[1], 0, 'g', 6)
                       .arg(this->table.header.spacing[2], 0, 'g', 6)
                       .arg(0.0, 0, 'g', 3)
                       .arg(0.0, 0, 'g', 3)
                       .arg(0.0, 0, 'g', 3)
                       .arg(this->currentImageData->GetScalarTypeAsString())
                       .arg(imageRange[0], 0, 'g', 8)
                       .arg(imageRange[1], 0, 'g', 8)
                       .arg(bounds[0], 0, 'g', 8)
                       .arg(bounds[1], 0, 'g', 8)
                       .arg(bounds[2], 0, 'g', 8)
                       .arg(bounds[3], 0, 'g', 8)
                       .arg(bounds[4], 0, 'g', 8)
                       .arg(bounds[5], 0, 'g', 8);
    qDebug().noquote()
            << QStringLiteral("[vbt-volume] mapper=%1 input_dims=%2x%3x%4 property=1 tf_range=%5..%6 opacity_points=%7 color_points=%8")
                       .arg(this->volumeMapper->GetClassName())
                       .arg(this->table.header.dimensions[0])
                       .arg(this->table.header.dimensions[1])
                       .arg(this->table.header.dimensions[2])
                       .arg(displayMin, 0, 'g', 8)
                       .arg(displayMax, 0, 'g', 8)
                       .arg(this->opacityTransfer->GetSize())
                       .arg(this->colorTransfer->GetSize());

    auto *props = this->renderer->GetViewProps();
    const double *visibleBounds = this->renderer->ComputeVisiblePropBounds();
    const double *position = this->renderer->GetActiveCamera()->GetPosition();
    const double *focalPoint = this->renderer->GetActiveCamera()->GetFocalPoint();
    double clippingRange[2]{ 0.0, 0.0 };
    this->renderer->GetActiveCamera()->GetClippingRange(clippingRange);
    qDebug().noquote()
            << QStringLiteral("[vbt-volume] renderer volume_actor=1 box_actor=%1 scalar_bar=%2 props=%3 visible_bounds=[%4,%5]x[%6,%7]x[%8,%9] camera_pos=%10,%11,%12 focal=%13,%14,%15 clip=%16,%17")
                       .arg(this->boxActor->GetVisibility())
                       .arg(this->scalarBar->GetVisibility())
                       .arg(props ? props->GetNumberOfItems() : 0)
                       .arg(visibleBounds ? visibleBounds[0] : 0.0, 0, 'g', 8)
                       .arg(visibleBounds ? visibleBounds[1] : 0.0, 0, 'g', 8)
                       .arg(visibleBounds ? visibleBounds[2] : 0.0, 0, 'g', 8)
                       .arg(visibleBounds ? visibleBounds[3] : 0.0, 0, 'g', 8)
                       .arg(visibleBounds ? visibleBounds[4] : 0.0, 0, 'g', 8)
                       .arg(visibleBounds ? visibleBounds[5] : 0.0, 0, 'g', 8)
                       .arg(position[0], 0, 'g', 8)
                       .arg(position[1], 0, 'g', 8)
                       .arg(position[2], 0, 'g', 8)
                       .arg(focalPoint[0], 0, 'g', 8)
                       .arg(focalPoint[1], 0, 'g', 8)
                       .arg(focalPoint[2], 0, 'g', 8)
                       .arg(clippingRange[0], 0, 'g', 8)
                       .arg(clippingRange[1], 0, 'g', 8);

    if (!visibleBounds || !(visibleBounds[0] < visibleBounds[1]) || !(visibleBounds[2] < visibleBounds[3])
        || !(visibleBounds[4] < visibleBounds[5])) {
        qDebug().noquote() << QStringLiteral("[vbt-volume] visible bounds invalid, applying known-good fallback");
        this->applyKnownGoodVisibleFallback(minValue, maxValue, bounds);
    }

    this->renderWindow->Render();
    this->updateSummary();
}

void vtkWindowVbtVolume::updateBoundsContext()
{
    if (!this->currentImageData) {
        return;
    }
    double bounds[6]{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    this->currentImageData->GetBounds(bounds);
    this->boxActor->SetBounds(bounds);
}

void vtkWindowVbtVolume::updateScalarBar()
{
    const QString activeField = this->comboScalarField ? this->comboScalarField->currentText() : QString();
    this->scalarBar->SetTitle(activeField.toStdString().c_str());
    this->scalarBar->SetVisibility(this->comboScalarField && this->comboScalarField->count() > 0 ? 1 : 0);
}

void vtkWindowVbtVolume::resetView()
{
    this->renderer->ResetCamera();
    this->renderer->ResetCameraClippingRange();
}

void vtkWindowVbtVolume::applyKnownGoodVisibleFallback(double minValue, double maxValue,
                                                       const double bounds[6])
{
    if (!(minValue < maxValue)) {
        maxValue = minValue + 1.0;
    }
    const double range = maxValue - minValue;
    this->colorTransfer->RemoveAllPoints();
    this->colorTransfer->AddRGBPoint(minValue, 0.0, 0.0, 0.0);
    this->colorTransfer->AddRGBPoint(minValue + range * 0.5, 0.8, 0.4, 0.1);
    this->colorTransfer->AddRGBPoint(maxValue, 1.0, 1.0, 1.0);

    this->opacityTransfer->RemoveAllPoints();
    this->opacityTransfer->AddPoint(minValue, 0.0);
    this->opacityTransfer->AddPoint(minValue + range * 0.1, 0.05);
    this->opacityTransfer->AddPoint(minValue + range * 0.4, 0.2);
    this->opacityTransfer->AddPoint(maxValue, 0.9);

    this->boxActor->SetVisibility(1);
    this->scalarBar->SetVisibility(1);
    this->boxActor->SetBounds(bounds);
    this->renderer->ResetCamera(bounds);
    this->renderer->ResetCameraClippingRange(bounds);
}

void vtkWindowVbtVolume::updateSummary()
{
    const QString activeField = this->comboScalarField ? this->comboScalarField->currentText() : QString();
    if (this->summaryLabel) {
        this->summaryLabel->setText(
                QStringLiteral("Dimensions: %1 x %2 x %3\nSpacing: %4, %5, %6\nFields: %7\nScalar type: %8\nActive scalar: %9")
                        .arg(this->table.header.dimensions[0])
                        .arg(this->table.header.dimensions[1])
                        .arg(this->table.header.dimensions[2])
                        .arg(this->table.header.spacing[0])
                        .arg(this->table.header.spacing[1])
                        .arg(this->table.header.spacing[2])
                        .arg(this->table.header.fieldCount)
                        .arg(VbtTableLoader::scalarTypeName(this->table.header.scalarType))
                        .arg(activeField));
    }
    if (this->metadataLabel) {
        this->metadataLabel->setText(
                QStringLiteral("Header: %1\nBinary: %2\nEndian: %3\nVoxel count: %4")
                        .arg(this->table.header.headerPath)
                        .arg(this->table.header.binaryPath)
                        .arg(VbtTableLoader::endianName(this->table.header.endian))
                        .arg(this->table.header.rowCount));
    }
}
