#ifndef VIALACTEAINITIALQUERY_H
#define VIALACTEAINITIALQUERY_H

#include <QWidget>
#include <QNetworkReply>
#include "xmlparser.h"
#include "loadingwidget.h"
#include "vtkwindow_new.h"

namespace Ui {
class VialacteaInitialQuery;
}

class VialacteaInitialQuery : public QWidget
{
    Q_OBJECT

public:
    explicit VialacteaInitialQuery(QString fn="",QWidget *parent = 0);
    ~VialacteaInitialQuery();
    void setL(QString l);
    void setB(QString b);
    void setR(QString r);
    void setDeltaRect(QString dl,QString db);

    void setSurveyname(QString s);
    void setSpecies(QString s){species=s;}
    void setTransition(QString s);
    void setCallingVtkWindow(vtkwindow_new *v){myCallingVtkWindow=v;}
    void setSelectedSurveyMap(QList < QPair<QString, QString> > s){selectedSurvey=s;}


private slots:
    void on_pushButton_clicked();
    void on_pointRadioButton_toggled(bool checked);
    void finishedSlot(QNetworkReply* reply);
    void on_download_completed();

public slots:
    void on_queryPushButton_clicked();
   // void cutoutRequest(QUrl url, QList< QMap<QString,QString> > el =QList< QMap<QString,QString> >(), int pos=0);
    void cutoutRequest(QString url, QList< QMap<QString,QString> > el, int pos);
    void selectedStartingLayersRequest(QUrl url);

private:
    Ui::VialacteaInitialQuery *ui;
    QString surveyname;
    xmlparser *parser;
    LoadingWidget *loading;
    QNetworkAccessManager *nam;
    QString downloadedFile;
    QString currentPath;
    QString outputFile;
    QString descriptionFromDB;
    QList< QMap<QString,QString> > elementsOnDb;
    QString species, transition,velfrom,velto;
    QMap<QString,QString> transitions;
    QList < QPair<QString, QString> > selectedSurvey;
    QString url_prefix;


    QWidget *p;
    QString pubdid;
    QString vlkbUrl;
    QString vlkbsearchUrl;
    QString vlkbcutoutUrl;
    QString velocityUnit;
    vtkwindow_new *myCallingVtkWindow;
    bool isRadius;
};

#endif // VIALACTEAINITIALQUERY_H
