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
        this->sources << s;
    }
}

const QList<Source *> &Catalogue::getSources() const
{
    return sources;
}
