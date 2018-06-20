#include "vlkbtable.h"

VlkbTable::VlkbTable()
{


}

void VlkbTable::addColumn(QString name, QString type)
{
    struct column c;
    c.name=name;
    c.type=type;

    column_list.append(c);

}

