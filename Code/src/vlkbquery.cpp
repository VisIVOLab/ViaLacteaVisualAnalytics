#include "vlkbquery.h"
#include "ui_vlkbquery.h"

#include "authkeys.h"
#include "authwrapper.h"
#include "sedvisualizerplot.h"
#include "singleton.h"
#include "vialactea_fileload.h"

#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QMessageBox>

VLKBQuery::VLKBQuery(QString q, vtkwindow_new *v, QString w, QWidget *parent, Qt::GlobalColor color)
    : QWidget(parent), ui(new Ui::VLKBQuery)
{
    ui->setupUi(this);

    if (parent != 0) {
        sd = dynamic_cast<SEDVisualizerPlot *>(parent);
    }

    m_sSettingsFile = QDir::homePath()
            .append(QDir::separator())
            .append("VisIVODesktopTemp")
            .append("/setting.ini");

    query = q; // QUrl::toPercentEncoding(q);

    loading = new LoadingWidget();
    loading->init();
    loading->setText("Querying VLKB..");
    loading->show();
    loading->activateWindow();

    vtkwin = v;
    what = w;
    modelColor = color;
    connectToVlkb();
}

void VLKBQuery::connectToVlkb()
{
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    url = settings.value("vlkbtableurl").toString();

    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(availReplyFinished(QNetworkReply *)));
    QUrl reqUrl(url + "/availability");
    QNetworkRequest req(reqUrl);

    IA2VlkbAuth::Instance().putAccessToken(req);
    QNetworkReply *reply = manager->get(req);
    loading->setLoadingProcess(reply);
}

void VLKBQuery::availReplyFinished(QNetworkReply *reply)
{
    if (reply->error()) {
        available = false;
    } else {
        QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
        QString vlkbtype = settings.value("vlkbtype", "ia2").toString();
        QString tag = "vosi:available";

        QDomDocument doc;
        doc.setContent(reply->readAll());
        QDomNodeList list = doc.elementsByTagName(tag);
        if (list.at(0).toElement().text() == "true") {

            available = true;
            executeQuery();
        }
    }
    reply->deleteLater();
}

void VLKBQuery::executeQuery()
{
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    manager = new QNetworkAccessManager(this);

    QByteArray postData;
    postData.append("REQUEST=doQuery&");
    postData.append("VERSION=1.0&");
    postData.append("LANG=ADQL&");
    postData.append("FORMAT=tsv&");
    postData.append("QUERY=" + QUrl::toPercentEncoding(query));

    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this,
            SLOT(onAuthenticationRequestSlot(QNetworkReply *, QAuthenticator *)));
    if (what.compare("bm") == 0)
        connect(manager, SIGNAL(finished(QNetworkReply *)), this,
                SLOT(queryReplyFinishedBM(QNetworkReply *)));
    else if (what.compare("model") == 0)
        connect(manager, SIGNAL(finished(QNetworkReply *)), this,
                SLOT(queryReplyFinishedModel(QNetworkReply *)));

    QNetworkRequest req(url + "/sync");
    IA2VlkbAuth::Instance().putAccessToken(req);
    QNetworkReply *reply = manager->post(req, postData);
    loading->setLoadingProcess(reply);
}

void VLKBQuery::executoSyncQuery()
{
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    QByteArray postData;

    postData.append("REQUEST=doQuery&");
    postData.append("VERSION=1.0&");
    postData.append("LANG=ADQL&");
    postData.append("FORMAT=tsv&");

    QNetworkAccessManager *networkMgr = new QNetworkAccessManager(this);
    connect(networkMgr, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this,
            SLOT(onAuthenticationRequestSlot(QNetworkReply *, QAuthenticator *)));

    QNetworkRequest req(url + "/sync?REQUEST=doQuery&VERSION=1.0&LANG=ADQL&FORMAT=tsv&QUERY="
                        + QUrl::toPercentEncoding(query));
    IA2VlkbAuth::Instance().putAccessToken(req);
    QNetworkReply *reply = networkMgr->get(req);
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));

    // Execute the event loop here, now we will wait here until readyRead() signal is emitted
    // which in turn will trigger event loop quit.
    loop.exec();
}

