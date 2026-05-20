#include "VOTableParser.h"

#include <QFile>
#include <QVariant>

using namespace Qt::StringLiterals;

VOTableParser::VOTableParser() = default;

VOTableParser::~VOTableParser() = default;

QString VOTableParser::errorString() const
{
    return this->reader.errorString();
}

VOTable VOTableParser::votable()
{
    return std::move(this->table);
}

bool VOTableParser::parseFile(const QString &filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    return parse(&file);
}

bool VOTableParser::parse(QIODevice *device)
{
    this->reader.setDevice(device);

    while (!this->reader.atEnd()) {
        this->reader.readNext();

        if (this->reader.isStartElement() && this->reader.name().compare("VOTABLE"_L1) == 0) {
            this->parseVOTable();
        }
    }

    return !this->reader.hasError();
}

void VOTableParser::parseVOTable()
{
    while (!(this->reader.isEndElement() && this->reader.name().compare("VOTABLE"_L1) == 0)) {
        this->reader.readNext();

        if (this->reader.isStartElement() && this->reader.name().compare("RESOURCE"_L1) == 0) {
            this->parseResource();
        }
    }
}

void VOTableParser::parseResource()
{
    while (!(this->reader.isEndElement() && this->reader.name().compare("RESOURCE"_L1) == 0)) {
        this->reader.readNext();

        if (this->reader.isStartElement() && this->reader.name().compare("TABLE"_L1) == 0) {
            this->parseTable();
        }
    }
}

void VOTableParser::parseTable()
{
    while (!(this->reader.isEndElement() && this->reader.name().compare("TABLE"_L1) == 0)) {
        this->reader.readNext();

        if (!this->reader.isStartElement()) {
            continue;
        }

        if (this->reader.name().compare("FIELD"_L1) == 0) {
            this->parseField();
        }

        if (this->reader.name().compare("DATA"_L1) == 0) {
            this->parseData();
        }
    }
}

void VOTableParser::parseField()
{
    const auto attrs = this->reader.attributes();

    VOTableField field;
    field.id = attrs.value("ID"_L1).toString();
    field.name = attrs.value("name"_L1).toString();
    field.datatype = attrs.value("datatype"_L1).toString();

    this->reader.readNextStartElement();
    field.description = this->reader.readElementText();

    this->table.fields.push_back(field);
}

void VOTableParser::parseData()
{
    while (!(this->reader.isEndElement() && this->reader.name().compare("DATA"_L1) == 0)) {
        this->reader.readNext();

        if (this->reader.isStartElement() && this->reader.name().compare("TABLEDATA"_L1) == 0) {
            this->parseTableData();
        }
    }
}

void VOTableParser::parseTableData()
{
    while (!(this->reader.isEndElement() && this->reader.name().compare("TABLEDATA"_L1) == 0)) {
        this->reader.readNext();

        if (this->reader.isStartElement() && this->reader.name().compare("TR"_L1) == 0) {
            VOTableRow row;
            int fieldIdx = 0;

            while (!(this->reader.isEndElement() && this->reader.name().compare("TR"_L1) == 0)) {
                this->reader.readNext();

                if (this->reader.isStartElement() && this->reader.name().compare("TD"_L1) == 0) {
                    const QString text = this->reader.readElementText();
                    QVariant value;

                    if (fieldIdx < this->table.fields.size()) {
                        value = VOTableParser::convertValue(text,
                                                            this->table.fields[fieldIdx].datatype);
                        row.push_back(value);
                        ++fieldIdx;
                    }
                }
            }

            this->table.rows.push_back(row);
        }
    }
}

QVariant VOTableParser::convertValue(const QString &value, const QString &datatype)
{
    if (value.isEmpty()) {
        return QVariant();
    }

    if (datatype == u"short"_s) {
        return value.toShort();
    }

    if (datatype == u"int"_s) {
        return value.toInt();
    }

    if (datatype == u"long"_s) {
        return value.toLongLong();
    }

    if (datatype == u"float"_s) {
        return value.toFloat();
    }

    if (datatype == u"double"_s) {
        return value.toDouble();
    }

    if (datatype == u"bool"_s) {
        return value == u"1"_s || value.compare("true"_L1, Qt::CaseInsensitive) == 0;
    }

    return value;
}
