#include "pqwindowimage.h"
#include "ui_pqwindowimage.h"

#include <pqActiveObjects.h>
#include <pqAlwaysConnectedBehavior.h>
#include <pqApplicationCore.h>
#include <pqLoadDataReaction.h>
#include <pqObjectBuilder.h>
#include <pqPipelineSource.h>
#include <pqRenderView.h>

#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMPVRepresentationProxy.h>
#include <vtkSMSessionProxyManager.h>
#include <vtkSMTransferFunctionPresets.h>
#include <vtkSMTransferFunctionProxy.h>
#include <vtkSMViewProxy.h>

#include <QFileInfo>

pqWindowImage::pqWindowImage(const QString &filepath, const CubeSubset &cubeSubset)
    : ui(new Ui::pqWindowImage),
      FitsFileName(QFileInfo(filepath).fileName()),
      cubeSubset(cubeSubset)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(FitsFileName);

    // Create LUTs menu actions
    auto presets = vtkSMTransferFunctionPresets::GetInstance();
    auto lutGroup = new QActionGroup(this);
    for (int i = 0; i < presets->GetNumberOfPresets(); ++i) {
        QString name = QString::fromStdString(presets->GetPresetName(i));
        auto lut = new QAction(name, lutGroup);
        lut->setCheckable(true);
        if (name == "Grayscale")
            lut->setChecked(true);

        lutGroup->addAction(lut);
        connect(lut, &QAction::triggered, this, [=]() { changeColorMap(name); });
    }
    ui->menuColorMap->addActions(lutGroup->actions());

    // ParaView Init
    builder = pqApplicationCore::instance()->getObjectBuilder();
    server = pqActiveObjects::instance().activeServer();
    serverProxyManager = server->proxyManager();
    new pqAlwaysConnectedBehavior(this);

    // Enable annotations such as Remote Rendering, FPS, etc...
    auto renderingSettings = serverProxyManager->GetProxy("settings", "RenderViewSettings");
    vtkSMPropertyHelper(renderingSettings, "ShowAnnotation").Set(1);

    vtkSMPropertyHelper(renderingSettings, "LODThreshold").Set(512);
    vtkSMPropertyHelper(renderingSettings, "LODResolution").Set(0);
    vtkSMPropertyHelper(renderingSettings, "UseOutlineForLODRendering").Set(1);
    vtkSMPropertyHelper(renderingSettings, "RemoteRenderThreshold").Set(512);
    vtkSMPropertyHelper(renderingSettings, "CompressorConfig").Set("vtkLZ4Compressor 0 5");

    // Load Reactions
    ImageSource = pqLoadDataReaction::loadData({ filepath });

    // Handle Subset selection
    setSubsetProperties(cubeSubset);

    createView();

    changeColorMap("Grayscale");

    // Interactor to show pixel coordinates in the status bar
    vtkNew<vtkInteractorStyleImageCustom> interactorStyle;
    interactorStyle->SetCoordsCallback(
            [this](const std::string &str) { showStatusBarMessage(str); });
    interactorStyle->SetLayerFitsReaderFunc(fitsHeaderPath.toStdString());
    interactorStyle->SetPixelZCompFunc([this]() { return currentSlice; });
    viewImage->getViewProxy()->GetRenderWindow()->GetInteractor()->SetInteractorStyle(
            interactorStyle);

    viewImage->resetDisplay();
    viewImage->render();
}

pqWindowImage::~pqWindowImage()
{
    builder->destroy(ImageSource);
    builder->destroySources(server);
    ImageSource = NULL;
    delete ui;
}

void pqWindowImage::setSubsetProperties(const CubeSubset &subset)
{
    auto sourceProxy = ImageSource->getProxy();
    vtkSMPropertyHelper(sourceProxy, "ReadSubExtent").Set(subset.ReadSubExtent);
    vtkSMPropertyHelper(sourceProxy, "SubExtent").Set(subset.SubExtent, 6);
    vtkSMPropertyHelper(sourceProxy, "AutoScale").Set(subset.AutoScale);
    vtkSMPropertyHelper(sourceProxy, "CubeMaxSize").Set(subset.CubeMaxSize);
    if (!subset.AutoScale)
        vtkSMPropertyHelper(sourceProxy, "ScaleFactor").Set(subset.ScaleFactor);
    sourceProxy->UpdateVTKObjects();
}

void pqWindowImage::changeColorMap(const QString &name)
{
    if (vtkSMProperty *lutProperty = imageProxy->GetProperty("LookupTable")) {

        auto presets = vtkSMTransferFunctionPresets::GetInstance();
        lutProxy->ApplyPreset(presets->GetFirstPresetWithName(name.toStdString().c_str()));
        vtkSMPropertyHelper(lutProperty).Set(lutProxy);
        lutProxy->UpdateVTKObjects();
        vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRange(imageProxy, false, false);
        imageProxy->UpdateVTKObjects();

        viewImage->resetDisplay();
        viewImage->render();
    }
}

void pqWindowImage::showStatusBarMessage(const std::string &msg)
{
    ui->statusBar->showMessage(QString::fromStdString(msg));
}

void pqWindowImage::createView()
{
    viewImage = qobject_cast<pqRenderView *>(
                builder->createView(pqRenderView::renderViewType(), server));
    viewImage->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->PVLayout->addWidget(viewImage->widget());
}
