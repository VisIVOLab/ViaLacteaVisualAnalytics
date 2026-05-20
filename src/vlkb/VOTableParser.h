#ifndef VOTableParser_h
#define VOTableParser_h

#include "VOTable.h"

#include <QXmlStreamReader>

class VOTableParser
{
public:
    VOTableParser();
    ~VOTableParser();

    bool parseFile(const QString &filePath);
    bool parse(QIODevice *device);

    QString errorString() const;

    VOTable votable();

private:
    QXmlStreamReader reader;
    VOTable table;

    void parseVOTable();
    void parseResource();
    void parseTable();
    void parseField();
    void parseData();
    void parseTableData();

    static QVariant convertValue(const QString &value, const QString &datatype);
};

#endif