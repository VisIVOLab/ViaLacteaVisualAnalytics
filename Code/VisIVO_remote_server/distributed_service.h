#pragma once

#include <QJsonObject>
#include <QString>
#include <vector>

class SceneManager;

// Orchestrates MPI workers; falls back to local SceneManager when size==1.
class DistributedService {
public:
    DistributedService(int rank, int size);
    ~DistributedService();

    QJsonObject loadDataset(const QString &path);
    QJsonObject setCamera(const QJsonObject &params);
    QJsonObject setSlice(int slice);
    QJsonObject setWindowLevel(double window, double level);
    QJsonObject renderFrame(int width, int height, const QString &mode = QString(), const QJsonObject &volumeParams = QJsonObject());

private:
    int m_rank;
    int m_size;
    SceneManager *m_localScene;
};
