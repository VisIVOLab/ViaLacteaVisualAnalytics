#ifndef VTKWINDOWCUBE_H
#define VTKWINDOWCUBE_H

#include "vtkwindowimage.h"

#include <QMainWindow>
#include <QPointer>

#include <vtkSmartPointer.h>

class vtkActor;
class vtkFitsReader;
class vtkFlyingEdges3D;
class vtkResliceImageViewer;
class vtkOrientationMarkerWidget;
class pqPipelineSource;
class pqRenderView;
class pqDataRepresentation;
class vtkSMProxy;
class vtkSMTransferFunctionProxy;

namespace Ui {
class vtkWindowCube;
}

class vtkWindowCube : public QMainWindow
{
    Q_OBJECT

public:
    explicit vtkWindowCube(QWidget *parent, vtkSmartPointer<vtkFitsReader> fitsReader,
                           QString velocityUnit = "km/s");
    explicit vtkWindowCube(QPointer<pqPipelineSource> fitsSource, std::string fn);
    ~vtkWindowCube();

private slots:
    void on_sliceSlider_sliderReleased();
    void on_sliceSpinBox_editingFinished();

    void on_actionFront_triggered();
    void on_actionBack_triggered();
    void on_actionTop_triggered();
    void on_actionRight_triggered();
    void on_actionBottom_triggered();
    void on_actionLeft_triggered();

    void on_thresholdText_editingFinished();
    void on_thresholdSlider_sliderReleased();

    void on_contourCheckBox_toggled(bool checked);
    void on_levelText_editingFinished();
    void on_lowerBoundText_editingFinished();
    void on_upperBoundText_editingFinished();

    void on_actionCalculate_order_0_triggered();
    void on_actionCalculate_order_1_triggered();

    void on_action0_triggered();
    void on_action25_triggered();
    void on_action50_triggered();
    void on_action100_triggered();

    void changeColorMap(QString name);
    void changeColorMap(QString name, vtkSMProxy *proxy);

private:
    Ui::vtkWindowCube *ui;
    vtkSmartPointer<vtkFitsReader> fitsReader;
    QPointer<vtkWindowImage> parentWindow;

    double initialCameraFocalPoint[3];
    double initialCameraPosition[3];

    std::string filename;

    int currentSlice;
    QString velocityUnit;

    double bounds[6];
    double dataMin;
    double dataMax;

    double rms;
    double lowerBound;
    double upperBound;

    pqPipelineSource *FitsSource;

    vtkSmartPointer<vtkActor> planeActor;
    vtkSmartPointer<vtkFlyingEdges3D> isosurface;
    vtkSmartPointer<vtkOrientationMarkerWidget> axesWidget;
    vtkSmartPointer<vtkResliceImageViewer> sliceViewer;
    vtkSmartPointer<vtkActor> contoursActor;
    vtkSmartPointer<vtkActor> contoursActorForParent;

    pqPipelineSource *contourFilter;
    pqPipelineSource *contourFilter2D;
    pqPipelineSource *sliceFilter;
    vtkSMProxy *reprProxySurface;
    pqDataRepresentation *drepSlice;
    pqDataRepresentation *drepSliceCube;
    QPointer<pqRenderView> viewCube;
    QPointer<pqRenderView> viewSlice;
    vtkSMTransferFunctionProxy *lutProxy;

    QMap<QString, QString> headerMap;

    void showStatusBarMessage(const std::string &msg);

    void updateVelocityText();

    void setThreshold(double threshold);
    void setSliceDatacube(int value);

    void showContours();
    void removeContours();

    void calculateAndShowMomentMap(int order);

    void setVolumeRenderingOpacity(double opacity);

    void resetCamera();
    void setCameraAzimuth(double az);
    void setCameraElevation(double el);

    QString createFitsHeader(const QMap<QString, QString> &headerMap);
    double readRMSFromHeader(const QMap<QString, QString> &headerMap);
};

#endif // VTKWINDOWCUBE_H
