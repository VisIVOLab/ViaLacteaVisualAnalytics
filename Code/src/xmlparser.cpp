#include "xmlparser.h"

#include <algorithm>
#include <QDebug>
#include <QFile>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QList>
#include <QListWidgetItem>
#include <QMap>
#include <QMessageBox>
#include <QModelIndex>
#include <QStandardItem>
#include <QStringList>
#include <qwidget.h>

xmlparser::xmlparser()
{
    queryUrl = "ia2-vo.oats.inaf.it/vialactea/cutouts/";
}
xmlparser::~xmlparser() { }

void xmlparser::parseXML_fitsDownload(QXmlStreamReader &xml_reader, QString &stringa)
{
    QList<QMap<QString, QString>> datacubes;
    QString url;
    QMap<QString, QString> survey_map;

    while (!xml_reader.atEnd() && !xml_reader.hasError()) {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = xml_reader.readNext();

        /* If token is just StartDocument, we'll go to next.*/
        if (token == QXmlStreamReader::StartDocument) {
            continue;
        }

        /* If token is StartElement, we'll see if we can read it.*/
        if (token == QXmlStreamReader::StartElement) {
            if (xml_reader.name() == "results") {
                continue;
            }
            if (xml_reader.name() == "description") {
                continue;
            }
            if (xml_reader.name() == "input") {
                continue;
            }
            if (xml_reader.name() == "OriginFits") {
                continue;
            }
            if (xml_reader.name() == "survey") {
                survey_map = this->parseSurvey(xml_reader);
            }

            if (xml_reader.name() == "datacube") {
                // datacubes.append(this->parseDatacube(xml_reader, stringa));

                QMap<QString, QString> datacube_map = this->parseDatacube(xml_reader);
                int test_flag_nanten = -1;
                if (datacube_map["PublisherDID"].compare("") != 0)
                    test_flag_nanten = datacube_map["PublisherDID"]
                            .split("_", QString::SkipEmptyParts)
                            .last()
                            .toInt();
                if (datacube_map["code"].compare("1") == 0
                        || datacube_map["code"].compare("6") == 0) {
                    qDebug() << "Do not add datacube wiwh code " << datacube_map["code"];
                } else if (survey_map["Survey"].compare("NANTEN") == 0 && test_flag_nanten == 2) {
                    qDebug() << "NANTEN 2d Element not added";
                    qDebug() << survey_map["Survey"] << " " << datacube_map["PublisherDID"] << " "
                                                     << test_flag_nanten;
                } else {
                    if (survey_map["Survey"].compare("") == 0) {
                        datacube_map.insert("Survey", "Extinction_Map");
                        datacube_map.insert("Species", "dust");
                        datacube_map.insert("Transition", "visual");
                    } else {
                        datacube_map.insert("Survey", survey_map["Survey"]);
                        datacube_map.insert("Species", survey_map["Species"]);
                        datacube_map.insert("Transition", survey_map["Transition"]);
                        datacube_map.insert("Description", survey_map["Description"]);
                    }
                    datacubes.append(datacube_map);
                }
            }

            if (xml_reader.name() == "URL") {
                xml_reader.readNext();
                if (xml_reader.tokenType() == QXmlStreamReader::Characters) {
                    url = xml_reader.text().toString();
                }
            }
        }
    }
    xml_reader.clear();
    stringa = this->extractPublisherDID_fits(datacubes, datacube_element);

    if (url.compare("") != 0) {
        stringa = url;
    }
}

