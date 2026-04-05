#ifndef BackendClient_h
#define BackendClient_h

#include <QByteArray>
#include <QJsonObject>
#include <QList>
#include <QNetworkRequest>
#include <QString>
#include <QStringList>
#include <chrono>

#include <array>
#include <vector>

struct BackendFileEntry
{
    QString name;
    QString path;
    QString type;
    qint64 size{ 0 };
    QString modifiedTime;
    bool isFits{ false };
};

struct BackendHealthResult
{
    bool ok{ false };
    QString error;
    int workers{ 0 };
    int activeSessions{ 0 };
    int productCacheEntries{ 0 };
    int productCacheCapacity{ 0 };
    int taskRegistryEntries{ 0 };
    bool taskTtlEnabled{ false };
    int taskTtlSeconds{ 0 };
};

struct BackendListFilesResult
{
    bool valid{ false };
    QString error;
    QString currentPath;
    std::vector<BackendFileEntry> entries;
};

struct BackendOpenDatasetResult
{
    bool valid{ false };
    QString error;
    QString datasetId;
    QString sessionId;   // R3: echo as X-Visivo-Session in subsequent requests
    QString kind;
    int activeAxes{ 0 };
    int width{ 0 };
    int height{ 0 };
    int depth{ 0 };
    QString degenerateAxesSummary;
    QString wcsStatus{ QStringLiteral("ok") };
    QString wcsWarningMessage;
    QList<int> wcsSanitizedAxes;
    std::array<double, 3> spacing{ 1.0, 1.0, 1.0 };
    std::array<double, 3> origin{ 0.0, 0.0, 0.0 };
    std::array<QString, 3> ctype{ QString(), QString(), QString() };
    std::array<QString, 3> cunit{ QString(), QString(), QString() };
    std::array<double, 3> crval{ 0.0, 0.0, 0.0 };
    std::array<double, 3> crpix{ 1.0, 1.0, 1.0 };
    std::array<double, 3> cdelt{ 1.0, 1.0, 1.0 };
};

struct BackendFileHeaderResult
{
    bool valid{ false };
    QString error;
    QStringList cards;
};

struct BackendMomentResult
{
    bool valid{ false };
    QString error;
    int width{ 0 };
    int height{ 0 };
    QString scalarType;
    double rangeMin{ 0. };
    double rangeMax{ 0. };
    QString spectralAxisType;
    QString spectralAxisUnit;
    QString momentUnit;
    QString bunit;
    QByteArray data;
};

struct BackendTaskCreateResult
{
    bool valid{ false };
    QString error;
    QString taskId;
    QString status;
    bool cacheHit{ false };
};

struct BackendTaskStatusResult
{
    bool valid{ false };
    QString error;
    QString taskId;
    QString operation;
    QString status;
    double progress{ 0.0 };
    bool cacheHit{ false };
    QJsonObject resultObject;
};

struct BackendCubePreviewResult
{
    bool valid{ false };
    QString error;
    int width{ 0 };
    int height{ 0 };
    int depth{ 0 };
    QString scalarType;
    double rangeMin{ 0. };
    double rangeMax{ 0. };
    QByteArray data;
};

struct BackendCubeSliceResult
{
    bool valid{ false };
    QString error;
    int width{ 0 };
    int height{ 0 };
    QString scalarType;
    double rangeMin{ 0. };
    double rangeMax{ 0. };
    QByteArray data;
};

struct BackendCubeSubvolumeResult
{
    bool valid{ false };
    QString error;
    int width{ 0 };
    int height{ 0 };
    int depth{ 0 };
    QString scalarType;
    QByteArray data;
};

struct BackendCubePvResult
{
    bool valid{ false };
    QString error;
    int numSamples{ 0 };
    int depth{ 0 };
    QString scalarType;
    QByteArray positions;
    QByteArray data;
    QString computedOn;
    int widthPixels{ 1 };
    int vertexCount{ 0 };
    double totalLength{ 0. };
    int validSamples{ 0 };
    QString spectralAxisType;
    QString spectralAxisUnit;
    QString bunit;
    double beamMajor{ 0.0 };
    double beamMinor{ 0.0 };
    double beamPa{ 0.0 };
};

struct BackendImageResult
{
    bool valid{ false };
    QString error;
    int width{ 0 };
    int height{ 0 };
    int fullWidth{ 0 };
    int fullHeight{ 0 };
    QString scalarType;
    double rangeMin{ 0. };
    double rangeMax{ 0. };
    bool isPreview{ false };
    double previewScaleFactor{ 1.0 };
    QByteArray data;
};

struct BackendIsosurfaceResult
{
    bool valid{ false };
    QString error;
    int numPoints{ 0 };
    int numPolys{ 0 };
    QByteArray pointsData;
    QByteArray polysData;
};

