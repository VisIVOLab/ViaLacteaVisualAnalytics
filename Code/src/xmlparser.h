#ifndef XMLPARSER_H
#define XMLPARSER_H
#include <QDialog>
#include <QPointer>
#include <QMainWindow>
#include <QWidget>

#include <QBoxLayout>
#include <QXmlStreamReader>
#include <QWidget>
#include <QModelIndex>

class xmlparser
{

public:
    xmlparser();
    ~xmlparser();
    void parseXML(QXmlStreamReader& xml_reader, QString &stringa, bool dc=true);
    QList< QMap<QString,QString> > parseXmlAndGetList(QXmlStreamReader & xml_reader);

    void datacubeExtraction(QXmlStreamReader& xml_reader, QList< QMap<QString,QString> > &datacubes);
    QMap<QString, QString> datacube_element;
    void parseXML_fitsDownload(QXmlStreamReader & xml_reader, QString &stringa);

private slots:
    void parseSpectralCoordinate(QXmlStreamReader& xml, QMap<QString, QString> &datacube_map);
    void parseOverlap(QXmlStreamReader& xml, QMap<QString, QString> &datacube_map);
    void parseVelocity(QXmlStreamReader& xml);

private:

    QString queryUrl;
    QPointer<QVBoxLayout> _layout;
QString velocity;
    void setupUI();
    //QMap<QString, QString> parseDatacube(QXmlStreamReader& xml, QString &stringa);
    QMap<QString, QString> parseDatacube(QXmlStreamReader& xml);
    void addDatacubeToMap(QXmlStreamReader& xml,
                             QMap<QString, QString>& map) const;

    void addDatacubesToUI(QList< QMap<QString,QString> >& datacubes);

    QString extractPublisherDID_fits(QList< QMap<QString,QString> >& datacubes, QMap<QString,QString>& element);
    QString extractPublisherDID(QList< QMap<QString,QString> >& datacubes, QMap<QString,QString>& element);
    QString extractURL(QList< QMap<QString,QString> >& datacubes);
    void parseBounds(QXmlStreamReader& xml, QMap<QString, QString> &datacube_map);
    QMap<QString, QString> parseSurvey(QXmlStreamReader& xml) ;


};
#endif // XMLPARSER_H
