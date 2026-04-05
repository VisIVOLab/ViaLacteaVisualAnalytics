#include "BackendClient.h"

#include <QByteArray>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcessEnvironment>
#include <QDebug>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

// ── Private helpers ───────────────────────────────────────────────────────────

namespace {

QByteArray decodeCompressedPayload(const QByteArray &encoded, const QString &compression,
                                   QString &error)
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

} // namespace

// ── Construction ──────────────────────────────────────────────────────────────

BackendClient::BackendClient(QString baseUrl, QString token)
    : m_baseUrl(std::move(baseUrl)), m_token(std::move(token))
{
    // R1: Auto-resolve token from env / file if not provided explicitly.
    if (m_token.isEmpty()) {
        m_token = BackendClient::readTokenFile();
    }
}

// ── Accessors ─────────────────────────────────────────────────────────────────

QString BackendClient::baseUrl() const
{
    return m_baseUrl;
}

void BackendClient::setBaseUrl(const QString &url)
{
    m_baseUrl = url;
}

QString BackendClient::token() const
{
    return m_token;
}

void BackendClient::setToken(const QString &token)
{
    m_token = token;
}

QString BackendClient::sessionId() const
{
    return m_sessionId;
}

void BackendClient::setSessionId(const QString &sessionId)
{
    m_sessionId = sessionId;
}

// ── Static helpers ────────────────────────────────────────────────────────────

QString BackendClient::readTokenFile()
{
    // Priority 1: environment variable (set by launch script or user shell).
    const QString envToken =
            QProcessEnvironment::systemEnvironment().value(QStringLiteral("VISIVO_TOKEN"));
    if (!envToken.isEmpty()) {
        return envToken.trimmed();
    }

    // Priority 2: ~/.visivo_token written by the Python backend at startup.
    const QString tokenPath =
            QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
                    .absoluteFilePath(QStringLiteral(".visivo_token"));
    QFile tokenFile(tokenPath);
    if (tokenFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString token = QString::fromUtf8(tokenFile.readAll()).trimmed();
        tokenFile.close();
        return token;
    }

    return {};
}

// ── Request builder ───────────────────────────────────────────────────────────

QNetworkRequest BackendClient::buildRequest(const QUrl &url) const
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    // R1: Authentication header.
    if (!m_token.isEmpty()) {
        request.setRawHeader(QByteArrayLiteral("X-Visivo-Token"), m_token.toUtf8());
    }

    // R3: Session isolation header.
    if (!m_sessionId.isEmpty()) {
        request.setRawHeader(QByteArrayLiteral("X-Visivo-Session"), m_sessionId.toUtf8());
    }

    return request;
}

// ── API calls ─────────────────────────────────────────────────────────────────

BackendHealthResult BackendClient::health() const
{
    BackendHealthResult result;
    QString error;
    // R8: /v1/ prefix on all routes.
    const QByteArray payload = performGet(QUrl(m_baseUrl + QStringLiteral("/v1/health")), error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }
    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.ok = object.value(QStringLiteral("ok")).toBool(false);
    result.workers = object.value(QStringLiteral("workers")).toInt();
    result.activeSessions = object.value(QStringLiteral("active_sessions")).toInt();
    result.productCacheEntries = object.value(QStringLiteral("product_cache_entries")).toInt();
    result.productCacheCapacity = object.value(QStringLiteral("product_cache_capacity")).toInt();
    result.taskRegistryEntries = object.value(QStringLiteral("task_registry_entries")).toInt();
    result.taskTtlEnabled = object.value(QStringLiteral("task_ttl_enabled")).toBool(false);
    result.taskTtlSeconds = object.value(QStringLiteral("task_ttl_seconds")).toInt();
    if (!result.ok) {
        result.error = QStringLiteral("Backend health check failed.");
    }
    return result;
}

