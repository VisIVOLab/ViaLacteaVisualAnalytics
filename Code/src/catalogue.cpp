#include "catalogue.h"

#include "source.h"

#include <vtkLODActor.h>
#include <vtkPolyDataAlgorithm.h>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

Catalogue::Catalogue(QObject *parent, const QString &fn) : QObject(parent), filepath(fn)
{
    QFile file(fn);
    file.open(QFile::ReadOnly);
    doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonArray sources = doc["sources"].toArray();
    if (sources.isEmpty()) {
        QMessageBox::information(nullptr, "No sources", "The file does not contain any sources.");
        return;
    }

    foreach (auto &&it, sources) {
        Source *s = Source::fromJson(it.toObject(), this);
        this->sources.insert(s->getIau_name(), s);
    }
}

void Catalogue::AddExtractedSource(vtkSmartPointer<vtkPolyDataAlgorithm> Filter,
                                   vtkSmartPointer<vtkLODActor> Actor)
{
    QPair<vtkSmartPointer<vtkPolyDataAlgorithm>, vtkSmartPointer<vtkLODActor>> pair(Filter, Actor);
    extracted.append(pair);
}

const QHash<QString, Source *> &Catalogue::getSources() const
{
    return sources;
}

const Source *Catalogue::getSource(QString iau_name) const
{
    if (this->sources.contains(iau_name)) {
        return this->sources.value(iau_name);
    }

    return nullptr;
}

vtkSmartPointer<vtkLODActor> Catalogue::GetActor() const
{
    return Actor;
}

void Catalogue::SetActor(vtkSmartPointer<vtkLODActor> actor)
{
    this->Actor = actor;
}

vtkSmartPointer<vtkPolyDataAlgorithm> Catalogue::GetPolyDataFilter() const
{
    return PolyDataFilter;
}

void Catalogue::SetPolyDataFilter(vtkSmartPointer<vtkPolyDataAlgorithm> PolyDataFilter)
{
    this->PolyDataFilter = PolyDataFilter;
}
