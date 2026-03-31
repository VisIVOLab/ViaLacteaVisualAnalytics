#ifndef BackendClient_h
#define BackendClient_h

#include <QByteArray>
#include <QString>

#include <vector>

struct BackendFileEntry
{
    QString name;
    QString path;
    QString type;
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
    std::vector<BackendFileEntry> entries;
};

struct BackendOpenDatasetResult
{
    bool valid{ false };
    QString error;
    QString datasetId;
    QString kind;
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

class QJsonDocument;
class QUrl;

class BackendClient
{
public:
    explicit BackendClient(QString baseUrl = QStringLiteral("http://127.0.0.1:8000"));

    QString baseUrl() const;

    BackendHealthResult health() const;
    BackendListFilesResult listFiles(const QString &path) const;
    BackendOpenDatasetResult openDataset(const QString &path) const;
    BackendMomentResult requestMoment(const QString &datasetId, int order) const;

private:
    QByteArray performGet(const QUrl &url, QString &error) const;
    QByteArray performPost(const QUrl &url, const QJsonDocument &body, QString &error) const;

    QString m_baseUrl;
};

#endif
