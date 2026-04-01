#ifndef PvDiagramWidget_h
#define PvDiagramWidget_h

#include <QVector>
#include <QWidget>

class QLabel;
class QCustomPlot;
class QCPColorMap;
class QCPTextElement;

class PvDiagramWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PvDiagramWidget(QWidget *parent = nullptr);

    void setPvData(const QVector<double> &positions, const QVector<double> &spectral,
                   const QVector<double> &intensities, int nx, int ny, const QString &xLabel,
                   const QString &yLabel, const QString &title, const QString &details);

private:
    QLabel *infoLabel;
    QCustomPlot *plot;
    QCPColorMap *colorMap;
    QCPTextElement *titleElement;
};

#endif
