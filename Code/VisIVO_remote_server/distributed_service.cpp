#include "distributed_service.h"
#include "scene_manager.h"

#include <mpi.h>
#include <QJsonArray>
#include <QJsonValue>
#include <QStringLiteral>
#include <QByteArray>

namespace {
enum CommandTag {
    CMD_NONE = 0,
    CMD_LOAD = 1,
    CMD_RENDER = 2,
    CMD_EXIT = 99
};
const int FRAME_TAG = 200;
}

DistributedService::DistributedService(int rank, int size)
    : m_rank(rank), m_size(size), m_localScene(new SceneManager) {}

DistributedService::~DistributedService() {
    delete m_localScene;
    // Tell workers to exit
    if (m_rank == 0 && m_size > 1) {
        int cmd = CMD_EXIT;
        MPI_Bcast(&cmd, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
}

QJsonObject DistributedService::loadDataset(const QString &path) {
    // Local load (for fallback/rendering)
    QJsonObject localRes = m_localScene->loadFits(path);

    if (m_rank == 0 && m_size > 1) {
        int cmd = CMD_LOAD;
        MPI_Bcast(&cmd, 1, MPI_INT, 0, MPI_COMM_WORLD);
        std::string p = path.toStdString();
        int len = static_cast<int>(p.size());
        MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(p.data(), len, MPI_CHAR, 0, MPI_COMM_WORLD);
        // Collect statuses (optional)
        for (int r = 1; r < m_size; ++r) {
            int status = -1;
            MPI_Recv(&status, 1, MPI_INT, r, CMD_LOAD, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    return localRes;
}

QJsonObject DistributedService::setCamera(const QJsonObject &params) {
    // Only local camera for now
    return m_localScene->setCamera(params);
}

QJsonObject DistributedService::renderFrame(int width, int height) {
    if (m_size <= 1) {
        return m_localScene->renderPng(width, height);
    }

    // Ask all workers to render their slice; pick first non-empty frame
    int cmd = CMD_RENDER;
    MPI_Bcast(&cmd, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int dims[2] = {width, height};
    MPI_Bcast(dims, 2, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<char> frameData;
    bool gotFrame = false;
    for (int r = 1; r < m_size; ++r) {
        int frameSize = 0;
        MPI_Status st;
        MPI_Recv(&frameSize, 1, MPI_INT, r, FRAME_TAG, MPI_COMM_WORLD, &st);
        if (frameSize > 0 && !gotFrame) {
            frameData.resize(frameSize);
            MPI_Recv(frameData.data(), frameSize, MPI_CHAR, r, FRAME_TAG + 1, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
            gotFrame = true;
        } else if (frameSize > 0) {
            // Drain the payload we won't use
            std::vector<char> tmp(frameSize);
            MPI_Recv(tmp.data(), frameSize, MPI_CHAR, r, FRAME_TAG + 1, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
        }
    }

    if (gotFrame) {
        const QString b64 = QString::fromLatin1(QByteArray(frameData.data(),
                                                          static_cast<int>(frameData.size()))
                                                          .toBase64());
        return {{"status", QStringLiteral("ok")}, {"image", b64}};
    }
    // Fallback to local render if no worker responded
    return m_localScene->renderPng(width, height);
}
