#include "vtkWindowVbt.h"

#include <QVTKOpenGLNativeWidget.h>

#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkCellArray.h>
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
    this->sliderPointSize = new QSlider(Qt::Horizontal, displayGroup);
    this->sliderPointSize->setRange(1, 12);
    this->sliderPointSize->setValue(3);
    displayLayout->addRow(u"Color"_s, this->comboColorField);
    displayLayout->addRow(u"Point size"_s, this->sliderPointSize);
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

    QObject::connect(this->comboColorField, &QComboBox::currentIndexChanged, this,
                     [this](int) { this->updateColorMapping(); });
    QObject::connect(this->sliderPointSize, &QSlider::valueChanged, this, [this](int value) {
        if (this->actor) {
            this->actor->GetProperty()->SetPointSize(value);
            this->renderWindow->Render();
            this->updateSummary();
        }
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
    this->renderer->ResetCamera();
    this->renderWindow->Render();
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

void vtkWindowVbt::updateColorMapping()
{
    if (!this->comboColorField) {
        return;
    }

    const QVariant data = this->comboColorField->currentData();
    if (!data.isValid() || data.toString().isEmpty()) {
        this->mapper->ScalarVisibilityOff();
        this->actor->GetProperty()->SetColor(0.8, 0.85, 1.0);
        this->renderWindow->Render();
        this->updateSummary();
        return;
    }

    bool okIndex = false;
    const int fieldIndex = data.toInt(&okIndex);
    if (!okIndex || fieldIndex < 0 || fieldIndex >= static_cast<int>(this->table.columns.size())) {
        this->mapper->ScalarVisibilityOff();
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
    this->pointScalars->Modified();
    this->polyData->GetPointData()->SetScalars(this->pointScalars.Get());
    this->scalarLut->SetTableRange(minValue, maxValue);
    this->scalarLut->Build();
    this->mapper->SetLookupTable(this->scalarLut.Get());
    this->mapper->SetScalarRange(minValue, maxValue);
    this->mapper->ScalarVisibilityOn();
    this->renderWindow->Render();
    this->updateSummary();
}

void vtkWindowVbt::updateSummary()
{
    if (this->summaryLabel) {
        this->summaryLabel->setText(
                QStringLiteral("Rows: %1\nFields: %2\nScalar type: %3\nActive color: %4\nPoint size: %5")
                        .arg(this->table.header.rowCount)
                        .arg(this->table.header.fieldCount)
                        .arg(VbtTableLoader::scalarTypeName(this->table.header.scalarType))
                        .arg(this->activeColorFieldName())
                        .arg(this->sliderPointSize ? this->sliderPointSize->value() : 0));
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
