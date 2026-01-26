#ifndef ProfileWidget_h
#define ProfileWidget_h

#include "AstroUtils.h"

#include <vtkSmartPointer.h>

#include <QWidget>

class QCPItemLine;
class vtkImageData;
class vtkInteractorStyleProfile;

QT_BEGIN_NAMESPACE
namespace Ui {
class ProfileWidget;
}
QT_END_NAMESPACE

class ProfileWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileWidget(vtkInteractorStyleProfile *style, vtkImageData *dataset,
                           const std::string &filepath, QWidget *parent = nullptr);
    ~ProfileWidget();

    void setupImagePlots();
    void setupSpectrumPlot();

private slots:
    void setLiveMode(bool live);

private:
    Ui::ProfileWidget *ui;
    vtkSmartPointer<vtkImageData> dataset;
    AstroUtils astro;

    float *ScalarPointer;
    int Dimensions[3];
    vtkIdType Increments[3];

    vtkSmartPointer<vtkInteractorStyleProfile> interactor;
    QCPItemLine *refLineX;
    QCPItemLine *refLineY;

    void plotProfile(double x, double y, bool live);
    void plotSpectrum(double x, double y, bool live);
};

#endif
