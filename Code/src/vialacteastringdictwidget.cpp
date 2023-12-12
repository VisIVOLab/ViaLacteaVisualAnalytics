#include "vialacteastringdictwidget.h"
#include "ui_vialacteastringdictwidget.h"

#include <QAuthenticator>
#include <QDir>
#include <QDomDocument>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>

#include "authkeys.h"
#include "authwrapper.h"
#include "singleton.h"

VialacteaStringDictWidget::VialacteaStringDictWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::VialacteaStringDictWidget)
{
    ui->setupUi(this);
}

void VialacteaStringDictWidget::clearDict()
{
    tableDescStringDict.clear();
    tableUtypeStringDict.clear();
    colDescStringDict.clear();
    colUtypeStringDict.clear();
}

void VialacteaStringDictWidget::buildDict()
{
    m_sSettingsFile = QDir::homePath()
                              .append(QDir::separator())
                              .append("VisIVODesktopTemp")
                              .append(QDir::separator())
                              .append("setting.ini");

    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(availReplyFinished(QNetworkReply *)));

    QNetworkRequest request(QUrl(settings.value("vlkbtableurl", "").toString() + "/availability"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("text/html; charset=utf-8"));

    IA2VlkbAuth::Instance().putAccessToken(request);
    manager->get(request);
}

void VialacteaStringDictWidget::availReplyFinished(QNetworkReply *reply)
{
    if (reply->error()) {
        QMessageBox::critical(this, "Error", "Error: \n" + reply->errorString());
    } else {
        QSettings settings(m_sSettingsFile, QSettings::IniFormat);
        QString vlkbtype = settings.value("vlkbtype", "ia2").toString();
        QString tag = "vosi:available";

        QDomDocument doc;
        doc.setContent(reply->readAll());
        QDomNodeList list = doc.elementsByTagName(tag);

        if (list.at(0).toElement().text() == "true") {
            clearDict();
            executeQueryTapSchemaTables();
            executeQueryTapSchemaColumns();
        }
    }

    reply->deleteLater();
}

void VialacteaStringDictWidget::executeQueryTapSchemaTables()
{

    QSettings settings(m_sSettingsFile, QSettings::IniFormat);

    QString query = "SELECT * FROM TAP_SCHEMA.tables";

    manager = new QNetworkAccessManager(this);

    QByteArray postData;

    postData.append("REQUEST=doQuery&");
    postData.append("VERSION=1.0&");
    postData.append("LANG=ADQL&");
    postData.append("FORMAT=tsv&");

    postData.append("QUERY=" + QUrl::toPercentEncoding(query));

    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this,
            SLOT(onAuthenticationRequestSlot(QNetworkReply *, QAuthenticator *)));

    connect(manager, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(queryReplyFinishedTapSchemaTables(QNetworkReply *)));

    QString url = settings.value("vlkbtableurl", "").toString() + "/sync";
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/x-www-form-urlencoded"));

    IA2VlkbAuth::Instance().putAccessToken(request);

    manager->post(request, postData);
}

void VialacteaStringDictWidget::executeQueryTapSchemaColumns()
{

    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    QString query = "SELECT * FROM TAP_SCHEMA.columns";
    manager = new QNetworkAccessManager(this);
    QByteArray postData;
    postData.append("REQUEST=doQuery&");
    postData.append("VERSION=1.0&");
    postData.append("LANG=ADQL&");
    postData.append("FORMAT=tsv&");

    postData.append("QUERY=" + QUrl::toPercentEncoding(query));

    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this,
            SLOT(onAuthenticationRequestSlot(QNetworkReply *, QAuthenticator *)));

    connect(manager, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(queryReplyFinishedTapSchemaColumns(QNetworkReply *)));

    QString url = settings.value("vlkbtableurl", "").toString() + "/sync";
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/x-www-form-urlencoded"));

    IA2VlkbAuth::Instance().putAccessToken(request);
    manager->post(request, postData);
}

