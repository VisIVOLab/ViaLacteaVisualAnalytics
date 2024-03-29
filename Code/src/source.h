#ifndef SOURCE_H
#define SOURCE_H

#include <QJsonObject>
#include <QObject>

#include <vtkSmartPointer.h>

class vtkPoints;
class vtkLODActor;
class Source;

class Island : public QObject
{
    Q_OBJECT
public:
    explicit Island(const QJsonObject &obj, QObject *parent);
    Source *getSource() const;

    const QString &getName() const;
    const QString &getIauName() const;
    int getNpix() const;
    double getX() const;
    double getY() const;
    double getXmin() const;
    double getXmax() const;
    double getYmin() const;
    double getYmax() const;
    double getRa() const;
    double getDec() const;
    double getRaMin() const;
    double getDecMin() const;
    double getRaMax() const;
    double getDecMax() const;
    double getS() const;
    double getSErr() const;
    double getSmax() const;
    double getStot() const;
    double getBkg() const;
    double getRms() const;
    double getBeamAreaRatioPar() const;
    bool getResolved() const;
    bool getBorder() const;
    bool isFitInfoPresent() const;
    const QString &getSourcenessLabel() const;
    double getSourcenessScore() const;
    const QString &getMorphLabel() const;
    const QList<QPair<double, double>> &getVertices() const;

    void updatePoints(vtkSmartPointer<vtkPoints> points);
    void setIauName(const QString &iau_name);

    const QJsonObject &getObj() const;

    void setMorph_label(const QString &newMorph_label);
    void setClass_label(const QString &newClass_label);
    void setSourceness_label(const QString &newSourceness_label);

    double getMinSize() const;
    double getMaxSize() const;

private:
    QJsonObject obj;

    QString name;
    QString iau_name;
    int npix;
    double x, y;
    double xmin, xmax;
    double ymin, ymax;
    double ra, dec;
    double ra_min, dec_min;
    double ra_max, dec_max;
    double min_size, max_size;
    double S, S_err;
    double Smax, Stot;
    double bkg, rms;
    bool fit_info;
    double beam_area_ratio_par;
    bool resolved;
    bool border;
    QString sourceness_label;
    double sourceness_score;
    QString morph_label;
    QString class_label;
    QList<QPair<double, double>> vertices;
};

class Source : public QObject
{
    Q_OBJECT
public:
    static Source *fromJson(const QJsonObject &obj, QObject *parent = nullptr);

    const QString &getName() const;
    int getIndex() const;
    const QString &getIauName() const;
    int getClassid() const;
    const QString &getClassLabel() const;
    int getNIslands() const;
    double getX0() const;
    double getY0() const;
    double getRa() const;
    double getDec() const;
    const QString &getMorphLabel() const;
    const QString &getSourcenessLabel() const;
    double getSourcenessScore() const;
    const QList<Island *> &getIslands() const;

    void updatePoints(vtkSmartPointer<vtkPoints> points);
    void setIauName(const QString &iau_name);

    const QJsonObject &getObj() const;

    void setMorph_label(const QString &newMorph_label);
    void setClass_label(const QString &newClass_label);
    void setSourceness_label(const QString &newSourceness_label);

private:
    explicit Source(QObject *parent, const QJsonObject &obj);

    QJsonObject obj;

    QString name;
    int index;
    QString iau_name;
    int classid;
    QString class_label;
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
