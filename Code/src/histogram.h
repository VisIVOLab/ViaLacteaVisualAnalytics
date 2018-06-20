#pragma once

#include <QObject>
#include <QWidget>

class Histogram : public QWidget
{
    Q_OBJECT
private:
    QVector<int> _bins;
    bool _toggled;
    int _heightMax;

public:
    Histogram(QWidget* parent = 0, Qt::WindowFlags f = 0);
    void setBins(QVector<int>& bins);
    void setBins(int* bins, int nbBins);
    void clear();
    bool toggled() {return _toggled;}

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
};
