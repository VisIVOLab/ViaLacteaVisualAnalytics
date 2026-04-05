#ifndef vtkWindowVbt_h
#define vtkWindowVbt_h

#include "VbtTableLoader.h"
#include "vtk/ColorMaps.h"

#include <vtkNew.h>
#include <vtkActor.h>
#include <vtkCubeAxesActor.h>
#include <vtkDoubleArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkLookupTable.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>

#include <QMainWindow>
#include <QPointer>

class QCheckBox;
class QComboBox;
class QDockWidget;
class QDoubleSpinBox;
class QLabel;
class QListWidget;
class QPushButton;
class QSlider;
class QVTKOpenGLNativeWidget;
class vtkWindowVbt : public QMainWindow
{
    Q_OBJECT

public:
    explicit vtkWindowVbt(const VbtTableData &table, QWidget *parent = nullptr);
    ~vtkWindowVbt() override = default;

private:
    void setupUi();
    void setupRenderer();
    void buildPointCloud();
    void updateColorMapping();
    void updateScalarBar();
    void updateSummary();
    void updateBoundsContext();
    void applyBackground();
    void resetView();
    QString activeColorFieldName() const;
    QString activeColormapName() const;
    void setScalarRange(double minValue, double maxValue);

    VbtTableData table;

    QPointer<QVTKOpenGLNativeWidget> vtkWidget;
    QPointer<QComboBox> comboColorField;
    QPointer<QComboBox> comboColormap;
    QPointer<QComboBox> comboBackground;
    QPointer<QSlider> sliderPointSize;
    QPointer<QCheckBox> checkShowBox;
    QPointer<QCheckBox> checkShowLut;
    QPointer<QCheckBox> checkShowOrientation;
    QPointer<QDoubleSpinBox> spinRangeMin;
    QPointer<QDoubleSpinBox> spinRangeMax;
    QPointer<QPushButton> buttonAutoscale;
    QPointer<QPushButton> buttonResetView;
    QPointer<QLabel> summaryLabel;
    QPointer<QDockWidget> metadataDock;
    QPointer<QListWidget> fieldList;
    QPointer<QLabel> metadataLabel;
    bool updatingRangeControls{ false };
    double activeScalarMin{ 0.0 };
    double activeScalarMax{ 1.0 };
    double dataScalarMin{ 0.0 };
    double dataScalarMax{ 1.0 };

    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkNew<vtkRenderer> renderer;
    vtkNew<vtkPoints> points;
    vtkNew<vtkPolyData> polyData;
    vtkNew<vtkDoubleArray> pointScalars;
    vtkNew<vtkLookupTable> scalarLut;
    vtkNew<vtkPolyDataMapper> mapper;
    vtkNew<vtkActor> actor;
    vtkNew<vtkCubeAxesActor> boxActor;
    vtkNew<vtkScalarBarActor> scalarBar;
    vtkNew<vtkOrientationMarkerWidget> axesWidget;
};

#endif
