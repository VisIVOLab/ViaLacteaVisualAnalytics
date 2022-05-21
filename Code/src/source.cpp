#include "source.h"

#include <QJsonArray>
#include <QJsonObject>

#include <vtkPoints.h>

Island::Island(const QJsonObject &obj, QObject *parent) : QObject(parent), obj(obj)
{
    this->name = obj["name"].toString();
    this->iau_name = obj["iau_name"].toString();
    this->npix = obj["npix"].toInt();
    this->x = obj["x"].toDouble();
    this->y = obj["y"].toDouble();
    this->xmin = obj["xmin"].toDouble();
    this->xmax = obj["xmax"].toDouble();
    this->ymin = obj["ymin"].toDouble();
    this->ymax = obj["ymax"].toDouble();
    this->ra = obj["ra"].toDouble();
    this->dec = obj["dec"].toDouble();
    this->ra_min = obj["ra_min"].toDouble();
    this->ra_max = obj["ra_max"].toDouble();
    this->dec_min = obj["dec_min"].toDouble();
    this->dec_max = obj["dec_max"].toDouble();
    this->S = obj["S"].toDouble();
    this->S_err = obj["S_err"].toDouble();
    this->Smax = obj["Smax"].toDouble();
    this->Stot = obj["Stot"].toDouble();
    this->bkg = obj["bkg"].toDouble();
    this->rms = obj["rms"].toDouble();
    this->beam_area_ratio_par = obj["beam_area_ratio_par"].toDouble();
    this->resolved = obj["resolved"].toBool();
    this->border = obj["border"].toBool();
    this->sourceness_label = obj["sourceness_label"].toString();
    this->sourceness_score = obj["sourceness_score"].toDouble();
    this->morph_label = obj["morph_label"].toString();

    foreach (auto &&it, obj["vertices"].toArray()) {
        QJsonArray vertex = it.toArray();
        QPair<double, double> point { vertex.at(0).toDouble(), vertex.at(1).toDouble() };
        this->vertices << point;
    }
}

Source *Island::getSource() const
{
    return qobject_cast<Source *>(this->parent());
}

const QList<QPair<double, double>> &Island::getVertices() const
{
    return vertices;
}

void Island::updatePoints(vtkSmartPointer<vtkPoints> points)
{
    this->vertices.clear();
    QJsonArray vertices;
    for (int i = 0; i < points->GetNumberOfPoints(); ++i) {
        double coords[3];
        points->GetPoint(i, coords);
        QJsonArray point;
        point.append(coords[0]);
        point.append(coords[1]);
        vertices.append(point);

        QPair<double, double> pair { coords[0], coords[1] };
        this->vertices << pair;
    }

    obj["vertices"] = vertices;
}

void Island::setIauName(const QString &iau_name)
{
    this->iau_name = iau_name;
    obj["iau_name"] = iau_name;
}

const QJsonObject &Island::getObj() const
{
    return obj;
}

const QString &Island::getName() const
{
    return name;
}

const QString &Island::getIauName() const
{
    return iau_name;
}

int Island::getNpix() const
{
    return npix;
}

double Island::getX() const
{
    return x;
}

double Island::getY() const
{
    return y;
}

double Island::getXmin() const
{
    return xmin;
}

double Island::getXmax() const
{
    return xmax;
}

double Island::getYmin() const
{
    return ymin;
}

double Island::getYmax() const
{
    return ymax;
}

double Island::getRa() const
{
    return ra;
}

double Island::getDec() const
{
    return dec;
}

double Island::getRaMin() const
{
    return ra_min;
}

double Island::getDecMin() const
{
    return dec_min;
}

double Island::getRaMax() const
{
    return ra_max;
}

double Island::getDecMax() const
{
    return dec_max;
}

double Island::getS() const
{
    return S;
}

double Island::getSErr() const
{
    return S_err;
}

double Island::getSmax() const
{
    return Smax;
}

double Island::getStot() const
{
    return Stot;
}

double Island::getBkg() const
{
    return bkg;
}

double Island::getRms() const
{
    return rms;
}

double Island::getBeamAreaRatioPar() const
{
    return beam_area_ratio_par;
}

bool Island::getResolved() const
{
    return resolved;
}

bool Island::getBorder() const
{
    return border;
}

const QString &Island::getSourcenessLabel() const
{
    return sourceness_label;
}

double Island::getSourcenessScore() const
{
    return sourceness_score;
}

const QString &Island::getMorphLabel() const
{
    return morph_label;
}

Source::Source(QObject *parent, const QJsonObject &obj) : QObject(parent), obj(obj) { }

const QJsonObject &Source::getObj() const
{
    return obj;
}

Source *Source::fromJson(const QJsonObject &obj, QObject *parent)
{
    Source *s = new Source(parent, obj);
    s->name = obj["name"].toString();
    s->index = obj["index"].toInt();
    s->iau_name = obj["iau_name"].toString();
    s->classid = obj["classid"].toInt();
    s->label = obj["label"].toString();
    s->nislands = obj["nislands"].toInt();
    s->x0 = obj["x0"].toDouble();
    s->y0 = obj["y0"].toDouble();
    s->ra = obj["ra"].toDouble();
    s->dec = obj["dec"].toDouble();
    s->morph_label = obj["morph_label"].toString();
    s->sourceness_label = obj["sourceness_label"].toString();
    s->sourceness_score = obj["sourceness_score"].toDouble();

    foreach (auto &&it, obj["islands"].toArray()) {
        Island *island = new Island(it.toObject(), s);
        s->islands.append(island);
    }

    return s;
}

const QList<Island *> &Source::getIslands() const
{
    return islands;
}

void Source::updatePoints(vtkSmartPointer<vtkPoints> points)
{
    auto island = islands.at(0);
    island->updatePoints(points);

    auto islands = obj["islands"].toArray();
    islands[0] = island->getObj();
    obj["islands"] = islands;
}

void Source::setIauName(const QString &iau_name)
{
    this->iau_name = iau_name;
    obj["iau_name"] = iau_name;

    auto island = islands.at(0);
    island->setIauName(iau_name);

    auto islands = obj["islands"].toArray();
    islands[0] = island->getObj();
    obj["islands"] = islands;
}

const QString &Source::getName() const
{
    return name;
}

int Source::getIndex() const
{
    return index;
}

const QString &Source::getIauName() const
{
    return iau_name;
}

int Source::getClassid() const
{
    return classid;
}

const QString &Source::getLabel() const
{
    return label;
}

int Source::getNIslands() const
{
    return nislands;
}

double Source::getX0() const
{
    return x0;
}

double Source::getY0() const
{
    return y0;
}

double Source::getRa() const
{
    return ra;
}

double Source::getDec() const
{
    return dec;
}

const QString &Source::getMorphLabel() const
{
    return morph_label;
}

const QString &Source::getSourcenessLabel() const
{
    return sourceness_label;
}

double Source::getSourcenessScore() const
{
    return sourceness_score;
}
