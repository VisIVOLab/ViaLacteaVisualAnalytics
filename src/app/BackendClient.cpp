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
    result.width = object.value("width").toInt();
    result.height = object.value("height").toInt();
    result.depth = object.value("depth").toInt();
    const QJsonArray spacing = object.value("spacing").toArray();
    const QJsonArray origin = object.value("origin").toArray();
    for (int i = 0; i < 3; ++i) {
        if (i < spacing.size()) {
            result.spacing[static_cast<std::size_t>(i)] = spacing.at(i).toDouble(1.0);
        }
        if (i < origin.size()) {
            result.origin[static_cast<std::size_t>(i)] = origin.at(i).toDouble(0.0);
        }
    }
    return result;
}

BackendCubePreviewResult BackendClient::requestPreview(const QString &datasetId, int downsample) const
{
    BackendCubePreviewResult result;
    QString error;
    const QJsonDocument body(QJsonObject { { QStringLiteral("dataset_id"), datasetId },
                                           { QStringLiteral("downsample"), downsample } });
    const QByteArray payload = this->performPost(QUrl(this->m_baseUrl + "/cube/preview"), body, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }

    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value("valid").toBool(false);
    result.error = object.value("error").toString();
    result.width = object.value("width").toInt();
    result.height = object.value("height").toInt();
    result.depth = object.value("depth").toInt();
    result.scalarType = object.value("scalar_type").toString();
    result.rangeMin = object.value("range_min").toDouble();
    result.rangeMax = object.value("range_max").toDouble();
    result.data = QByteArray::fromBase64(object.value("data_base64").toString().toUtf8());
    return result;
}

BackendCubeSliceResult BackendClient::requestSlice(const QString &datasetId, const QString &axis,
                                                   int index) const
{
    BackendCubeSliceResult result;
    QString error;
    const QJsonDocument body(QJsonObject { { QStringLiteral("dataset_id"), datasetId },
                                           { QStringLiteral("axis"), axis },
                                           { QStringLiteral("index"), index } });
    const QByteArray payload = this->performPost(QUrl(this->m_baseUrl + "/cube/slice"), body, error);
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

BackendCubeSubvolumeResult BackendClient::requestSubvolume(const QString &datasetId, int x0, int x1,
                                                           int y0, int y1, int z0, int z1) const
{
    BackendCubeSubvolumeResult result;
    QString error;
    const QJsonDocument body(QJsonObject { { QStringLiteral("dataset_id"), datasetId },
                                           { QStringLiteral("x0"), x0 },
                                           { QStringLiteral("x1"), x1 },
                                           { QStringLiteral("y0"), y0 },
                                           { QStringLiteral("y1"), y1 },
                                           { QStringLiteral("z0"), z0 },
                                           { QStringLiteral("z1"), z1 } });
    const QByteArray payload =
            this->performPost(QUrl(this->m_baseUrl + "/cube/subvolume"), body, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }

    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value("valid").toBool(false);
    result.error = object.value("error").toString();
    result.width = object.value("width").toInt();
    result.height = object.value("height").toInt();
    result.depth = object.value("depth").toInt();
    result.scalarType = object.value("scalar_type").toString();
    result.data = QByteArray::fromBase64(object.value("data_base64").toString().toUtf8());
    return result;
}

BackendImageResult BackendClient::requestImage(const QString &datasetId) const
{
    BackendImageResult result;
    QString error;
    const QJsonDocument body(QJsonObject { { QStringLiteral("dataset_id"), datasetId } });
    const QByteArray payload =
            this->performPost(QUrl(this->m_baseUrl + "/image/full"), body, error);
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
    result.data = QByteArray::fromBase64(object.value("data_base64").toString().toUtf8());
    return result;
}

BackendIsosurfaceResult BackendClient::requestIsosurface(const QString &datasetId,
                                                         double threshold) const
{
    BackendIsosurfaceResult result;
    QString error;
    const QJsonDocument body(QJsonObject { { QStringLiteral("dataset_id"), datasetId },
                                           { QStringLiteral("threshold"), threshold } });
    const QByteArray payload =
            this->performPost(QUrl(this->m_baseUrl + "/products/isosurface"), body, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }

    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value("valid").toBool(false);
    result.error = object.value("error").toString();
    result.numPoints = object.value("num_points").toInt();
    result.numPolys = object.value("num_polys").toInt();
    result.pointsData =
            QByteArray::fromBase64(object.value("points_base64").toString().toUtf8());
    result.polysData = QByteArray::fromBase64(object.value("polys_base64").toString().toUtf8());
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
