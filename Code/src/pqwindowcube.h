#ifndef PQWINDOWCUBE_H
#define PQWINDOWCUBE_H

#include "pqPVWindow.h"
#include "src/interactors/vtkdrawlineinteractorstyleimage.h"
#include "src/interactors/vtkinteractorstyleimagecustom.h"
#include "subsetselectordialog.h"

#include <QMainWindow>
#include <QMap>
#include <QPointer>

class pqPipelineSource;
class pqServer;
class vtkSMSessionProxyManager;
class pqObjectBuilder;
class pqRenderView;
class vtkSMProxy;
class vtkSMTransferFunctionProxy;

namespace Ui {
class pqWindowCube;
}

class pqWindowCube : public QMainWindow
{
    Q_OBJECT

    using FitsHeaderMap = QMap<QString, QString>;

public:
    explicit pqWindowCube(const QString &filepath, const CubeSubset &cubeSubset = CubeSubset());
    ~pqWindowCube() override;

private slots:
    void on_sliceSlider_actionTriggered(int action);
    void on_sliceSlider_sliderReleased();
    void on_sliceSlider_valueChanged();
    void on_sliceSpinBox_editingFinished();
    void on_sliceSpinBox_valueChanged(int arg1);
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

    void on_action0_triggered();
    void on_action25_triggered();
    void on_action50_triggered();
    void on_action75_triggered();
    void on_action100_triggered();

    void generateVolumeRendering();

    void on_actionDraw_PV_line_triggered();

    void on_actionGen_test_PV_slice_triggered();

private:
    Ui::pqWindowCube *ui;

    QString FitsFileName;
    pqPipelineSource *CubeSource;
    pqPipelineSource *SliceSource;

    pqPVWindow* pvSlice;

    CubeSubset cubeSubset;

    pqServer *server;
    vtkSMSessionProxyManager *serverProxyManager;
    pqObjectBuilder *builder;
    pqRenderView *viewCube;
    pqRenderView *viewSlice;
    vtkSMProxy *cubeSliceProxy;
    vtkSMProxy *sliceProxy;
    vtkSMTransferFunctionProxy *lutProxy;
    pqPipelineSource *extractGrid;
    QPointer<pqPipelineSource> contourFilter2D;
    pqPipelineSource *contourFilter;
    vtkSMProxy *contourProxy;

    QString fitsHeaderPath;
    FitsHeaderMap fitsHeader;
    double bounds[6];
    double dataMin;
    double dataMax;
    double rms;
    double lowerBound;
    double upperBound;
    int currentSlice;
    bool logScaleActive;
    QString currentColorMap;

    void showStatusBarMessage(const std::string &msg);

    void setSubsetProperties(const CubeSubset &subset);

    void readInfoFromSource();
    void readHeaderFromSource();
    QString createFitsHeaderFile(const FitsHeaderMap &fitsHeader);
    double getRMSFromHeader(const FitsHeaderMap &fitsHeader);

    void createViews();
    void showOutline();
    void showLegendScaleActors();
    void showSlice();
    void changeColorMap(const QString &name);
    void setLogScale(bool logScale);

    void setSliceDatacube(int value);
    void updateVelocityText();

    void setThreshold(double threshold);
    void setVolumeRenderingOpacity(double opacity);

    void showContours();
    void removeContours();

    int zDepth;
    void sendLineEndPoints(std::pair<float, float> start, std::pair<float, float> end);
    void endDrawLine();
    void showPVSlice();
    void showPVSlice(std::pair<int, int> start, std::pair<int, int> end);
    bool drawing;

    bool loadChange = false;

    vtkNew<vtkInteractorStyleImageCustom> pixCoordInteractorStyle;
    vtkNew<vtkDrawLineInteractorStyleUser> drawPVLineInteractorStyle;
};

#endif // PQWINDOWCUBE_H
