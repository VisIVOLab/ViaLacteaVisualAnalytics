#include "distributed_service.h"
#include "scene_manager.h"

#include <mpi.h>
#include <QJsonArray>
#include <QJsonValue>
#include <QStringLiteral>
#include <QByteArray>
#include <QImage>
#include <QBuffer>

namespace {
enum CommandTag {
    CMD_NONE = 0,
    CMD_LOAD = 1,
    CMD_RENDER = 2,
    CMD_SET_SLICE = 3,
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

QJsonObject DistributedService::setSlice(int slice) {
    auto res = m_localScene->setSlice(slice);
    if (m_rank == 0 && m_size > 1) {
        int cmd = CMD_SET_SLICE;
        MPI_Bcast(&cmd, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&slice, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    return res;
}

QJsonObject DistributedService::setWindowLevel(double window, double level) {
    auto res = m_localScene->setWindowLevel(window, level);
    return res;
}

QJsonObject DistributedService::renderFrame(int width, int height, const QString &mode, const QJsonObject &volumeParams) {
    // Volume mode: for ora solo locale
    if (mode == QStringLiteral("volume")) {
        return m_localScene->renderVolumePng(width, height, volumeParams);
    }

    if (m_size <= 1) {
        return m_localScene->renderPng(width, height);
    }

    // Ask all workers to render; composite color by depth (nearest depth wins)
    int cmd = CMD_RENDER;
    MPI_Bcast(&cmd, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int dims[2] = {width, height};
    MPI_Bcast(dims, 2, MPI_INT, 0, MPI_COMM_WORLD);

    const int pixelCount = width * height;
    std::vector<float> bestDepth(pixelCount, 1e9f);
    std::vector<unsigned char> bestColor(pixelCount * 4, 0);

    bool gotAny = false;
    for (int r = 1; r < m_size; ++r) {
        int rgbaSize = 0, depthSize = 0;
        MPI_Recv(&rgbaSize, 1, MPI_INT, r, FRAME_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&depthSize, 1, MPI_INT, r, FRAME_TAG + 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::vector<unsigned char> rgbaBuf;
        std::vector<float> depthBuf;
        if (rgbaSize > 0) {
            rgbaBuf.resize(static_cast<size_t>(rgbaSize));
            MPI_Recv(rgbaBuf.data(), rgbaSize, MPI_CHAR, r, FRAME_TAG + 2, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
        }
        if (depthSize > 0) {
            depthBuf.resize(static_cast<size_t>(depthSize) / sizeof(float));
            MPI_Recv(depthBuf.data(), depthSize, MPI_CHAR, r, FRAME_TAG + 3, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
        }
        if (rgbaBuf.empty() || depthBuf.empty()) continue;
        gotAny = true;
        const int count = std::min(pixelCount, static_cast<int>(depthBuf.size()));
        for (int i = 0; i < count; ++i) {
            float d = depthBuf[i];
            if (d <= 0.0f) continue; // skip invalid depth
            if (d < bestDepth[i]) {
                bestDepth[i] = d;
                const int bi = i * 4;
                bestColor[bi] = rgbaBuf[bi];
                bestColor[bi + 1] = rgbaBuf[bi + 1];
                bestColor[bi + 2] = rgbaBuf[bi + 2];
                bestColor[bi + 3] = rgbaBuf[bi + 3];
            }
        }
    }

    if (!gotAny) {
        return m_localScene->renderPng(width, height);
    }

    QImage img(bestColor.data(), width, height, QImage::Format_RGBA8888);
    QByteArray png;
    QBuffer buf(&png);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    const QString b64 = QString::fromLatin1(png.toBase64());
    return {{"status", QStringLiteral("ok")}, {"image", b64}};
}
