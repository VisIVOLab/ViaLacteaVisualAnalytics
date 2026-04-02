#ifndef BackendClient_h
#define BackendClient_h

#include <QByteArray>
#include <QString>
#include <QStringList>

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
    QString kind;
    int activeAxes{ 0 };
    int width{ 0 };
    int height{ 0 };
    int depth{ 0 };
    QString degenerateAxesSummary;
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
    QByteArray data;
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

class BackendClient
{
public:
    explicit BackendClient(QString baseUrl = QStringLiteral("http://127.0.0.1:8000"));

    QString baseUrl() const;

    BackendHealthResult health() const;
    BackendListFilesResult listFiles(const QString &path) const;
    BackendFileHeaderResult fileHeader(const QString &path) const;
    BackendOpenDatasetResult openDataset(const QString &path) const;
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

private:
    QByteArray performGet(const QUrl &url, QString &error) const;
    QByteArray performPost(const QUrl &url, const QJsonDocument &body, QString &error) const;
    static QByteArray decodePayload(const QJsonObject &object, const QString &base64Field,
                                    const QString &compressionField, QString &error);

    QString m_baseUrl;
};

#endif
