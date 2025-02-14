#ifndef MCUTOUTSUMMARY_H
#define MCUTOUTSUMMARY_H

#include "VLKBInventoryTree.h"

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
    explicit MCutoutSummary(QWidget *parent, const QList<Cutout> &cutouts);
    explicit MCutoutSummary(QWidget *parent, const QString &pendingFile);
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

    void saveStatus(const QString &jobId);

private:
    explicit MCutoutSummary(QWidget *parent);

    Ui::MCutoutSummary *ui;
    QList<Cutout> cutouts;

    QNetworkAccessManager *nam;

    QSettings settings;
    QString mcutoutEndpoint;
    int pollTimeout;
    QJsonArray requestBody;
    QStringList responseContents;
    QUrl downloadUrl;
    QString pendingFile;

    void createRequestBody(const QList<Cutout> &cutouts);
    void initSummaryTable();
    void submitJob();
    void downloadArchive(const QString &absolutePath);
    void parseJobReport(QNetworkReply *body);
};

#endif // MCUTOUTSUMMARY_H
