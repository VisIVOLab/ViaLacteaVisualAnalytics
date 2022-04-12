#ifndef CATALOGUE_H
#define CATALOGUE_H

#include <QJsonDocument>
#include <QObject>

class Source;

class Catalogue : public QObject
{
    Q_OBJECT
public:
    explicit Catalogue(QObject *parent, const QString &fn);

    const QList<Source *> &getSources() const;

private:
    QString filepath;
    QJsonDocument doc;
    QList<Source *> sources;
};

#endif // CATALOGUE_H
