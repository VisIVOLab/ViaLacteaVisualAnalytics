#include "BackendClient.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

BackendClient::BackendClient(QString baseUrl) : m_baseUrl(std::move(baseUrl))
{
}

QString BackendClient::baseUrl() const
{
    return this->m_baseUrl;
}

BackendHealthResult BackendClient::health() const
{
    BackendHealthResult result;
    QString error;
    const QByteArray payload = this->performGet(QUrl(this->m_baseUrl + "/health"), error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }

    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.ok = object.value("ok").toBool(false);
    if (!result.ok) {
        result.error = QStringLiteral("Backend health check failed.");
    }
    return result;
}

BackendListFilesResult BackendClient::listFiles(const QString &path) const
{
    BackendListFilesResult result;
    QString error;
    QUrl url(this->m_baseUrl + "/files/list");
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("path"), path);
    url.setQuery(query);
    const QByteArray payload = this->performGet(url, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }

    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value("valid").toBool(false);
    result.error = object.value("error").toString();
    const QJsonArray entries = object.value("entries").toArray();
    result.entries.reserve(static_cast<std::size_t>(entries.size()));
    for (const QJsonValue &value : entries) {
        const QJsonObject entryObject = value.toObject();
        result.entries.push_back({ entryObject.value("name").toString(),
                                   entryObject.value("path").toString(),
                                   entryObject.value("type").toString() });
    }

    return result;
}

BackendOpenDatasetResult BackendClient::openDataset(const QString &path) const
{
    BackendOpenDatasetResult result;
    QString error;
    const QJsonDocument body(QJsonObject { { QStringLiteral("path"), path } });
    const QByteArray payload =
            this->performPost(QUrl(this->m_baseUrl + "/datasets/open"), body, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }

    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value("valid").toBool(false);
    result.error = object.value("error").toString();
    result.datasetId = object.value("dataset_id").toString();
    result.kind = object.value("kind").toString();
    return result;
}

BackendMomentResult BackendClient::requestMoment(const QString &datasetId, int order) const
{
    BackendMomentResult result;
    QString error;
    const QJsonDocument body(QJsonObject { { QStringLiteral("dataset_id"), datasetId },
                                           { QStringLiteral("moment_order"), order } });
    const QByteArray payload =
            this->performPost(QUrl(this->m_baseUrl + "/products/moment"), body, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }

    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value("valid").toBool(false);
    result.error = object.value("error").toString();
    result.width = object.value("width").toInt();
    result.height = object.value("height").toInt();
    result.scalarType = object.value("scalar_type").toString();
    result.rangeMin = object.value("range_min").toDouble();
    result.rangeMax = object.value("range_max").toDouble();
    result.data = QByteArray::fromBase64(object.value("data_base64").toString().toUtf8());
    return result;
}

QByteArray BackendClient::performGet(const QUrl &url, QString &error) const
{
    QNetworkAccessManager nam;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QEventLoop loop;
    QNetworkReply *reply = nam.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const QByteArray data = reply->readAll();
    if (reply->error() != QNetworkReply::NoError) {
        error = reply->errorString();
    } else {
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status >= 400) {
            error = QString::fromUtf8(data);
        }
    }

    reply->deleteLater();
    return data;
}

QByteArray BackendClient::performPost(const QUrl &url, const QJsonDocument &body, QString &error) const
{
    QNetworkAccessManager nam;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QEventLoop loop;
    QNetworkReply *reply = nam.post(request, body.toJson(QJsonDocument::Compact));
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const QByteArray data = reply->readAll();
    if (reply->error() != QNetworkReply::NoError) {
        error = reply->errorString();
    } else {
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status >= 400) {
            error = QString::fromUtf8(data);
        }
    }

    reply->deleteLater();
    return data;
}
