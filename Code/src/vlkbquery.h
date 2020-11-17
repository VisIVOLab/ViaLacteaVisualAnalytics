#ifndef VLKBQUERY_H
#define VLKBQUERY_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAuthenticator>
#include "loadingwidget.h"
#include "sedvisualizerplot.h"

namespace Ui {
class VLKBQuery;
}

class VLKBQuery : public QWidget
{
    Q_OBJECT

public:
    explicit VLKBQuery(QString q, vtkwindow_new *v, QString w="bm", QWidget *parent = 0, Qt::GlobalColor color = Qt::blue);
    ~VLKBQuery();

private:
    Ui::VLKBQuery *ui;
    QString query;
    QString url;
    QNetworkAccessManager *manager;
    void connectToVlkb();
    bool available;
    void executeQuery();
    void on_queryPushButton_clicked();
    QUrl redirectUrl(const QUrl& possibleRedirectUrl, const QUrl& oldRedirectUrl) const;
    QUrl urlRedirectedTo;
    void executoSyncQuery();
    LoadingWidget *loading;
    vtkwindow_new *vtkwin;
    QString what;
    SEDVisualizerPlot *sd;
    Qt::GlobalColor modelColor;


private slots:
    void availReplyFinished (QNetworkReply *reply);
    //   void queryReplyFinished (QNetworkReply *reply);
    void queryReplyFinishedBM (QNetworkReply *reply);
    void queryReplyFinishedModel (QNetworkReply *reply);
    void onAuthenticationRequestSlot(QNetworkReply *aReply, QAuthenticator *aAuthenticator);

};

#endif // VLKBQUERY_H
