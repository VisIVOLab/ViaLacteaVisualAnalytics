#ifndef PQPVWINDOW_H
#define PQPVWINDOW_H

#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqServer.h"
#include "vtkSMTransferFunctionProxy.h"

#include <QMainWindow>

namespace Ui {
class pqPVWindow;
}

class pqPVWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit pqPVWindow(pqServer* serv, pqPipelineSource* cbSrc, std::pair<int, int>& start, std::pair<int, int>& end, QWidget *parent = nullptr);
    ~pqPVWindow();

    void update(std::pair<int, int>& start, std::pair<int, int>& end);

    QString getColourMap() const;

private slots:
    int changeLutScale();
    int changeOpacity(int opacity);
    int changeLut(const QString &lutName);

    void on_actionSave_as_PNG_triggered();

    void on_actionSave_as_FITS_triggered();

private:
    Ui::pqPVWindow *ui;

    pqServer* server;
    pqObjectBuilder* builder;
    pqPipelineSource* cubeSource;
    pqPipelineSource* PVSliceFilter;
    vtkSMProxy* imageProxy;
    vtkSMTransferFunctionProxy* lutProxy;
    pqRenderView* viewImage;

    QString colourMap;
    bool logScale;

    int saveAsPNG();
    int saveAsFITS();
};

#endif // PQPVWINDOW_H
