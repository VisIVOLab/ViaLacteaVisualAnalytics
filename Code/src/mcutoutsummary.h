#ifndef MCUTOUTSUMMARY_H
#define MCUTOUTSUMMARY_H

#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QSettings>
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

signals:
    void jobSubmitted(const QString &jobId);
    void jobCompleted(const QString &jobId);

private slots:
    void startJob(const QString &jobId);
    void pollJob(const QString &jobId);
    void getJobReport(const QString &jobId);

    void on_tableSummary_itemClicked(QTableWidgetItem *item);
    void on_btnSendRequest_clicked();

private:
    Ui::MCutoutSummary *ui;
    QNetworkAccessManager *nam;

    QSettings settings;
    QString mcutoutEndpoint;
    int pollTimeout;
    QJsonArray requestBody;
    QStringList responseContents;
    QUrl downloadUrl;

    void createRequestBody(const QStringList &cutouts);
    void initSummaryTable();
    void submitJob();
    void downloadArchive(const QString &absolutePath);
    void parseXmlResponse(QNetworkReply *body);
};

#endif // MCUTOUTSUMMARY_H
