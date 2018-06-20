#ifndef VLKBTABLE_H
#define VLKBTABLE_H
#include <QString>
#include <QList>

struct column{
    QString name;
    QString type;
};

class VlkbTable
{
public:
    VlkbTable();
    void setName(QString n){name=n;}
    void setShortname(QString n){shortname=n;}
    QString getName(){return name;}
    QString getShortName(){return shortname;}

    void addColumn(QString name, QString type);
    QList<column> getColumnList(){return column_list; }
private:
    QString name;
    QString shortname;
    QList<column> column_list;

};

#endif // VLKBTABLE_H
