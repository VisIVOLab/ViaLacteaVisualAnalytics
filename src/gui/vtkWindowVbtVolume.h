#ifndef vtkWindowVbtVolume_h
#define vtkWindowVbtVolume_h

#include "VbtTableLoader.h"

#include <vtkNew.h>
#include <vtkColorTransferFunction.h>
#include <vtkCubeAxesActor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

#include <QMainWindow>
#include <QPointer>

class QComboBox;
class QDockWidget;
class QLabel;
class QListWidget;
class QSlider;
class QVTKOpenGLNativeWidget;
class vtkImageData;

class vtkWindowVbtVolume : public QMainWindow
{
    Q_OBJECT

public:
    explicit vtkWindowVbtVolume(const VbtTableData &table, QWidget *parent = nullptr);
    ~vtkWindowVbtVolume() override = default;

private:
    void setupUi();
    void setupRenderer();
    void rebuildVolumeData();
    void updateSummary();
    void updateBoundsContext();
    void updateScalarBar();
    void resetView();
    void applyKnownGoodVisibleFallback(double minValue, double maxValue, const double bounds[6]);
    static double percentileFromSortedSample(const std::vector<double> &values, double fraction);

    VbtTableData table;

    QPointer<QVTKOpenGLNativeWidget> vtkWidget;
    QPointer<QComboBox> comboScalarField;
    QPointer<QSlider> sliderOpacity;
    QPointer<QLabel> summaryLabel;
    QPointer<QDockWidget> metadataDock;
    QPointer<QListWidget> fieldList;
    QPointer<QLabel> metadataLabel;

    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkNew<vtkRenderer> renderer;
    vtkNew<vtkSmartVolumeMapper> volumeMapper;
    vtkNew<vtkVolumeProperty> volumeProperty;
    vtkNew<vtkVolume> volumeActor;
    vtkNew<vtkColorTransferFunction> colorTransfer;
    vtkNew<vtkPiecewiseFunction> opacityTransfer;
    vtkNew<vtkCubeAxesActor> boxActor;
    vtkNew<vtkScalarBarActor> scalarBar;
    vtkNew<vtkOrientationMarkerWidget> axesWidget;
    vtkSmartPointer<vtkImageData> currentImageData;
};

#endif