BackendListFilesResult BackendClient::listFiles(const QString &path) const
{
    BackendListFilesResult result;
    QString error;
    QUrl url(m_baseUrl + QStringLiteral("/v1/files/list"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("path"), path);
    url.setQuery(query);
    const QByteArray payload = performGet(url, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }
    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value(QStringLiteral("valid")).toBool(false);
    result.error = object.value(QStringLiteral("error")).toString();
    result.currentPath = object.value(QStringLiteral("current_path")).toString();
    const QJsonArray entries = object.value(QStringLiteral("entries")).toArray();
    result.entries.reserve(static_cast<std::size_t>(entries.size()));
    for (const QJsonValue &value : entries) {
        const QJsonObject e = value.toObject();
        result.entries.push_back({ e.value(QStringLiteral("name")).toString(),
                                   e.value(QStringLiteral("path")).toString(),
                                   e.value(QStringLiteral("type")).toString(),
                                   e.value(QStringLiteral("size")).toInteger(),
                                   e.value(QStringLiteral("modified_time")).toString(),
                                   e.value(QStringLiteral("is_fits")).toBool(false) });
    }
    return result;
}

BackendFileHeaderResult BackendClient::fileHeader(const QString &path) const
{
    BackendFileHeaderResult result;
    QString error;
    const QJsonDocument body(QJsonObject{ { QStringLiteral("path"), path } });
    const QByteArray payload =
            performPost(QUrl(m_baseUrl + QStringLiteral("/v1/files/header")), body, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }
    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value(QStringLiteral("valid")).toBool(false);
    result.error = object.value(QStringLiteral("error")).toString();
    const QJsonArray cards = object.value(QStringLiteral("cards")).toArray();
    result.cards.reserve(cards.size());
    for (const QJsonValue &v : cards) {
        result.cards.push_back(v.toString());
    }
    return result;
}

BackendOpenDatasetResult BackendClient::openDataset(const QString &path)
{
    BackendOpenDatasetResult result;
    QString error;
    const QJsonDocument body(QJsonObject{ { QStringLiteral("path"), path } });
    const QByteArray payload =
            performPost(QUrl(m_baseUrl + QStringLiteral("/v1/datasets/open")), body, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }
    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value(QStringLiteral("valid")).toBool(false);
    result.error = object.value(QStringLiteral("error")).toString();
    result.datasetId = object.value(QStringLiteral("dataset_id")).toString();
    result.sessionId = object.value(QStringLiteral("session_id")).toString();

    // R3: Persist the session_id so subsequent requests use the same session.
    if (result.valid && !result.sessionId.isEmpty()) {
        m_sessionId = result.sessionId;
    }

    result.kind = object.value(QStringLiteral("kind")).toString();
    result.activeAxes = object.value(QStringLiteral("active_axes")).toInt();
    result.width = object.value(QStringLiteral("width")).toInt();
    result.height = object.value(QStringLiteral("height")).toInt();
    result.depth = object.value(QStringLiteral("depth")).toInt();
    result.degenerateAxesSummary =
            object.value(QStringLiteral("degenerate_axes_summary")).toString();
    result.wcsStatus = object.value(QStringLiteral("wcs_status")).toString(QStringLiteral("ok"));
    result.wcsWarningMessage = object.value(QStringLiteral("wcs_warning_message")).toString();
    const QJsonArray wcsSanitizedAxes = object.value(QStringLiteral("wcs_sanitized_axes")).toArray();
    result.wcsSanitizedAxes.clear();
    result.wcsSanitizedAxes.reserve(wcsSanitizedAxes.size());
    for (const QJsonValue &value : wcsSanitizedAxes) {
        result.wcsSanitizedAxes.push_back(value.toInt());
    }
    result.spectralAxisType = object.value(QStringLiteral("spectral_axis_type")).toString();
    result.spectralAxisUnit = object.value(QStringLiteral("spectral_axis_unit")).toString();

    const QJsonArray spacing = object.value(QStringLiteral("spacing")).toArray();
    const QJsonArray origin = object.value(QStringLiteral("origin")).toArray();
    const QJsonArray ctype = object.value(QStringLiteral("ctype")).toArray();
    const QJsonArray cunit = object.value(QStringLiteral("cunit")).toArray();
    const QJsonArray crval = object.value(QStringLiteral("crval")).toArray();
    const QJsonArray crpix = object.value(QStringLiteral("crpix")).toArray();
    const QJsonArray cdelt = object.value(QStringLiteral("cdelt")).toArray();
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

BackendCubePreviewResult BackendClient::requestPreview(const QString &datasetId,
                                                       int downsample) const
{
    BackendCubePreviewResult result;
    QString error;
    const QJsonDocument body(QJsonObject{ { QStringLiteral("dataset_id"), datasetId },
                                          { QStringLiteral("downsample"), downsample } });
    const QByteArray payload =
            performPost(QUrl(m_baseUrl + QStringLiteral("/v1/cube/preview")), body, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }
    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value(QStringLiteral("valid")).toBool(false);
    result.error = object.value(QStringLiteral("error")).toString();
    result.width = object.value(QStringLiteral("width")).toInt();
    result.height = object.value(QStringLiteral("height")).toInt();
    result.depth = object.value(QStringLiteral("depth")).toInt();
    result.scalarType = object.value(QStringLiteral("scalar_type")).toString();
    result.rangeMin = object.value(QStringLiteral("range_min")).toDouble();
    result.rangeMax = object.value(QStringLiteral("range_max")).toDouble();
    result.data = decodePayload(object, QStringLiteral("data_base64"),
                                QStringLiteral("compression"), result.error);
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
    const QJsonDocument body(QJsonObject{ { QStringLiteral("dataset_id"), datasetId },
                                          { QStringLiteral("axis"), axis },
                                          { QStringLiteral("index"), index } });
    const QByteArray payload =
            performPost(QUrl(m_baseUrl + QStringLiteral("/v1/cube/slice")), body, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }
    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value(QStringLiteral("valid")).toBool(false);
    result.error = object.value(QStringLiteral("error")).toString();
    result.width = object.value(QStringLiteral("width")).toInt();
    result.height = object.value(QStringLiteral("height")).toInt();
    result.scalarType = object.value(QStringLiteral("scalar_type")).toString();
    result.rangeMin = object.value(QStringLiteral("range_min")).toDouble();
    result.rangeMax = object.value(QStringLiteral("range_max")).toDouble();
    result.data = decodePayload(object, QStringLiteral("data_base64"),
                                QStringLiteral("compression"), result.error);
    if (!result.error.isEmpty()) {
        result.valid = false;
    }
    return result;
}

BackendCubeSubvolumeResult BackendClient::requestSubvolume(const QString &datasetId, int x0,
                                                           int x1, int y0, int y1, int z0,
                                                           int z1) const
{
    BackendCubeSubvolumeResult result;
    QString error;
    const QJsonDocument body(QJsonObject{ { QStringLiteral("dataset_id"), datasetId },
                                          { QStringLiteral("x0"), x0 },
                                          { QStringLiteral("x1"), x1 },
                                          { QStringLiteral("y0"), y0 },
                                          { QStringLiteral("y1"), y1 },
                                          { QStringLiteral("z0"), z0 },
                                          { QStringLiteral("z1"), z1 } });
    const QByteArray payload =
            performPost(QUrl(m_baseUrl + QStringLiteral("/v1/cube/subvolume")), body, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }
    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value(QStringLiteral("valid")).toBool(false);
    result.error = object.value(QStringLiteral("error")).toString();
    result.width = object.value(QStringLiteral("width")).toInt();
    result.height = object.value(QStringLiteral("height")).toInt();
    result.depth = object.value(QStringLiteral("depth")).toInt();
    result.scalarType = object.value(QStringLiteral("scalar_type")).toString();
    result.data = decodePayload(object, QStringLiteral("data_base64"),
                                QStringLiteral("compression"), result.error);
    if (!result.error.isEmpty()) {
        result.valid = false;
    }
    return result;
}

BackendCubePvResult BackendClient::requestPv(const QString &datasetId,
                                             const std::vector<std::array<int, 2>> &vertices,
                                             int widthPixels) const
{
    QString error;
    QJsonArray vertexArray;
    for (const auto &v : vertices) {
        vertexArray.append(QJsonArray{ v[0], v[1] });
    }
    const QJsonDocument body(QJsonObject{ { QStringLiteral("dataset_id"), datasetId },
                                          { QStringLiteral("vertices"), vertexArray },
                                          { QStringLiteral("width_pixels"), widthPixels } });
    const QByteArray payload =
            performPost(QUrl(m_baseUrl + QStringLiteral("/v1/cube/pv")), body, error);
    if (!error.isEmpty()) {
        BackendCubePvResult result;
        result.error = error;
        return result;
    }
    return parsePvResultObject(QJsonDocument::fromJson(payload).object());
}

BackendImageResult BackendClient::requestImage(const QString &datasetId) const
{
    BackendImageResult result;
    QString error;
    const QJsonDocument body(QJsonObject{ { QStringLiteral("dataset_id"), datasetId } });
    const QByteArray payload =
            performPost(QUrl(m_baseUrl + QStringLiteral("/v1/image/full")), body, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }
    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value(QStringLiteral("valid")).toBool(false);
    result.error = object.value(QStringLiteral("error")).toString();
    result.width = object.value(QStringLiteral("width")).toInt();
    result.height = object.value(QStringLiteral("height")).toInt();
    result.fullWidth = object.value(QStringLiteral("full_width")).toInt(result.width);
    result.fullHeight = object.value(QStringLiteral("full_height")).toInt(result.height);
    result.scalarType = object.value(QStringLiteral("scalar_type")).toString();
    result.rangeMin = object.value(QStringLiteral("range_min")).toDouble();
    result.rangeMax = object.value(QStringLiteral("range_max")).toDouble();
    result.isPreview = object.value(QStringLiteral("is_preview")).toBool(false);
    result.previewScaleFactor =
            object.value(QStringLiteral("preview_scale_factor")).toDouble(1.0);
    result.data = decodePayload(object, QStringLiteral("data_base64"),
                                QStringLiteral("compression"), result.error);
    if (!result.error.isEmpty()) {
        result.valid = false;
    }
    return result;
}

BackendImageResult BackendClient::requestImagePreview(const QString &datasetId,
                                                      int maxLongestSide) const
{
    BackendImageResult result;
    QString error;
    const QJsonDocument body(QJsonObject{ { QStringLiteral("dataset_id"), datasetId },
                                          { QStringLiteral("max_longest_side"), maxLongestSide } });
    const QByteArray payload =
            performPost(QUrl(m_baseUrl + QStringLiteral("/v1/image/preview")), body, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }
    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value(QStringLiteral("valid")).toBool(false);
    result.error = object.value(QStringLiteral("error")).toString();
    result.width = object.value(QStringLiteral("width")).toInt();
    result.height = object.value(QStringLiteral("height")).toInt();
    result.fullWidth = object.value(QStringLiteral("full_width")).toInt(result.width);
    result.fullHeight = object.value(QStringLiteral("full_height")).toInt(result.height);
    result.scalarType = object.value(QStringLiteral("scalar_type")).toString();
    result.rangeMin = object.value(QStringLiteral("range_min")).toDouble();
    result.rangeMax = object.value(QStringLiteral("range_max")).toDouble();
    result.isPreview = object.value(QStringLiteral("is_preview")).toBool(false);
    result.previewScaleFactor =
            object.value(QStringLiteral("preview_scale_factor")).toDouble(1.0);
    result.data = decodePayload(object, QStringLiteral("data_base64"),
                                QStringLiteral("compression"), result.error);
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
    const QJsonDocument body(QJsonObject{ { QStringLiteral("dataset_id"), datasetId },
                                          { QStringLiteral("threshold"), threshold } });
    const QByteArray payload =
            performPost(QUrl(m_baseUrl + QStringLiteral("/v1/products/isosurface")), body, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }
    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value(QStringLiteral("valid")).toBool(false);
    result.error = object.value(QStringLiteral("error")).toString();
    result.numPoints = object.value(QStringLiteral("num_points")).toInt();
    result.numPolys = object.value(QStringLiteral("num_polys")).toInt();
    result.pointsData = decodePayload(object, QStringLiteral("points_base64"),
                                      QStringLiteral("compression"), result.error);
    if (result.error.isEmpty()) {
        result.polysData = decodePayload(object, QStringLiteral("polys_base64"),
                                         QStringLiteral("compression"), result.error);
    }
    if (!result.error.isEmpty()) {
        result.valid = false;
    }
    return result;
}

BackendMomentResult BackendClient::requestMoment(const QString &datasetId, int order,
                                                 int channelStart, int channelEnd,
                                                 bool maskEnabled, double thresholdValue) const
{
    QString error;
    const QJsonDocument body(
            QJsonObject{ { QStringLiteral("dataset_id"), datasetId },
                         { QStringLiteral("moment_order"), order },
                         { QStringLiteral("channel_start"), channelStart },
                         { QStringLiteral("channel_end"), channelEnd },
                         { QStringLiteral("mask_enabled"), maskEnabled },
                         { QStringLiteral("threshold_value"), thresholdValue } });
    const QByteArray payload =
            performPost(QUrl(m_baseUrl + QStringLiteral("/v1/products/moment")), body, error);
    if (!error.isEmpty()) {
        BackendMomentResult result;
        result.error = error;
        return result;
    }
    return parseMomentResultObject(QJsonDocument::fromJson(payload).object());
}

BackendTaskCreateResult BackendClient::createMomentTask(const QString &datasetId, int order,
                                                        int channelStart, int channelEnd,
                                                        bool maskEnabled,
                                                        double thresholdValue) const
{
    BackendTaskCreateResult result;
    QString error;
    const QJsonDocument body(
            QJsonObject{ { QStringLiteral("dataset_id"), datasetId },
                         { QStringLiteral("moment_order"), order },
                         { QStringLiteral("channel_start"), channelStart },
                         { QStringLiteral("channel_end"), channelEnd },
                         { QStringLiteral("mask_enabled"), maskEnabled },
                         { QStringLiteral("threshold_value"), thresholdValue } });
    const QByteArray payload =
            performPost(QUrl(m_baseUrl + QStringLiteral("/v1/tasks/moment")), body, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }
    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value(QStringLiteral("valid")).toBool(false);
    result.error = object.value(QStringLiteral("error")).toString();
    result.taskId = object.value(QStringLiteral("task_id")).toString();
    result.status = object.value(QStringLiteral("status")).toString();
    result.cacheHit = object.value(QStringLiteral("cache_hit")).toBool(false);
    return result;
}

BackendTaskCreateResult BackendClient::createPvTask(const QString &datasetId,
                                                    const std::vector<std::array<int, 2>> &vertices,
                                                    int widthPixels) const
{
    BackendTaskCreateResult result;
    QString error;
    QJsonArray vertexArray;
    for (const auto &v : vertices) {
        vertexArray.append(QJsonArray{ v[0], v[1] });
    }
    const QJsonDocument body(QJsonObject{ { QStringLiteral("dataset_id"), datasetId },
                                          { QStringLiteral("vertices"), vertexArray },
                                          { QStringLiteral("width_pixels"), widthPixels } });
    const QByteArray payload =
            performPost(QUrl(m_baseUrl + QStringLiteral("/v1/tasks/pv")), body, error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }
    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value(QStringLiteral("valid")).toBool(false);
    result.error = object.value(QStringLiteral("error")).toString();
    result.taskId = object.value(QStringLiteral("task_id")).toString();
    result.status = object.value(QStringLiteral("status")).toString();
    result.cacheHit = object.value(QStringLiteral("cache_hit")).toBool(false);
    return result;
}

BackendTaskStatusResult BackendClient::requestTaskStatus(const QString &taskId) const
{
    BackendTaskStatusResult result;
    QString error;
    const QByteArray payload =
            performGet(QUrl(m_baseUrl + QStringLiteral("/v1/tasks/") + taskId), error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }
    const QJsonObject object = QJsonDocument::fromJson(payload).object();
    result.valid = object.value(QStringLiteral("valid")).toBool(false);
    result.error = object.value(QStringLiteral("error")).toString();
    result.taskId = object.value(QStringLiteral("task_id")).toString();
    result.operation = object.value(QStringLiteral("operation")).toString();
    result.status = object.value(QStringLiteral("status")).toString();
    result.progress = object.value(QStringLiteral("progress")).toDouble();
    result.cacheHit = object.value(QStringLiteral("cache_hit")).toBool(false);
    result.resultObject = object.value(QStringLiteral("result")).toObject();
    return result;
}

BackendTaskStatusResult BackendClient::waitForTaskCompletion(const BackendTaskCreateResult &createResult,
                                                             const QString &logTag,
                                                             int maxPollAttempts,
                                                             int pollIntervalMs) const
{
    BackendTaskStatusResult terminal;
    terminal.taskId = createResult.taskId;
    if (!createResult.valid || createResult.taskId.isEmpty()) {
        terminal.error = createResult.error.isEmpty()
                ? QStringLiteral("Task creation failed.")
                : createResult.error;
        qWarning().noquote()
                << QStringLiteral("%1 create failed error=%2").arg(logTag, terminal.error);
        return terminal;
    }

    qDebug().noquote()
            << QStringLiteral("%1 created task_id=%2 status=%3 cache_hit=%4")
                       .arg(logTag, createResult.taskId, createResult.status)
                       .arg(createResult.cacheHit ? QStringLiteral("true")
                                                 : QStringLiteral("false"));

    for (int attempt = 0; attempt < maxPollAttempts; ++attempt) {
        QThread::msleep(pollIntervalMs);
        const auto taskStatus = this->requestTaskStatus(createResult.taskId);
        if (!taskStatus.valid && !taskStatus.status.isEmpty()
            && taskStatus.status != QStringLiteral("failed")) {
            qWarning().noquote()
                    << QStringLiteral("%1 polling invalid task_id=%2 error=%3")
                               .arg(logTag, createResult.taskId, taskStatus.error);
            return taskStatus;
        }
        qDebug().noquote()
                << QStringLiteral("%1 poll task_id=%2 status=%3 progress=%4")
                           .arg(logTag, createResult.taskId, taskStatus.status)
                           .arg(taskStatus.progress, 0, 'f', 2);
        if (taskStatus.status == QStringLiteral("completed")) {
            qDebug().noquote()
                    << QStringLiteral("%1 completed task_id=%2 cache_hit=%3")
                               .arg(logTag, createResult.taskId)
                               .arg(taskStatus.cacheHit ? QStringLiteral("true")
                                                        : QStringLiteral("false"));
            return taskStatus;
        }
        if (taskStatus.status == QStringLiteral("failed")) {
            qWarning().noquote()
                    << QStringLiteral("%1 failed task_id=%2 error=%3")
                               .arg(logTag, createResult.taskId, taskStatus.error);
            return taskStatus;
        }
    }

    terminal.error = QStringLiteral("Task polling timed out.");
    qWarning().noquote()
            << QStringLiteral("%1 polling timed out task_id=%2").arg(logTag, createResult.taskId);
    return terminal;
}

// ── Low-level HTTP ────────────────────────────────────────────────────────────

std::chrono::milliseconds BackendClient::requestTimeoutFor(const QUrl &url) const
{
    const QString path = url.path();
    if (path.endsWith(QStringLiteral("/health"))
        || path.endsWith(QStringLiteral("/files/list"))
        || path.endsWith(QStringLiteral("/files/header"))) {
        return std::chrono::milliseconds(5000);
    }
    if (path.endsWith(QStringLiteral("/datasets/open"))) {
        return std::chrono::milliseconds(10000);
    }
    if (path.endsWith(QStringLiteral("/tasks/moment")) || path.contains(QStringLiteral("/tasks/"))) {
        return std::chrono::milliseconds(10000);
    }
    if (path.endsWith(QStringLiteral("/cube/slice")) || path.endsWith(QStringLiteral("/image/preview"))) {
        return std::chrono::milliseconds(30000);
    }
    return std::chrono::milliseconds(120000);
}

BackendMomentResult BackendClient::parseMomentResultObject(const QJsonObject &object)
{
    BackendMomentResult result;
    result.valid = object.value(QStringLiteral("valid")).toBool(false);
    result.error = object.value(QStringLiteral("error")).toString();
    result.width = object.value(QStringLiteral("width")).toInt();
    result.height = object.value(QStringLiteral("height")).toInt();
    result.scalarType = object.value(QStringLiteral("scalar_type")).toString();
    result.rangeMin = object.value(QStringLiteral("range_min")).toDouble();
    result.rangeMax = object.value(QStringLiteral("range_max")).toDouble();
    result.spectralAxisType = object.value(QStringLiteral("spectral_axis_type")).toString();
    result.spectralAxisUnit = object.value(QStringLiteral("spectral_axis_unit")).toString();
    result.momentUnit = object.value(QStringLiteral("moment_unit")).toString();
    result.bunit = object.value(QStringLiteral("bunit")).toString();
    result.wcsStatus = object.value(QStringLiteral("wcs_status")).toString(QStringLiteral("ok"));
    result.wcsWarningMessage = object.value(QStringLiteral("wcs_warning_message")).toString();
    result.data = decodePayload(object, QStringLiteral("data_base64"),
                                QStringLiteral("compression"), result.error);
    if (!result.error.isEmpty()) {
        result.valid = false;
    }
    return result;
}

BackendCubePvResult BackendClient::parsePvResultObject(const QJsonObject &object)
{
    BackendCubePvResult result;
    result.valid = object.value(QStringLiteral("valid")).toBool(false);
    result.error = object.value(QStringLiteral("error")).toString();
    result.numSamples = object.value(QStringLiteral("num_samples")).toInt();
    result.depth = object.value(QStringLiteral("depth")).toInt();
    result.scalarType = object.value(QStringLiteral("scalar_type")).toString();
    result.computedOn = object.value(QStringLiteral("computed_on")).toString();
    result.widthPixels = object.value(QStringLiteral("width_pixels")).toInt(1);
    result.vertexCount = object.value(QStringLiteral("vertex_count")).toInt();
    result.totalLength = object.value(QStringLiteral("total_length")).toDouble();
    result.validSamples = object.value(QStringLiteral("valid_samples")).toInt();
    result.spectralAxisType = object.value(QStringLiteral("spectral_axis_type")).toString();
    result.spectralAxisUnit = object.value(QStringLiteral("spectral_axis_unit")).toString();
    result.bunit = object.value(QStringLiteral("bunit")).toString();
    result.beamMajor = object.value(QStringLiteral("beam_major")).toDouble();
    result.beamMinor = object.value(QStringLiteral("beam_minor")).toDouble();
    result.beamPa = object.value(QStringLiteral("beam_pa")).toDouble();
    result.positionsArcsec = decodePayload(object, QStringLiteral("positions_arcsec_base64"),
                                           QStringLiteral("compression"), result.error);
    result.error.clear(); // missing arcsec positions is non-fatal
    result.pixelScaleArcsecPerPixel =
            object.value(QStringLiteral("pixel_scale_arcsec_per_pixel")).toDouble(0.);
    result.spatialUnit = object.value(QStringLiteral("spatial_unit")).toString(QStringLiteral("pixel"));
    result.positions = decodePayload(object, QStringLiteral("positions_base64"),
                                     QStringLiteral("compression"), result.error);
    if (result.error.isEmpty()) {
        result.data = decodePayload(object, QStringLiteral("data_base64"),
                                    QStringLiteral("compression"), result.error);
    }
    if (!result.error.isEmpty()) {
        result.valid = false;
    }
    return result;
}

QByteArray BackendClient::performGet(const QUrl &url, QString &error) const
{
    QNetworkAccessManager nam;
    QNetworkRequest request = buildRequest(url);

    QEventLoop loop;
    QNetworkReply *reply = nam.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, [&]() {
        error = QStringLiteral("Backend request timed out after %1 ms: %2")
                        .arg(this->requestTimeoutFor(url).count())
                        .arg(url.path());
        qWarning().noquote() << error;
        reply->abort();
        loop.quit();
    });
    timeoutTimer.start(static_cast<int>(this->requestTimeoutFor(url).count()));
    loop.exec();
    timeoutTimer.stop();

    const QByteArray data = reply->readAll();
    if (error.isEmpty() && reply->error() != QNetworkReply::NoError) {
        error = reply->errorString();
    } else if (error.isEmpty()) {
        const int status =
                reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status >= 400) {
            error = QString::fromUtf8(data);
        }
    }
    reply->deleteLater();
    return data;
}