QList<QMap<QString, QString>> xmlparser::parseXmlAndGetList(QXmlStreamReader &xml_reader)
{
    QList<QMap<QString, QString>> datacubes;
    QString url;
    QMap<QString, QString> survey_map;

    while (!xml_reader.atEnd() && !xml_reader.hasError()) {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = xml_reader.readNext();

        /* If token is just StartDocument, we'll go to next.*/
        if (token == QXmlStreamReader::StartDocument) {
            continue;
        }

        /* If token is StartElement, we'll see if we can read it.*/
        if (token == QXmlStreamReader::StartElement) {
            if (xml_reader.name() == "results") {
                continue;
            }
            if (xml_reader.name() == "description") {
                continue;
            }
            if (xml_reader.name() == "input") {
                continue;
            }
            if (xml_reader.name() == "msg") {
                continue;
            }
            if (xml_reader.name() == "OriginFits") {
                continue;
            }
            if (xml_reader.name() == "DatacubeCount") {
                continue;
            }

            if (xml_reader.name() == "survey") {
                survey_map = this->parseSurvey(xml_reader);
            }

            if (xml_reader.name() == "VelocityUnit") {
                this->parseVelocity(xml_reader);
            }
            if (xml_reader.name() == "datacube") {
                // datacubes.append(this->parseDatacube(xml_reader, stringa));

                QMap<QString, QString> datacube_map = this->parseDatacube(xml_reader);
                int test_flag_nanten = -1;
                if (datacube_map["PublisherDID"].compare("") != 0)
                    test_flag_nanten = datacube_map["PublisherDID"]
                            .split("_", QString::SkipEmptyParts)
                            .last()
                            .toInt();
                if (datacube_map["code"].compare("1") == 0
                        || datacube_map["code"].compare("6") == 0) {
                } else if (test_flag_nanten == 2
                           || test_flag_nanten
                           == 3) { // datacube_map["Survey"].compare("Cornish")==0){
                } else {
                    if (survey_map["Survey"].compare("") == 0) {
                        datacube_map.insert("Survey", "Extinction_Map");
                        datacube_map.insert("Species", "dust");
                        datacube_map.insert("Transition", "visual");
                    } else {
                        datacube_map.insert("Survey", survey_map["Survey"]);
                        datacube_map.insert("Species", survey_map["Species"]);
                        datacube_map.insert("Transition", survey_map["Transition"]);
                        datacube_map.insert("Description", survey_map["Description"]);
                    }
                    datacubes.append(datacube_map);
                }
            }
            if (xml_reader.name() == "URL") {
                xml_reader.readNext();
                if (xml_reader.tokenType() == QXmlStreamReader::Characters) {
                    url = xml_reader.text().toString();
                }
            }
        }
    }
    xml_reader.clear();

    std::sort(datacubes.begin(), datacubes.end());

    return datacubes;
}

bool operator<(const QMap<QString, QString> &a, const QMap<QString, QString> &b)
{
    return a.value("Survey") < b.value("Survey");
}

void xmlparser::parseXML(QXmlStreamReader &xml_reader, QString &stringa, bool dc)
{
    QList<QMap<QString, QString>> datacubes;
    QString url;
    double perc = 0.0;

    while (!xml_reader.atEnd() && !xml_reader.hasError()) {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = xml_reader.readNext();

        /* If token is just StartDocument, we'll go to next.*/
        if (token == QXmlStreamReader::StartDocument) {
            continue;
        }

        /* If token is StartElement, we'll see if we can read it.*/
        if (token == QXmlStreamReader::StartElement) {
            if (xml_reader.name() == "results") {
                continue;
            }
            if (xml_reader.name() == "description") {
                continue;
            }
            if (xml_reader.name() == "input") {
                continue;
            }
            if (xml_reader.name() == "OriginFits") {
                continue;
            }

            if (xml_reader.name() == "datacube") {
                QMap<QString, QString> datacube_map = this->parseDatacube(xml_reader);

                int test_flag_nanten = -1;
                if (datacube_map["PublisherDID"].compare("") != 0)
                    test_flag_nanten = datacube_map["PublisherDID"]
                            .split("_", QString::SkipEmptyParts)
                            .last()
                            .toInt();
                if (datacube_map["code"].compare("1") == 0
                        || datacube_map["code"].compare("6") == 0) {
                } else if (datacube_map["Survey"].compare("NANTEN") == 0 && test_flag_nanten == 2
                           || datacube_map["Survey"].compare("Cornish") == 0) {
                } else {
                    if (datacube_map["Survey"].compare("") == 0) {
                        datacube_map.insert("Survey", "Extinction_Map");
                        datacube_map.insert("Species", "dust");
                        datacube_map.insert("Transition", "visual");
                    }
                    datacubes.append(datacube_map);
                }
            }
            /* Read here percentage of null values into the vlkb_cutout service*/
            if (xml_reader.name() == "percent") {
                xml_reader.readNext();
                if (xml_reader.tokenType() == QXmlStreamReader::Characters) {
                    perc = xml_reader.text().toDouble();
                }
            }

            if (xml_reader.name() == "URL") {
                xml_reader.readNext();
                if (xml_reader.tokenType() == QXmlStreamReader::Characters) {
                    url = xml_reader.text().toString();
                }
            }
        }
    }
    xml_reader.clear();
    if (dc)
        stringa = this->extractPublisherDID(datacubes, datacube_element);
    else
        ;
    if (url.compare("") != 0) {
        if (perc < 95.0)
            stringa = url;
        else
            stringa = "NULL " + QString::number(perc);
    }
}
QMap<QString, QString> xmlparser::parseSurvey(QXmlStreamReader &xml)
{
    velocity = "";
    QMap<QString, QString> survey_map;

    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == "RestFreq") {
        return survey_map;
    }

    xml.readNext();

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "RestFreq")) {

        if (xml.tokenType() == QXmlStreamReader::StartElement) {

            /* We've found Description. */
            if (xml.name() == "Description") {
                this->addDatacubeToMap(xml, survey_map);
            }
            /* We've found Survey. */
            if (xml.name() == "Survey") {
                this->addDatacubeToMap(xml, survey_map);
            }
            /* We've found Species. */
            if (xml.name() == "Species") {
                this->addDatacubeToMap(xml, survey_map);
            }
            /* We've found Transition. */
            if (xml.name() == "Transition") {
                this->addDatacubeToMap(xml, survey_map);
            }

            if (xml.name() == "VelocityUnit") {
                this->parseVelocity(xml);
            }
        }
        /* ...and next... */
        xml.readNext();
    }

    return survey_map;
}

