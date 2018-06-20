#ifndef VIALACTEASTRINGDICT_H
#define VIALACTEASTRINGDICT_H

#include <QObject>
#include <QHash>
#include <QWidget>

#include <QNetworkAccessManager>
#include <QNetworkRequest>


class VialacteaStringDict
{

public:
    VialacteaStringDict();
    ~VialacteaStringDict();
private:
    QHash<QString, QString> tableUtypeStringDict;
    QHash<QString, QString> tableDescStringDict;

    QHash<QString, QString> colUtypeStringDict;
    QHash<QString, QString> colDescStringDict;

    QNetworkAccessManager *manager;

private slots:
    void availReplyFinished (QNetworkReply *reply);
    void tableReplyFinished (QNetworkReply *reply);


};

#endif // VIALACTEASTRINGDICT_H
