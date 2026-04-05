#include "vtkWindowVbtVolume.h"

#include <QVTKOpenGLNativeWidget.h>

#include <vtkAxesActor.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPointData.h>
#include <vtkRenderWindowInteractor.h>

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

vtkWindowVbtVolume::vtkWindowVbtVolume(const VbtTableData &table, QWidget *parent)
    : QMainWindow(parent)
    , table(table)
{
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
    this->volumeProperty->ShadeOff();
    this->volumeProperty->SetInterpolationTypeToLinear();
    this->volumeProperty->SetColor(this->colorTransfer.Get());
    this->volumeProperty->SetScalarOpacity(this->opacityTransfer.Get());
    this->volumeActor->SetMapper(this->volumeMapper.Get());
    this->volumeActor->SetProperty(this->volumeProperty.Get());
    this->renderer->AddVolume(this->volumeActor.Get());
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

    vtkNew<vtkImageData> image;
    image->SetDimensions(this->table.header.dimensions[0], this->table.header.dimensions[1],
                         this->table.header.dimensions[2]);
    image->SetSpacing(this->table.header.spacing[0], this->table.header.spacing[1],
                      this->table.header.spacing[2]);
    image->SetOrigin(0.0, 0.0, 0.0);
    image->AllocateScalars(VTK_FLOAT, 1);

    auto *scalars = static_cast<float *>(image->GetScalarPointer());
    const char *base = this->table.rawBinary.constData();
    const int fieldCount = this->table.header.fieldCount;
    double minValue = std::numeric_limits<double>::max();
    double maxValue = std::numeric_limits<double>::lowest();

    for (std::size_t voxel = 0; voxel < voxelCount; ++voxel) {
        const char *ptr = base + (static_cast<qint64>(voxel) * fieldCount + fieldIndex) * scalarSize;
        const double value = this->table.header.scalarType == VbtScalarType::Float64
                ? maybeSwapValue<double>(ptr, swapBytes)
                : maybeSwapValue<float>(ptr, swapBytes);
        scalars[voxel] = static_cast<float>(value);
        minValue = std::min(minValue, value);
        maxValue = std::max(maxValue, value);
    }

    if (!(minValue < maxValue)) {
        maxValue = minValue + 1.0;
    }

    this->colorTransfer->RemoveAllPoints();
    this->colorTransfer->AddRGBPoint(minValue, 0.0, 0.0, 0.0);
    this->colorTransfer->AddRGBPoint(minValue + (maxValue - minValue) * 0.25, 0.0, 0.2, 0.8);
    this->colorTransfer->AddRGBPoint(minValue + (maxValue - minValue) * 0.6, 0.9, 0.6, 0.1);
    this->colorTransfer->AddRGBPoint(maxValue, 1.0, 1.0, 1.0);

    const double opacityScale = (this->sliderOpacity ? this->sliderOpacity->value() : 35) / 100.0;
    this->opacityTransfer->RemoveAllPoints();
    this->opacityTransfer->AddPoint(minValue, 0.0);
    this->opacityTransfer->AddPoint(minValue + (maxValue - minValue) * 0.2, 0.02 * opacityScale);
    this->opacityTransfer->AddPoint(minValue + (maxValue - minValue) * 0.6, 0.15 * opacityScale);
    this->opacityTransfer->AddPoint(maxValue, 0.45 * opacityScale);

    this->volumeMapper->SetInputData(image.Get());
    this->renderer->ResetCamera();
    this->renderer->ResetCameraClippingRange();
    this->renderWindow->Render();
    this->updateSummary();
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