void xmlparser::parseVelocity(QXmlStreamReader &xml)
{
    /* We need a start element, like <foo> */
    if (xml.tokenType() != QXmlStreamReader::StartElement) {
        return;
    }
    /* Let's read the name... */
    QString elementName = xml.name().toString();

    /* ...go to the next. */
    xml.readNext();
    /*
     * This elements needs to contain Characters so we know it's
     * actually data, if it's not we'll leave.
     */
    if (xml.tokenType() != QXmlStreamReader::Characters) {
        return;
    }
    /* Now we can add it to the map.*/
    if (xml.text().toString().compare("") != 0) {
        velocity = xml.text().toString();
    }
}

QMap<QString, QString> xmlparser::parseDatacube(QXmlStreamReader &xml)
{

    QMap<QString, QString> datacube_map;
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == "datacube") {
        return datacube_map;
    }

    xml.readNext();

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "datacube")) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            /* We've found SQL. */
            if (xml.name() == "SQL") {
                this->addDatacubeToMap(xml, datacube_map);
            }
            /* We've found overlap. */
            if (xml.name() == "overlap") {
                parseOverlap(xml, datacube_map);
            }
            /* We've found PublisherDID. */
            if (xml.name() == "PublisherDID") {
                this->addDatacubeToMap(xml, datacube_map);
            }
            /* We've found Description. */
            if (xml.name() == "Description") {
                this->addDatacubeToMap(xml, datacube_map);
            }
            /* We've found Survey. */
            if (xml.name() == "Survey") {
                this->addDatacubeToMap(xml, datacube_map);
            }
            /* We've found Species. */
            if (xml.name() == "Species") {
                this->addDatacubeToMap(xml, datacube_map);
            }
            /* We've found Transition. */
            if (xml.name() == "Transition") {
                this->addDatacubeToMap(xml, datacube_map);
            }
            if (xml.name() == "URL") {
                // temp fix: prendo solo cutout url
                if ((xml.attributes().value("type").toString().compare("cutout") == 0)
                        || (xml.attributes().value("type").toString().compare("mosaic") == 0))
                    this->addDatacubeToMap(xml, datacube_map);
            }

            if (xml.name() == "vertices") {
                parseBounds(xml, datacube_map);
            }

            datacube_map.insert("VelocityUnit", velocity);
        }
        /* ...and next... */
        xml.readNext();
    }

    return datacube_map;
}

void xmlparser::parseSpectralCoordinate(QXmlStreamReader &xml, QMap<QString, QString> &datacube_map)
{

    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == "velocity") {
        return;
    }

    xml.readNext();

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "velocity")) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {

            if (xml.name() == "from") {
                this->addDatacubeToMap(xml, datacube_map);
            }
            if (xml.name() == "to") {
                this->addDatacubeToMap(xml, datacube_map);
            }
        }
        xml.readNext();
    }
}

void xmlparser::parseOverlap(QXmlStreamReader &xml, QMap<QString, QString> &datacube_map)
{

    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == "overlap") {
        return;
    }

    xml.readNext();

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "overlap")) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {

            if (xml.name() == "description") {
                if (xml.tokenType() == QXmlStreamReader::StartElement) {
                    xml.readNext();
                    datacube_map.insert("overlapDescription", xml.text().toString());
                }
            }
            if (xml.name() == "code") {
                this->addDatacubeToMap(xml, datacube_map);
            }
        }
        xml.readNext();
    }
}

void xmlparser::parseBounds(QXmlStreamReader &xml, QMap<QString, QString> &datacube_map)
{
    while (!(xml.name() == "SkyCoordSystem" && xml.isEndElement())) {
        if (!xml.readNextStartElement()) {
            continue;
        }

        if (xml.name() == "P1") {
            xml.readNextStartElement(); // start longitude
            datacube_map.insert("longitudeP1", xml.readElementText());
            xml.readNextStartElement(); // start latitude
            datacube_map.insert("latitudeP1", xml.readElementText());
        }

        if (xml.name() == "P2") {
            xml.readNextStartElement(); // start longitude
            datacube_map.insert("longitudeP2", xml.readElementText());
            xml.readNextStartElement(); // start latitude
            datacube_map.insert("latitudeP2", xml.readElementText());
        }

        if (xml.name() == "P3") {
            xml.readNextStartElement(); // start longitude
            datacube_map.insert("longitudeP3", xml.readElementText());
            xml.readNextStartElement(); // start latitude
            datacube_map.insert("latitudeP3", xml.readElementText());
        }

        if (xml.name() == "P4") {
            xml.readNextStartElement(); // start longitude
            datacube_map.insert("longitudeP4", xml.readElementText());
            xml.readNextStartElement(); // start latitude
            datacube_map.insert("latitudeP4", xml.readElementText());
        }
    }
}

