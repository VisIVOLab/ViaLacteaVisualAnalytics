#ifndef MCUTOUTSUMMARY_H
#define MCUTOUTSUMMARY_H

#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QTableWidgetItem>
#include <QWidget>

namespace Ui {
class MCutoutSummary;
}

class MCutoutSummary : public QWidget
{
    Q_OBJECT

public:
    explicit MCutoutSummary(QWidget *parent, const QStringList &cutouts);
    ~MCutoutSummary();

private slots:
    void on_tableSummary_itemClicked(QTableWidgetItem *item);
    void on_btnSendRequest_clicked();

private:
    Ui::MCutoutSummary *ui;
    QNetworkAccessManager *nam;

    QString mcutoutEndpoint;
    QJsonArray requestBody;
    QStringList responseContents;
    QUrl downloadUrl;

    void createRequestBody(const QStringList &cutouts);
    void initSummaryTable();
    void sendRequest();
    void downloadArchive(const QString &absolutePath);
    void parseXmlResponse(QNetworkReply *body);
};

#endif // MCUTOUTSUMMARY_H
