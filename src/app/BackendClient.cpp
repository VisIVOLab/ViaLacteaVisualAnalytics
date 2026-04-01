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

namespace {
QByteArray decodeCompressedPayload(const QByteArray &encoded, const QString &compression, QString &error)
{
    const QByteArray raw = QByteArray::fromBase64(encoded);
    if (compression.isEmpty() || compression == QStringLiteral("none")) {
        return raw;
    }

    if (compression == QStringLiteral("qt-zlib")) {
        const QByteArray uncompressed = qUncompress(raw);
        if (uncompressed.isEmpty()) {
            error = QStringLiteral("Failed to decompress backend payload.");
        }
        return uncompressed;
    }

    error = QStringLiteral("Unsupported backend compression: %1").arg(compression);
    return {};
}
}

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
    result.currentPath = object.value("current_path").toString();
    const QJsonArray entries = object.value("entries").toArray();
    result.entries.reserve(static_cast<std::size_t>(entries.size()));
    for (const QJsonValue &value : entries) {
        const QJsonObject entryObject = value.toObject();
        result.entries.push_back({ entryObject.value("name").toString(),
                                   entryObject.value("path").toString(),
                                   entryObject.value("type").toString(),
                                   entryObject.value("size").toInteger(),
                                   entryObject.value("modified_time").toString(),
                                   entryObject.value("is_fits").toBool(false) });
    }

    return result;
}

BackendFileHeaderResult BackendClient::fileHeader(const QString &path) const
{
    BackendFileHeaderResult result;
    QString error;
    const QJsonDocument body(QJsonObject { { QStringLiteral("path"), path } });
    const QByteArray payload =
            this->performPost(QUrl(this->m_baseUrl + "/files/header"), body, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }

    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value("valid").toBool(false);
    result.error = object.value("error").toString();
    const QJsonArray cards = object.value("cards").toArray();
    result.cards.reserve(cards.size());
    for (const QJsonValue &value : cards) {
        result.cards.push_back(value.toString());
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
    const QJsonArray ctype = object.value("ctype").toArray();
    const QJsonArray cunit = object.value("cunit").toArray();
    const QJsonArray crval = object.value("crval").toArray();
    const QJsonArray crpix = object.value("crpix").toArray();
    const QJsonArray cdelt = object.value("cdelt").toArray();
    for (int i = 0; i < 3; ++i) {
        if (i < spacing.size()) {
            result.spacing[static_cast<std::size_t>(i)] = spacing.at(i).toDouble(1.0);
        }
        if (i < origin.size()) {
            result.origin[static_cast<std::size_t>(i)] = origin.at(i).toDouble(0.0);
        }
        if (i < ctype.size()) {
            result.ctype[static_cast<std::size_t>(i)] = ctype.at(i).toString();
        }
        if (i < cunit.size()) {
            result.cunit[static_cast<std::size_t>(i)] = cunit.at(i).toString();
        }
        if (i < crval.size()) {
            result.crval[static_cast<std::size_t>(i)] = crval.at(i).toDouble(0.0);
        }
        if (i < crpix.size()) {
            result.crpix[static_cast<std::size_t>(i)] = crpix.at(i).toDouble(1.0);
        }
        if (i < cdelt.size()) {
            result.cdelt[static_cast<std::size_t>(i)] = cdelt.at(i).toDouble(1.0);
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
    result.data = this->decodePayload(object, QStringLiteral("data_base64"), QStringLiteral("compression"),
                                      result.error);
    if (!result.error.isEmpty()) {
        result.valid = false;
    }
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
    result.data = this->decodePayload(object, QStringLiteral("data_base64"), QStringLiteral("compression"),
                                      result.error);
    if (!result.error.isEmpty()) {
        result.valid = false;
    }
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
    result.data = this->decodePayload(object, QStringLiteral("data_base64"), QStringLiteral("compression"),
                                      result.error);
    if (!result.error.isEmpty()) {
        result.valid = false;
    }
    return result;
}

BackendCubePvResult BackendClient::requestPv(const QString &datasetId,
                                             const std::vector<std::array<int, 2>> &vertices,
                                             int widthPixels) const
{
    BackendCubePvResult result;
    QString error;
    QJsonArray vertexArray;
    for (const auto &vertex : vertices) {
        vertexArray.append(QJsonArray{ vertex[0], vertex[1] });
    }

    const QJsonDocument body(QJsonObject{ { QStringLiteral("dataset_id"), datasetId },
                                          { QStringLiteral("vertices"), vertexArray },
                                          { QStringLiteral("width_pixels"), widthPixels } });
    const QByteArray payload = this->performPost(QUrl(this->m_baseUrl + "/cube/pv"), body, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }

    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value("valid").toBool(false);
    result.error = object.value("error").toString();
    result.numSamples = object.value("num_samples").toInt();
    result.depth = object.value("depth").toInt();
    result.scalarType = object.value("scalar_type").toString();
    result.computedOn = object.value("computed_on").toString();
    result.widthPixels = object.value("width_pixels").toInt(1);
    result.vertexCount = object.value("vertex_count").toInt();
    result.totalLength = object.value("total_length").toDouble();
    result.validSamples = object.value("valid_samples").toInt();
    result.positions =
            this->decodePayload(object, QStringLiteral("positions_base64"), QStringLiteral("compression"),
                                result.error);
    if (result.error.isEmpty()) {
        result.data =
                this->decodePayload(object, QStringLiteral("data_base64"), QStringLiteral("compression"),
                                    result.error);
    }
    if (!result.error.isEmpty()) {
        result.valid = false;
    }
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
    result.data = this->decodePayload(object, QStringLiteral("data_base64"), QStringLiteral("compression"),
                                      result.error);
    if (!result.error.isEmpty()) {
        result.valid = false;
    }
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
            this->decodePayload(object, QStringLiteral("points_base64"), QStringLiteral("compression"),
                                result.error);
    if (result.error.isEmpty()) {
        result.polysData =
                this->decodePayload(object, QStringLiteral("polys_base64"), QStringLiteral("compression"),
                                    result.error);
    }
    if (!result.error.isEmpty()) {
        result.valid = false;
    }
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
    result.data = this->decodePayload(object, QStringLiteral("data_base64"), QStringLiteral("compression"),
                                      result.error);
    if (!result.error.isEmpty()) {
        result.valid = false;
    }
    return result;
}

QByteArray BackendClient::decodePayload(const QJsonObject &object, const QString &base64Field,
                                        const QString &compressionField, QString &error)
{
    return decodeCompressedPayload(object.value(base64Field).toString().toUtf8(),
                                   object.value(compressionField).toString(), error);
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