void xmlparser::addDatacubeToMap(QXmlStreamReader &xml, QMap<QString, QString> &map) const
{

    /* We need a start element, like <foo> */
    if (xml.tokenType() != QXmlStreamReader::StartElement) {
        return;
    }
    /* Let's read the name... */
    QString elementName = xml.name().toString();

    /* ...go to the next. */
    xml.readNext();
    /*
     * This elements needs to contain Characters so we know it's
     * actually data, if it's not we'll leave.
     */
    if (xml.tokenType() != QXmlStreamReader::Characters) {
        return;
    }
    /* Now we can add it to the map.*/
    map.insert(elementName, xml.text().toString());
}

QString xmlparser::extractPublisherDID_fits(QList<QMap<QString, QString>> &datacubes,
                                            QMap<QString, QString> &element)
{
    if (!datacubes.isEmpty()) {

        int bestOverlapIndex = 0;
        int bestOverlapCode = 6;

        int tmpIndex = 0;

        for (QList<QMap<QString, QString>>::iterator it = datacubes.begin(); it != datacubes.end();
             ++it) {

            QMap<QString, QString> datacube = *it;
            if (datacube.value("code").toDouble() < bestOverlapCode) {
                bestOverlapIndex = tmpIndex;
                bestOverlapCode = datacube.value("code").toDouble();
            }

            tmpIndex++;
        }

        QMap<QString, QString> datacube = datacubes.at(bestOverlapIndex);
        return datacube["PublisherDID"];
    }
    return "NULL";
}

QString xmlparser::extractPublisherDID(QList<QMap<QString, QString>> &datacubes,
                                       QMap<QString, QString> &element)
{

    while (!datacubes.isEmpty()) {
        QMap<QString, QString> datacube = datacubes.takeFirst();

        if (datacube["Survey"].compare(element["Survey"]) == 0
                && datacube["Species"].compare(element["Species"]) == 0
                && datacube["Transition"].compare(element["Transition"]) == 0) {
            return datacube["PublisherDID"];
        }
    }
    return "NULL";
}

QString xmlparser::extractURL(QList<QMap<QString, QString>> &datacubes)
{

    while (!datacubes.isEmpty()) {
        QMap<QString, QString> datacube = datacubes.takeFirst();
        if (datacube["URL"].compare("") != 0)
            return datacube["URL"];
    }
    return "NULL";
}

void xmlparser::addDatacubesToUI(QList<QMap<QString, QString>> &datacubes)
{

    while (!datacubes.isEmpty()) {

    }
}

void xmlparser::datacubeExtraction(QXmlStreamReader &xml_reader,
                                   QList<QMap<QString, QString>> &datacubes)
{


    while (!xml_reader.atEnd() && !xml_reader.hasError()) {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = xml_reader.readNext();

        /* If token is just StartDocument, we'll go to next.*/
        if (token == QXmlStreamReader::StartDocument) {
            continue;
        }

        /* If token is StartElement, we'll see if we can read it.*/
        if (token == QXmlStreamReader::StartElement) {
            if (xml_reader.name() == "results") {
                continue;
            }
            if (xml_reader.name() == "description") {
                continue;
            }
            if (xml_reader.name() == "input") {
                continue;
            }
            if (xml_reader.name() == "OriginFits") {
                continue;
            }

            if (xml_reader.name() == "datacube") {
                // datacubes.append(this->parseDatacube(xml_reader, stringa));
                QMap<QString, QString> datacube_map = this->parseDatacube(xml_reader);

                int test_flag_nanten = -1;
                if (datacube_map["PublisherDID"].compare("") != 0)
                    test_flag_nanten = datacube_map["PublisherDID"]
                            .split("_", QString::SkipEmptyParts)
                            .last()
                            .toInt();
                if (datacube_map["code"].compare("1") == 0
                        || datacube_map["code"].compare("6") == 0) {
                } else if ((datacube_map["Survey"].compare("NANTEN") == 0 && test_flag_nanten == 2)
                           || datacube_map["Survey"].compare("Cornish") == 0) {
                } else {
                    if (datacube_map["Survey"].compare("") == 0) {
                        datacube_map.insert("Survey", "Extinction_Map");
                        datacube_map.insert("Species", "dust");
                        datacube_map.insert("Transition", "visual");
                    }
                    datacubes.append(datacube_map);
                }
            }
        }
    }
    xml_reader.clear();
}