class QJsonDocument;
class QJsonObject;
class QUrl;

/**
 * HTTP client for the VisIVO FastAPI backend.
 *
 * Changes vs original:
 *   R1  – Every request carries the X-Visivo-Token header (auth).
 *   R3  – openDataset() stores the returned session_id; all subsequent
 *          requests carry X-Visivo-Session so the backend can isolate
 *          per-session state.
 *   R8  – All URL paths are prefixed with /v1/.
 *
 * The token is resolved in this priority order:
 *   1. setToken() call (e.g. from Settings dialog)
 *   2. VISIVO_TOKEN environment variable
 *   3. Content of ~/.visivo_token (written by the backend at startup)
 */
class BackendClient
{
public:
    /**
     * @param baseUrl  Backend base URL, e.g. "http://compute-node42:8000"
     * @param token    Auth token.  If empty, readTokenFile() is attempted.
     */
    explicit BackendClient(QString baseUrl = QStringLiteral("http://127.0.0.1:8000"),
                           QString token = QString());

    // ── Accessors ─────────────────────────────────────────────────────────────

    QString baseUrl() const;
    void setBaseUrl(const QString &url);

    /** Current auth token.  Empty means no authentication is sent. */
    QString token() const;
    void setToken(const QString &token);

    /**
     * Session ID returned by the last successful openDataset() call.
     * Sent as X-Visivo-Session on every subsequent request.
     */
    QString sessionId() const;
    void setSessionId(const QString &sessionId);

    // ── Static helpers ────────────────────────────────────────────────────────

    /**
     * Try to read the auth token from:
     *   1. VISIVO_TOKEN environment variable
     *   2. ~/.visivo_token (file written by the backend at startup)
     * Returns an empty string if neither source is available.
     */
    static QString readTokenFile();

    // ── API calls ─────────────────────────────────────────────────────────────

    BackendHealthResult health() const;
    BackendListFilesResult listFiles(const QString &path) const;
    BackendFileHeaderResult fileHeader(const QString &path) const;

    /**
     * Open a FITS dataset on the backend.
     * On success the returned sessionId is automatically stored in this client
     * so all subsequent calls carry the correct X-Visivo-Session header.
     */
    BackendOpenDatasetResult openDataset(const QString &path);

    BackendCubePreviewResult requestPreview(const QString &datasetId, int downsample) const;
    BackendCubeSliceResult requestSlice(const QString &datasetId, const QString &axis,
                                        int index) const;
    BackendCubeSubvolumeResult requestSubvolume(const QString &datasetId, int x0, int x1, int y0,
                                                int y1, int z0, int z1) const;
    BackendCubePvResult requestPv(const QString &datasetId,
                                  const std::vector<std::array<int, 2>> &vertices,
                                  int widthPixels) const;
    BackendImageResult requestImagePreview(const QString &datasetId, int maxLongestSide) const;
    BackendImageResult requestImage(const QString &datasetId) const;
    BackendIsosurfaceResult requestIsosurface(const QString &datasetId, double threshold) const;
    BackendMomentResult requestMoment(const QString &datasetId, int order, int channelStart,
                                      int channelEnd, bool maskEnabled,
                                      double thresholdValue) const;
    BackendTaskCreateResult createMomentTask(const QString &datasetId, int order, int channelStart,
                                             int channelEnd, bool maskEnabled,
                                             double thresholdValue) const;
    BackendTaskCreateResult createPvTask(const QString &datasetId,
                                         const std::vector<std::array<int, 2>> &vertices,
                                         int widthPixels) const;
    BackendTaskStatusResult requestTaskStatus(const QString &taskId) const;
    BackendTaskStatusResult waitForTaskCompletion(const BackendTaskCreateResult &createResult,
                                                  const QString &logTag,
                                                  int maxPollAttempts = 120,
                                                  int pollIntervalMs = 250) const;
    static BackendMomentResult parseMomentResultObject(const QJsonObject &object);
    static BackendCubePvResult parsePvResultObject(const QJsonObject &object);

private:
    /** Build a QNetworkRequest with auth + session headers pre-applied. */
    QNetworkRequest buildRequest(const QUrl &url) const;

    QByteArray performGet(const QUrl &url, QString &error) const;
    QByteArray performPost(const QUrl &url, const QJsonDocument &body, QString &error) const;
    std::chrono::milliseconds requestTimeoutFor(const QUrl &url) const;

    static QByteArray decodePayload(const QJsonObject &object, const QString &base64Field,
                                    const QString &compressionField, QString &error);

    QString m_baseUrl;
    QString m_token;
    QString m_sessionId;
};

#endif
