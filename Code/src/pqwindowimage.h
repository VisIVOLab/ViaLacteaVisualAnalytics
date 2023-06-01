#ifndef PQWINDOWIMAGE_H
#define PQWINDOWIMAGE_H

#include "subsetselectordialog.h"
#include "vtkImageStack.h"

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
class pqWindowImage;
}

class pqWindowImage : public QMainWindow
{
    Q_OBJECT

    using FitsHeaderMap = QMap<QString, QString>;
public:
    explicit pqWindowImage(const QString &filepath, const CubeSubset &cubeSubset = CubeSubset());
    ~pqWindowImage();

private slots:
    void on_cmbxLUTSelect_currentIndexChanged(int index);

    void on_linearRadioButton_toggled(bool checked);

    void on_opacitySlider_actionTriggered(int action);

    void on_opacitySlider_valueChanged(int value);

    void on_opacitySlider_sliderReleased();

private:
    Ui::pqWindowImage *ui;

    QString FitsFileName;
    pqPipelineSource *ImageSource;
    vtkImageStack *imagestack;

    CubeSubset cubeSubset;

    pqServer *server;
    vtkSMSessionProxyManager *serverProxyManager;
    pqObjectBuilder *builder;
    pqRenderView *viewImage;
    vtkSMProxy *imageProxy;
    vtkSMProxy *dataProxy;
    vtkSMTransferFunctionProxy *lutProxy;

    bool clmInit;

    bool loadOpacityChange;

    bool logScaleActive;

    QString fitsHeaderPath;
    FitsHeaderMap fitsHeader;
    double bounds[6];
    double dataMin;
    double dataMax;
    double rms;
    double lowerBound;
    double upperBound;

    void setSubsetProperties(const CubeSubset& subset);
    void changeColorMap(const QString &name);
    void showStatusBarMessage(const std::string &msg);

    QString createFitsHeaderFile(const FitsHeaderMap &fitsHeader);
    void createView();
    void readInfoFromSource();
    void readHeaderFromSource();
    void showLegendScaleActor();
    void rescaleForLog();
    void setLogScale(bool logScale);
    void setOpacity(float value);
};

#endif // PQWINDOWIMAGE_H
