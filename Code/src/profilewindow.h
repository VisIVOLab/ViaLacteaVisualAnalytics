#ifndef PROFILEWINDOW_H
#define PROFILEWINDOW_H

#include <QWidget>

class QCustomPlot;

namespace Ui {
class ProfileWindow;
}

class ProfileWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileWindow(const QString &bunit = QString(), QWidget *parent = nullptr);
    ~ProfileWindow();

    void setupSpectrumPlot(int channels, double crval, double cdelt, double crpix,
                           const QString &cunit);
    void setupSpectrumPlot(int channels, double crval, double cdelt, double crpix, double restfrq);

    void setLiveProfileFlag(bool flag);
    void plotProfiles(const QVector<double> &xProfile, double xRef, const QVector<double> &yProfile,
                      double yRef);
    void plotSpectrum(const QVector<double> &spectrum, const QVector<double> &nanIndices, int x,
                      int y, double nulval);

signals:
    void liveUpdateStateChanged(int status);

private:
    Ui::ProfileWindow *ui;
    QVector<double> key;

    void plotProfile(const QVector<double> &profile, double ref, QCustomPlot *plot);
};

#endif // PROFILEWINDOW_H
