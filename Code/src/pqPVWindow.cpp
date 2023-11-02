#include "pqPVWindow.h"
#include "ui_pqPVWindow.h"

#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "vtkSMCoreUtilities.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMTransferFunctionManager.h"
#include "vtkSMTransferFunctionPresets.h"

pqPVWindow::pqPVWindow(pqServer *serv, pqPipelineSource *cbSrc, std::pair<int, int> &start, std::pair<int, int> &end, QWidget *parent) :
      QMainWindow(parent),
      ui(new Ui::pqPVWindow)
{
    ui->setupUi(this);

    this->builder = pqApplicationCore::instance()->getObjectBuilder();
    this->server = serv;
    this->cubeSource = cbSrc;

    viewImage = qobject_cast<pqRenderView *>(builder->createView(pqRenderView::renderViewType(), server));
    viewImage->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->centralwidget->layout()->addWidget(viewImage->widget());

    auto presets = vtkSMTransferFunctionPresets::GetInstance();
    for (int i = 0; i < presets->GetNumberOfPresets(); ++i) {
        QString name = QString::fromStdString(presets->GetPresetName(i));
        ui->comboLut->addItem(name);
    }

    this->PVSliceFilter = builder->createFilter("filters", "PVSliceFilter", this->cubeSource);
    auto filterProxy = this->PVSliceFilter->getProxy();
    if (auto startProp = filterProxy->GetProperty("StartPoint")){
        int* startVals = new int[2];
        startVals[0] = start.first;
        startVals[1] = start.second;
        vtkSMPropertyHelper(startProp).Set(startVals, 2);
        filterProxy->UpdateVTKObjects();
        delete[] startVals;
    }
    else{
        std::cerr << "Error when setting filter properties!" << std::endl;
    }

    if (auto endProp = filterProxy->GetProperty("StartPoint")){
        int* endVals = new int[2];
        endVals[0] = end.first;
        endVals[1] = end.second;
        vtkSMPropertyHelper(endProp).Set(endVals, 2);
        filterProxy->UpdateVTKObjects();
        delete[] endVals;
    }
    else{
        std::cerr << "Error when setting filter properties!" << std::endl;

    }
    filterProxy->UpdateVTKObjects();
    PVSliceFilter->updatePipeline();

    this->imageProxy = builder->createDataRepresentation(this->PVSliceFilter->getOutputPort(0), viewImage)->getProxy();
    vtkSMPropertyHelper(imageProxy, "Representation").Set("Slice");
    vtkSMPVRepresentationProxy::SetScalarColoring(imageProxy, "FITSImage", vtkDataObject::POINT);
    imageProxy->UpdateVTKObjects();

    vtkNew<vtkSMTransferFunctionManager> mgr;
    lutProxy = vtkSMTransferFunctionProxy::SafeDownCast(mgr->GetColorTransferFunction("PVSliceTransferFunction", imageProxy->GetSessionProxyManager()));

    connect(ui->radioLinear, &QRadioButton::toggled, this, &pqPVWindow::changeLutScale);
    connect(ui->opacitySlider, &QSlider::valueChanged, this, &pqPVWindow::changeOpacity);
    connect(ui->comboLut, &QComboBox::currentTextChanged, this, &pqPVWindow::changeLut);

    viewImage->resetDisplay();
    viewImage->render();
}

pqPVWindow::~pqPVWindow()
{
    delete ui;
}

void pqPVWindow::update(std::pair<int, int> &start, std::pair<int, int> &end)
{
    auto filterProxy = this->PVSliceFilter->getProxy();
    if (auto startProp = filterProxy->GetProperty("StartPoint")){
        int* startVals = new int[2];
        startVals[0] = start.first;
        startVals[1] = start.second;
        vtkSMPropertyHelper(startProp).Set(startVals, 2);
        filterProxy->UpdateVTKObjects();
        delete[] startVals;
    }
    else{
        std::cerr << "Error when setting filter properties!" << std::endl;
    }

    if (auto endProp = filterProxy->GetProperty("StartPoint")){
        int* endVals = new int[2];
        endVals[0] = end.first;
        endVals[1] = end.second;
        vtkSMPropertyHelper(endProp).Set(endVals, 2);
        filterProxy->UpdateVTKObjects();
        delete[] endVals;
    }
    else{
        std::cerr << "Error when setting filter properties!" << std::endl;

    }
    filterProxy->UpdateVTKObjects();
    PVSliceFilter->updatePipeline();
}

QString pqPVWindow::getColourMap() const
{
    return colourMap;
}

int pqPVWindow::changeLutScale()
{
    logScale = !logScale;
    if (logScale) {
        if (auto logProperty = lutProxy->GetProperty("UseLogScale"))
        {
            double range[2];
            vtkSMTransferFunctionProxy::GetRange(lutProxy, range);
            vtkSMCoreUtilities::AdjustRangeForLog(range);

            vtkSMTransferFunctionProxy::RescaleTransferFunction(lutProxy, range);
            vtkSMPropertyHelper(logProperty).Set(1);

            lutProxy->UpdateVTKObjects();
            imageProxy->UpdateVTKObjects();
            changeLut(this->getColourMap());
            viewImage->render();
            return 1;
        }
        else
        {
            std::cerr << "Error with logscale proxy: not found correctly." << std::endl;
            return 0;
        }
    } else {
        if (auto logProperty = lutProxy->GetProperty("UseLogScale"))
        {
            this->logScale = false;
            vtkSMTransferFunctionProxy::RescaleTransferFunctionToDataRange(lutProxy);
            vtkSMPropertyHelper(logProperty).Set(0);

            lutProxy->UpdateVTKObjects();
            imageProxy->UpdateVTKObjects();
            changeLut(this->getColourMap());
            viewImage->render();
            return 1;
        }
        else
        {
            std::cerr << "Error with logscale proxy: not found correctly." << std::endl;
            return 0;
        }
    }
}

int pqPVWindow::changeOpacity(int opacity)
{
    if (vtkSMProperty *opacityProperty = imageProxy->GetProperty("Opacity")) {

        vtkSMPropertyHelper(opacityProperty).Set(opacity * 0.01);
        imageProxy->UpdateVTKObjects();

        viewImage->render();
        return 1;
    }
    return 0;
}

int pqPVWindow::changeLut(const QString &lutName)
{
    if (vtkSMProperty *lutProperty = imageProxy->GetProperty("LookupTable")) {
        auto presets = vtkSMTransferFunctionPresets::GetInstance();
        lutProxy->ApplyPreset(presets->GetFirstPresetWithName(lutName.toStdString().c_str()));
        vtkSMPropertyHelper(lutProperty).Set(lutProxy);
        lutProxy->UpdateVTKObjects();
        vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRange(imageProxy, false, false);

        vtkSMPVRepresentationProxy::SetScalarBarVisibility(imageProxy, viewImage->getProxy(), true);

        imageProxy->UpdateVTKObjects();
        this->colourMap = lutName;
        viewImage->render();
        return 1;
    }
    return 0;
}
