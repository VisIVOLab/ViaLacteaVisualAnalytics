#include "catalogue.h"

#include "source.h"

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
