#include "source.h"

#include <QJsonArray>
#include <QJsonObject>

Island::Island(const QJsonObject &obj, QObject *parent) : QObject(parent)
{
    QJsonArray vertices = obj["vertices"].toArray();
    foreach (auto &&it, vertices) {
        QJsonArray vertex = it.toArray();
        QPair<double, double> point { vertex.at(0).toDouble(), vertex.at(1).toDouble() };
        this->vertices << point;
    }
}

const QList<QPair<double, double>> &Island::getVertices() const
{
    return vertices;
}

void Island::setActor(vtkLODActor *newActor)
{
    actor = newActor;
}

Source::Source(QObject *parent) : QObject(parent) { }

const QList<Island *> &Source::getIslands() const
{
    return islands;
}

const QString &Source::getName() const
{
    return name;
}

int Source::getIndex() const
{
    return index;
}

const QString &Source::getIau_name() const
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

int Source::getNislands() const
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

const QString &Source::getMorph_label() const
{
    return morph_label;
}

const QString &Source::getSourceness_label() const
{
    return sourceness_label;
}

double Source::getSourceness_score() const
{
    return sourceness_score;
}

Source *Source::fromJson(const QJsonObject &obj, QObject *parent)
{
    Source *s = new Source(parent);
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
        s->islands << island;
    }

    return s;
}
