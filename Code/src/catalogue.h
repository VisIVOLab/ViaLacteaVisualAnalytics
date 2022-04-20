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

    const QHash<QString, Source *> &getSources() const;
    const Source *getSource(QString iau_name) const;

private:
    QString filepath;
    QJsonDocument doc;
    QHash<QString, Source *> sources;
};

#endif // CATALOGUE_H
