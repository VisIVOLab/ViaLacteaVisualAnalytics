#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslError>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include "loadingwidget.h"

#include <stdio.h>


class QSslError;

class DownloadManager: public QObject
{
    Q_OBJECT
    QNetworkAccessManager *man;
    QList<QNetworkReply *> currentDownloads;

public:
    DownloadManager();
    QString doDownload(const QUrl &url, QString fn="");
    QString saveFileName(const QUrl &url, QString outputFile="");
    bool saveToDisk(const QString &filename, QIODevice *data);
    QString filenamePath;

private:
    LoadingWidget *loading;
    QString savedFilename;

public slots:
    void execute();
    void downloadFinished(QNetworkReply *reply);
    void sslErrors(const QList<QSslError> &errors);

signals:
    void downloadCompleted();
};


#endif // DOWNLOADMANAGER_H
