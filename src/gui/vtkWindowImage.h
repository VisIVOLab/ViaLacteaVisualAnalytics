#ifndef vtkWindowImage_h
#define vtkWindowImage_h

#include "AstroUtils.h"

#include <vtkNew.h>

#include <QMainWindow>
#include <QPointer>

#include <memory>

class ImageLayerController;
class LayerListModel;
class LUTCustomizerDialog;
class ProfileWidget;
class vtkCoordinate;
class vtkImageStack;
class vtkLegendScaleActorWCS;
class vtkScalarBarActor;

QT_BEGIN_NAMESPACE
namespace Ui {
class vtkWindowImage;
}
QT_END_NAMESPACE

class vtkWindowImage : public QMainWindow
{
    Q_OBJECT

public:
    explicit vtkWindowImage(const QString &filepath, QWidget *parent = nullptr);
    ~vtkWindowImage();

private slots:
    void showLUTCustomizer();
    void updateLUTCustomizer();

    void addLocalFile();

    void vtkRender();
    void changeLegendWCS();
    void changeCurrentColorMap();
    void changeCurrentColorScale();
    void changeCurrentLayerOpacity();
    void showCurrentLayerSettings();

    // Interactors
    void setInteractorStyleImage();
    void setInteractorStyleProfile();

private:
    Ui::vtkWindowImage *ui;
    const QString filepath;
    AstroUtils astro;
    QPointer<LUTCustomizerDialog> lutCustomizer;
    QPointer<ProfileWidget> profileWidget;

    // Renderer
    vtkNew<vtkLegendScaleActorWCS> legendWCS;
    vtkNew<vtkScalarBarActor> colorbar;
    vtkNew<vtkCoordinate> coordinate;
    void setupRenderer();
    void mouseCallback();

    // Stack
    vtkNew<vtkImageStack> stack;
    LayerListModel *layers;
    std::unique_ptr<ImageLayerController> layerController;
    int currentLayerIndex() const;
    void addLayerImage(const std::string &filepath);
};

#endif