void VLKBQuery::onAuthenticationRequestSlot(QNetworkReply *aReply, QAuthenticator *aAuthenticator)
{
    Q_UNUSED(aReply);
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    if (settings.value("vlkbtype", "ia2").toString() == "ia2") {
        aAuthenticator->setUser(IA2_TAP_USER);
        aAuthenticator->setPassword(IA2_TAP_PASS);
    }
}

void VLKBQuery::queryReplyFinishedModel(QNetworkReply *reply)
{
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);

    if (reply->error()) {
    } else {
        QVariant possibleRedirectUrl =
                reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        /* We'll deduct if the redirection is valid in the redirectUrl function */
        urlRedirectedTo = redirectUrl(possibleRedirectUrl.toUrl(), urlRedirectedTo);

        /* If the URL is not empty, we're being redirected. */
        if (!urlRedirectedTo.isEmpty()) {
            /* We'll do another request to the redirection url. */
            QNetworkRequest req(urlRedirectedTo);
            IA2VlkbAuth::Instance().putAccessToken(req);
            manager->get(req);
        } else {

            QByteArray bytes = reply->readAll();

            loading->loadingEnded();
            loading->hide();

            QVector<QStringList> headerAndValueList;
            QList<QByteArray> lines = bytes.split('\r\n');

            foreach (const QByteArray &line, lines) {
                QString myString(line);
                headerAndValueList.append(myString.split("\t"));
            }
            sd->setModelFitValue(headerAndValueList, modelColor);
        }
        /* Clean up. */
        reply->deleteLater();
    }
}

void VLKBQuery::queryReplyFinishedBM(QNetworkReply *reply)
{
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    if (reply->error()) {
    } else {

        QVariant possibleRedirectUrl =
                reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        /* We'll deduct if the redirection is valid in the redirectUrl function */
        urlRedirectedTo = redirectUrl(possibleRedirectUrl.toUrl(), urlRedirectedTo);
        /* If the URL is not empty, we're being redirected. */
        if (!urlRedirectedTo.isEmpty()) {
            /* We'll do another request to the redirection url. */
            QNetworkRequest req(urlRedirectedTo);
            IA2VlkbAuth::Instance().putAccessToken(req);
            manager->get(req);
        } else {

            QByteArray bytes = reply->readAll();
            QString s_data = QString::fromLatin1(bytes.data());
            // se inizia per < è un xml, se è xml c'e' stato un errore
            if (QString::compare(s_data.at(0), "<", Qt::CaseInsensitive) == 0) {
                QMessageBox::critical(this, "Error", "Error: \n" + bytes);
            }
            // Fare un controllo più serio, su quando non ci sono sed restituite
            else if (bytes.size() != 1187) {
                QString output_file =
                        QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp");
                output_file.append("/tmp_download/").append("test_bm").append(".dat");

                QFile file(output_file);
                file.open(QIODevice::WriteOnly);
                file.write(bytes);
                file.close();

                Vialactea_FileLoad *fileload = new Vialactea_FileLoad(output_file, vtkwin);
                fileload->importBandMerged();
            } else {
                QMessageBox::information(this, "Alert", "No SED");
            }
            loading->loadingEnded();
            loading->hide();
        }
        /* Clean up. */
        reply->deleteLater();
    }
}

QUrl VLKBQuery::redirectUrl(const QUrl &possibleRedirectUrl, const QUrl &oldRedirectUrl) const
{
    QUrl redirectUrl;
    /*
     * Check if the URL is empty and
     * that we aren't being fooled into a infinite redirect loop.
     * We could also keep track of how many redirects we have been to
     * and set a limit to it, but we'll leave that to you.
     */
    if (!possibleRedirectUrl.isEmpty() && possibleRedirectUrl != oldRedirectUrl) {
        redirectUrl = possibleRedirectUrl;
    }

    return redirectUrl;
}

VLKBQuery::~VLKBQuery()
{
    delete ui;
}
