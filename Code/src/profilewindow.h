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

    void setLiveProfileFlag(bool flag);
    void plotProfiles(const QVector<double> &xProfile, double xRef, const QVector<double> &yProfile,
                      double yRef);

signals:
    void liveUpdateStateChanged(int status);

private:
    Ui::ProfileWindow *ui;
    void plotProfile(const QVector<double> &profile, double ref, QCustomPlot *plot);
};

#endif // PROFILEWINDOW_H
