#ifndef VOTable_h
#define VOTable_h

#include <QString>
#include <QVector>

struct VOTableField
{
    QString id;
    QString name;
    QString datatype;
    QString description;
};

using VOTableRow = QVector<QVariant>;

struct VOTable
{
    QVector<VOTableField> fields;
    QVector<VOTableRow> rows;
};

#endif