void VialacteaStringDictWidget::queryReplyFinishedTapSchemaTables(QNetworkReply *reply)
{
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    if (reply->error()) {
        QMessageBox::critical(this, "Error", "Error: \n" + reply->errorString());
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
            QByteArray bytes = reply->readLine();
            QString s_data = QString::fromLatin1(bytes.data());
            // se inizia per '<' è un xml, se è xml c'e' stato un errore
            if (QString::compare(s_data.at(0), "<", Qt::CaseInsensitive) == 0) {
                QMessageBox::critical(this, "Error", "Error: \n" + bytes);
            } else {
                int table_name, description, utype;
                QStringList columns = s_data.split('\t');

                for (int i = 0; i < columns.length(); ++i) {
                    QString col = columns[i].remove(QRegExp("[\n\t\r]"));
                    if (col == "table_name")
                        table_name = i;
                    if (col == "description")
                        description = i;
                    if (col == "utype")
                        utype = i;
                }
                QString line_data;
                while (!reply->atEnd()) {
                    line_data = QString::fromLatin1(reply->readLine().data());
                    QStringList list2 = line_data.split('\t');
                    tableUtypeStringDict.insert(list2[table_name],
                                                list2[utype].remove(QRegExp("[\n\t\r]")));
                    tableDescStringDict.insert(list2[table_name],
                                               list2[description].remove(QRegExp("[\n\t\r]")));
                }
            }
        }
        /* Clean up. */
        reply->deleteLater();
    }
}

void VialacteaStringDictWidget::queryReplyFinishedTapSchemaColumns(QNetworkReply *reply)
{
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    if (reply->error()) {
        QMessageBox::critical(this, "Error", "Error: \n" + reply->errorString());
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

            QByteArray bytes = reply->readLine();
            QString s_data = QString::fromLatin1(bytes.data());
            // se inizia per '<' è un xml, se è xml c'e' stato un errore
            if (QString::compare(s_data.at(0), "<", Qt::CaseInsensitive) == 0) {
                QMessageBox::critical(this, "Error", "Error: \n" + bytes);
            } else {
                int table_name, column_name, description, utype;
                QStringList columns = s_data.split('\t');

                for (int i = 0; i < columns.length(); ++i) {
                    QString col = columns[i].remove(QRegExp("[\n\t\r]"));
                    if (col == "table_name")
                        table_name = i;
                    if (col == "column_name")
                        column_name = i;
                    if (col == "description")
                        description = i;
                    if (col == "utype")
                        utype = i;
                }

                QString line_data;
                while (!reply->atEnd()) {
                    line_data = QString::fromLatin1(reply->readLine().data());
                    QStringList list2 = line_data.split('\t');
                    colUtypeStringDict.insert(list2[table_name] + "." + list2[column_name],
                                              list2[utype].remove(QRegExp("[\n\t\r]")));
                    colDescStringDict.insert(list2[table_name] + "." + list2[column_name],
                                             list2[description].remove(QRegExp("[\n\t\r]")));
                }
            }
        }
        reply->deleteLater();
    }
}

QUrl VialacteaStringDictWidget::redirectUrl(const QUrl &possibleRedirectUrl,
                                            const QUrl &oldRedirectUrl) const
{
    QUrl redirectUrl;

    if (!possibleRedirectUrl.isEmpty() && possibleRedirectUrl != oldRedirectUrl) {
        redirectUrl = possibleRedirectUrl;
    }

    return redirectUrl;
}

void VialacteaStringDictWidget::onAuthenticationRequestSlot(QNetworkReply *aReply,
                                                            QAuthenticator *aAuthenticator)
{
    Q_UNUSED(aReply);
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    if (settings.value("vlkbtype", "ia2").toString() == "ia2") {
        aAuthenticator->setUser(IA2_TAP_USER);
        aAuthenticator->setPassword(IA2_TAP_PASS);
    }
}

VialacteaStringDictWidget::~VialacteaStringDictWidget()
{
    delete ui;
}
