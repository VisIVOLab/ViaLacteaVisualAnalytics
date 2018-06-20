#include <QCoreApplication>
#include <QDebug>

#include "downloadmanager.h"
#include "dcvisualizer.h"
#include "loadingwidget.h"
#include <QDir>
#include <QMessageBox>

// constructor
DownloadManager::DownloadManager()
{
    qDebug()<<"Download manager Constructor";

    man = new QNetworkAccessManager(this);
    connect(man, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(downloadFinished(QNetworkReply*)));

    loading = new LoadingWidget();

}

void DownloadManager::execute()
{
    /*
    QString urlString="http://ia2-vo.oats.inaf.it/vialactea/cutouts/vlkb-cutout_2015-03-12_18-29-49_584568_NANTEN_L291_L279_hup.fits";
    QUrl url = QUrl::fromEncoded(urlString.toLocal8Bit());
    // makes a request
    doDownload(url);
*/
}

// Constructs a QList of QNetworkReply
QString DownloadManager::doDownload(const QUrl &url, QString fn)
{

    qDebug()<<" ********************************** FN: "<<fn;
    loading ->show();
    loading->activateWindow();
    loading->setFileName("Downloading...");

    savedFilename=fn;
    QNetworkRequest request(url);
    QNetworkReply *reply = man->get(request);

    qDebug()<<"doDownload, request:"<<request.url()<<" and saving to: "<<savedFilename;
#ifndef QT_NO_SSL
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrors(QList<QSslError>)));
#endif

    // List of reply
    currentDownloads.append(reply);
    QUrl myurl = reply->url();
    
    return saveFileName(myurl,savedFilename);
}

QString DownloadManager::saveFileName(const QUrl &url,QString outputFile )
{
    if (outputFile=="")
    {
        //    m_sSettingsFile = QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp").append("/setting.ini");

        //  QString path = QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp").append
        QString path = url.path();
        QString basename =QFileInfo(path).fileName() ;
        basename=  QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp").append(QDir::separator()).append("tmp_download").append(QDir::separator()).append( basename);

        qDebug()<<"QFileInfo(path):"<<basename;

        if (basename.isEmpty())
            basename = "download";

        if (QFile::exists(basename)) {
            // already exists, don't overwrite
            int i = 0;
            basename += '.';
            while (QFile::exists(basename + QString::number(i)))
                ++i;

            basename += QString::number(i);
        }

        return basename;
    }
    else
        return outputFile ;
}

void DownloadManager::downloadFinished(QNetworkReply *reply)
{
    qDebug()<<"Download Finished: "<<reply->url().toString();
    
    QUrl url = reply->url();
    if (reply->error()) {
        qDebug()<<"Download of"<< url.toEncoded().constData()<<"failed: "<<qPrintable(reply->errorString());

        QMessageBox::critical(loading ,"Error", qPrintable(reply->errorString()));

        //fprintf(stderr, "Download of %s failed: %s\n",url.toEncoded().constData(),qPrintable(reply->errorString()));
    } else {

        filenamePath = saveFileName(url,savedFilename);
        if (saveToDisk(filenamePath, reply))
            qDebug()<<"Download of"<<url.toEncoded().constData()<<" succeeded and saved to"<<qPrintable(filenamePath);

        emit downloadCompleted();

    }

    currentDownloads.removeAll(reply);
    reply->deleteLater();
    loading->hide();

}

bool DownloadManager::saveToDisk(const QString &filename, QIODevice *reply)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug()<<"Could not open";
        fprintf(stderr, "Could not open %s for writing: %s\n",
                qPrintable(filename),
                qPrintable(file.errorString()));
        return false;
    }

    file.write(reply->readAll());
    file.close();


    // dcvisualizer *datacubeVisualizer=new dcvisualizer();
    //datacubeVisualizer->visualize(filename);
    return true;
}


void DownloadManager::sslErrors(const QList<QSslError> &sslErrors)
{
#ifndef QT_NO_SSL
    foreach (const QSslError &error, sslErrors)
        fprintf(stderr, "SSL error: %s\n", qPrintable(error.errorString()));
#else
    Q_UNUSED(sslErrors);
#endif
}