QByteArray BackendClient::performPost(const QUrl &url, const QJsonDocument &body,
                                      QString &error) const
{
    QNetworkAccessManager nam;
    QNetworkRequest request = buildRequest(url);

    QEventLoop loop;
    QNetworkReply *reply = nam.post(request, body.toJson(QJsonDocument::Compact));
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, [&]() {
        error = QStringLiteral("Backend request timed out after %1 ms: %2")
                        .arg(this->requestTimeoutFor(url).count())
                        .arg(url.path());
        qWarning().noquote() << error;
        reply->abort();
        loop.quit();
    });
    timeoutTimer.start(static_cast<int>(this->requestTimeoutFor(url).count()));
    loop.exec();
    timeoutTimer.stop();

    const QByteArray data = reply->readAll();
    if (error.isEmpty() && reply->error() != QNetworkReply::NoError) {
        error = reply->errorString();
    } else if (error.isEmpty()) {
        const int status =
                reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status >= 400) {
            error = QString::fromUtf8(data);
        }
    }
    reply->deleteLater();
    return data;
}

QByteArray BackendClient::decodePayload(const QJsonObject &object, const QString &base64Field,
                                        const QString &compressionField, QString &error)
{
    return decodeCompressedPayload(object.value(base64Field).toString().toUtf8(),
                                   object.value(compressionField).toString(), error);
}
