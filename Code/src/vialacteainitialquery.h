#ifndef VIALACTEAINITIALQUERY_H
#define VIALACTEAINITIALQUERY_H

#include "loadingwidget.h"
#include "VLKBInventoryTree.h"
#include "vtkwindow_new.h"
#include "xmlparser.h"

#include <QAuthenticator>
#include <QNetworkReply>
#include <QWidget>

namespace Ui {
class VialacteaInitialQuery;
}

class VialacteaInitialQuery : public QWidget
{
    Q_OBJECT

public:
    explicit VialacteaInitialQuery(QString fn = "", QWidget *parent = 0);
    ~VialacteaInitialQuery();
    void setL(QString l);
    void setB(QString b);
    void setR(QString r);
    void setDeltaRect(QString dl, QString db);

    void setSurveyname(QString s);
    void setSpecies(QString s) { species = s; }
    void setTransition(QString s);
    void setCallingVtkWindow(vtkwindow_new *v) { myCallingVtkWindow = v; }
    void setSelectedSurveyMap(QList<QPair<QString, QString>> s) { selectedSurvey = s; }

    static QString posCutoutString(double l, double b, double r);
    static QString posCutoutString(double l1, double l2, double b1, double b2);
    void cutoutRequest(const QString &id, const QDir &dir, const QString &pos,
                       const Cutout &src = Cutout());

    // POS=RANGE
    void searchRequest(double l, double b, double dl, double db);
    void cutoutRequest(const QString &id, const QDir &dir, double l1, double l2, double b1,
                       double b2, const Cutout &src = Cutout());

    // POS=CIRCLE
    void searchRequest(double l, double b, double r);
    void cutoutRequest(const QString &id, const QDir &dir, double l, double b, double r,
                       const Cutout &src = Cutout());

    void cutoutRequest(const QString &url, const QDir &dir, const Cutout &src = Cutout());

signals:
    void searchDone(QList<QMap<QString, QString>>);
    void searchDoneVO(const QByteArray &votable);
    void cutoutDone(const QString &filepath, const Cutout &src = Cutout());

private slots:
    void on_pushButton_clicked();
    void on_pointRadioButton_toggled(bool checked);
    void finishedSlot(QNetworkReply *reply);
    void onDownloadCompleted();
    void onAuthenticationRequired(QNetworkReply *reply, QAuthenticator *auth);

public slots:
    void on_queryPushButton_clicked();
    void cutoutRequest(QString url, QList<QMap<QString, QString>> el, int pos);
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
    QList<QMap<QString, QString>> elementsOnDb;
    QString species, transition, velfrom, velto;
    QMap<QString, QString> transitions;
    QList<QPair<QString, QString>> selectedSurvey;
    QWidget *p;
    QString pubdid;
    QString vlkbUrl;
    QString velocityUnit;
    vtkwindow_new *myCallingVtkWindow;
    bool isRadius;

    void searchRequest(const QString &url);
};

#endif // VIALACTEAINITIALQUERY_H
