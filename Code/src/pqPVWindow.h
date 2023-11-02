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
};

#endif // PQPVWINDOW_H
