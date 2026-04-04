#ifndef vtkWindowVbt_h
#define vtkWindowVbt_h

#include "VbtTableLoader.h"

#include <vtkNew.h>
#include <vtkActor.h>
#include <vtkDoubleArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkLookupTable.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>

#include <QMainWindow>
#include <QPointer>

class QComboBox;
class QDockWidget;
class QLabel;
class QListWidget;
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
    void updateSummary();
    QString activeColorFieldName() const;

    VbtTableData table;

    QPointer<QVTKOpenGLNativeWidget> vtkWidget;
    QPointer<QComboBox> comboColorField;
    QPointer<QSlider> sliderPointSize;
    QPointer<QLabel> summaryLabel;
    QPointer<QDockWidget> metadataDock;
    QPointer<QListWidget> fieldList;
    QPointer<QLabel> metadataLabel;

    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkNew<vtkRenderer> renderer;
    vtkNew<vtkPoints> points;
    vtkNew<vtkPolyData> polyData;
    vtkNew<vtkDoubleArray> pointScalars;
    vtkNew<vtkLookupTable> scalarLut;
    vtkNew<vtkPolyDataMapper> mapper;
    vtkNew<vtkActor> actor;
    vtkNew<vtkOrientationMarkerWidget> axesWidget;
};

#endif
