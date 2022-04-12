#ifndef SOURCE_H
#define SOURCE_H

#include <QObject>

class vtkLODActor;

class Island : public QObject
{
    Q_OBJECT
public:
    explicit Island(const QJsonObject &obj, QObject *parent);

    const QList<QPair<double, double>> &getVertices() const;

    void setActor(vtkLODActor *newActor);

private:
    QList<QPair<double, double>> vertices;
    vtkLODActor *actor;
};

class Source : public QObject
{
    Q_OBJECT
public:
    static Source *fromJson(const QJsonObject &obj, QObject *parent = nullptr);

    const QString &getName() const;
    int getIndex() const;
    const QString &getIau_name() const;
    int getClassid() const;
    const QString &getLabel() const;
    int getNislands() const;
    double getX0() const;
    double getY0() const;
    double getRa() const;
    double getDec() const;
    const QString &getMorph_label() const;
    const QString &getSourceness_label() const;
    double getSourceness_score() const;
    const QList<Island *> &getIslands() const;

private:
    explicit Source(QObject *parent = nullptr);

    QString name;
    int index;
    QString iau_name;
    int classid;
    QString label;
    int nislands;
    double x0;
    double y0;
    double ra;
    double dec;
    QString morph_label;
    QString sourceness_label;
    double sourceness_score;
    QList<Island *> islands;
};

#endif // SOURCE_H
