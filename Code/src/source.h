#ifndef SOURCE_H
#define SOURCE_H

#include <QJsonObject>
#include <QObject>

class vtkLODActor;

/*
== SOURCE PARAMETRI ==
name (iau_name)
nislands
pos(x0,y0)
pos_wcs(ra,dec)
label
classid
morph_label
sourceness_label
sourceness_score
== ISLAND PARAMETRI ==
name (iau_name)
npix
pos=(x,y) bbox(xmin,xmax,ymin,ymax)
pos_wcs(ra,dec) bbox_wcs(ra_min,ra_max,dec_min,dec_max)
flux= S +- S_err
Smax, Stot
bkg, rms
fitinfo=0/1, ncomponents (fit_quality, redchi2=chi/ndf)
min_size, max_size (per il momento sono non implementate)
beam_area_ratio_par
resolved
border
sourceness_label
sourceness_score
*/

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
    const QString &getSourcenessLabel() const;
    double getSourcenessScore() const;
    const QString &getMorphLabel() const;
    const QList<QPair<double, double>> &getVertices() const;

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
    double S, S_err;
    double Smax, Stot;
    double bkg, rms;
    double beam_area_ratio_par;
    bool resolved;
    bool border;
    QString sourceness_label;
    double sourceness_score;
    QString morph_label;
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
    const QString &getLabel() const;
    int getNIslands() const;
    double getX0() const;
    double getY0() const;
    double getRa() const;
    double getDec() const;
    const QString &getMorphLabel() const;
    const QString &getSourcenessLabel() const;
    double getSourcenessScore() const;
    const QList<Island *> &getIslands() const;

private:
    explicit Source(QObject *parent, const QJsonObject &obj);

    QJsonObject obj;

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
