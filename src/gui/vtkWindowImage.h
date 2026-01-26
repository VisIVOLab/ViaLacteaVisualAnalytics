#ifndef vtkWindowImage_h
#define vtkWindowImage_h

#include "AstroUtils.h"

#include <vtkNew.h>

#include <QMainWindow>
#include <QPointer>

class LayerListModel;
class vtkCoordinate;
class vtkImageStack;
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
    ~vtkWindowImage() override;

private slots:
    void vtkRender();
    void changeCurrentColorMap();
    void changeCurrentColorScale();
    void changeCurrentLayerOpacity();
    void showCurrentLayerSettings();

    // Interactors
    void setInteractorStyleImage();

private:
    Ui::vtkWindowImage *ui;
    const QString filepath;
    AstroUtils astro;

    // Renderer
    vtkNew<vtkScalarBarActor> colorbar;
    vtkNew<vtkCoordinate> coordinate;
    void setupRenderer();
    void mouseCallback();

    // Stack
    vtkNew<vtkImageStack> stack;
    LayerListModel *layers;
    int currentLayerIndex() const;
    void addLayerImage(const std::string &filepath);
};

#endif
