#ifndef ProfileWidget_h
#define ProfileWidget_h

#include "AstroUtils.h"

#include <vtkSmartPointer.h>

#include <QWidget>
#include <QVector>

#include <memory>

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
    explicit ProfileWidget(QWidget *parent = nullptr);
    ~ProfileWidget();

    void setupImagePlots();
    void setupImagePlots(const QString &xLabel, const QString &yLabel);
    void setupSpectrumPlot();
    void setupSpectrumPlot(const QString &xLabel, const QString &yLabel);
    void updateSpectrumPlot(const QVector<double> &key, const QVector<double> &values,
                            const QString &title, bool live);
    void updateImageProfiles(const QVector<double> &keyX, const QVector<double> &valuesX,
                             const QVector<double> &keyY, const QVector<double> &valuesY,
                             double probeX, double probeY, bool live);

private slots:
    void setLiveMode(bool live);

private:
    Ui::ProfileWidget *ui;
    vtkSmartPointer<vtkImageData> dataset;
    std::unique_ptr<AstroUtils> astro;

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
