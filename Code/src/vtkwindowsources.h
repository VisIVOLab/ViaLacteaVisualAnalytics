#ifndef VTKWINDOWSOURCES_H
#define VTKWINDOWSOURCES_H

#include <QMainWindow>

#include <vtkSmartPointer.h>

class VisPoint;
class vtkOrientationMarkerWidget;
class vtkRenderer;

namespace Ui {
class vtkWindowSources;
}

class vtkWindowSources : public QMainWindow
{
    Q_OBJECT

public:
    explicit vtkWindowSources(QWidget *parent, VisPoint *visPoint);
    ~vtkWindowSources();

private:
    Ui::vtkWindowSources *ui;
    VisPoint *visPoint;

    double xThresholdMin, xThresholdMax; // xRange[2];
    double yThresholdMin, yThresholdMax; // yRange[2];
    double zThresholdMin, zThresholdMax; // zRange[2];

    double zScaleFactor;

    vtkSmartPointer<vtkOrientationMarkerWidget> axesWidget;

    inline void render();
    inline vtkRenderer *renderer();

    void showSources();
    void showBoundingBox();
    void showXYAxes();
    void showAxesLegend();
    void showOutlineTextLegend();
};

#endif // VTKWINDOWSOURCES_H
