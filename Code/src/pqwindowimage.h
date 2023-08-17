#ifndef PQWINDOWIMAGE_H
#define PQWINDOWIMAGE_H

#include "subsetselectordialog.h"
#include "vlvastackimage.h"

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

    void on_btnAddImageToStack_clicked();

    void on_btnRemoveImageFromStack_clicked();

    void on_sbxStackActiveLayer_valueChanged(int arg1);

private:
    Ui::pqWindowImage *ui;

    std::vector<vlvaStackImage> images;

    pqServer *server;
    vtkSMSessionProxyManager *serverProxyManager;
    pqObjectBuilder *builder;
    pqRenderView *viewImage;
    vtkSMProxy *imageProxy;
    vtkSMTransferFunctionProxy *lutProxy;

    bool clmInit;

    bool loadOpacityChange;

    bool logScaleActive;

    int activeIndex;

    void changeColorMap(const QString &name);
    void showStatusBarMessage(const std::string &msg);

    void updateUI();

    QString createFitsHeaderFile(const FitsHeaderMap &fitsHeader);
    void createView();
    void readInfoFromSource();
    void readHeaderFromSource();
    void showLegendScaleActor();
    void setLogScale(bool logScale);
    void setOpacity(float value);
    int addImageToStack(QString file);
    int addImageToStack(QString file, const CubeSubset& subset);
    int removeImageFromStack(const int index);
};

#endif // PQWINDOWIMAGE_H
