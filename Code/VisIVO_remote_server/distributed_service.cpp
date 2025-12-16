#include "distributed_service.h"
#include "scene_manager.h"
#include "video_encoder.h"

#include <mpi.h>
#include <QJsonArray>
#include <QJsonValue>
#include <QStringLiteral>
#include <QByteArray>
#include <QImage>
#include <QBuffer>
#include <QDebug>

namespace {
enum CommandTag {
    CMD_NONE = 0,
    CMD_LOAD = 1,
    CMD_RENDER = 2,
    CMD_SET_SLICE = 3,
    CMD_SET_LUT = 4,
    CMD_SET_RANGE = 5,
    CMD_ROTATE = 6,
    CMD_PAN = 7,
    CMD_ZOOM = 8,
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
    qInfo() << "[DistributedService] rank" << m_rank << "loading dataset" << path;
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

QJsonObject DistributedService::setRange(double min, double max) {
    auto res = m_localScene->setRange(min, max);
    if (m_rank == 0 && m_size > 1) {
        int cmd = CMD_SET_RANGE;
        double range[2] = {min, max};
        MPI_Bcast(&cmd, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(range, 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    return res;
}

QJsonObject DistributedService::setWindowLevel(double window, double level) {
    auto res = m_localScene->setWindowLevel(window, level);
    return res;
}

QJsonObject DistributedService::setLut(const QJsonObject &params) {
    auto res = m_localScene->setLut(params);
    if (m_rank == 0 && m_size > 1) {
        int cmd = CMD_SET_LUT;
        MPI_Bcast(&cmd, 1, MPI_INT, 0, MPI_COMM_WORLD);
        QByteArray p = QJsonDocument(params).toJson(QJsonDocument::Compact);
        int len = p.size();
        MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (len > 0) {
            MPI_Bcast(p.data(), len, MPI_CHAR, 0, MPI_COMM_WORLD);
        }
    }
    return res;
}

QJsonObject DistributedService::rotateCamera(double yawDeg, double pitchDeg) {
    auto res = m_localScene->rotateCamera(yawDeg, pitchDeg);
    if (m_rank == 0 && m_size > 1) {
        int cmd = CMD_ROTATE;
        MPI_Bcast(&cmd, 1, MPI_INT, 0, MPI_COMM_WORLD);
        double vals[2] = {yawDeg, pitchDeg};
        MPI_Bcast(vals, 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    return res;
}

QJsonObject DistributedService::panCamera(double dx, double dy) {
    auto res = m_localScene->panCamera(dx, dy);
    if (m_rank == 0 && m_size > 1) {
        int cmd = CMD_PAN;
        MPI_Bcast(&cmd, 1, MPI_INT, 0, MPI_COMM_WORLD);
        double vals[2] = {dx, dy};
        MPI_Bcast(vals, 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    return res;
}

QJsonObject DistributedService::zoomCamera(double factor) {
    auto res = m_localScene->zoomCamera(factor);
    if (m_rank == 0 && m_size > 1) {
        int cmd = CMD_ZOOM;
        MPI_Bcast(&cmd, 1, MPI_INT, 0, MPI_COMM_WORLD);
        double val = factor;
        MPI_Bcast(&val, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    return res;
}

static QString encodeCompositeImage(const std::vector<unsigned char> &rgba, int width, int height, const QString &qualityTag) {
    QImage img(rgba.data(), width, height, QImage::Format_RGBA8888);
    QByteArray out;
    QBuffer buf(&out);
    buf.open(QIODevice::WriteOnly);
    const char *fmt = "PNG";
    int quality = -1;
    if (qualityTag.compare(QStringLiteral("interactive"), Qt::CaseInsensitive) == 0) {
        fmt = "JPEG";
        quality = 40;
    } else if (qualityTag.compare(QStringLiteral("final"), Qt::CaseInsensitive) == 0) {
        fmt = "JPEG";
        quality = 95;
    }
    img.save(&buf, fmt, quality);
    return QString::fromLatin1(out.toBase64());
}

QJsonObject DistributedService::renderFrame(int width, int height, const QString &mode, const QJsonObject &volumeParams, const QString &quality, const QString &format) {
    const bool wantH264 = format.compare(QStringLiteral("h264"), Qt::CaseInsensitive) == 0;
    // Riduci risoluzione durante l'interazione per fluidità
    int targetW = width;
    int targetH = height;
    int lod = 1;
    if (quality.compare(QStringLiteral("interactive"), Qt::CaseInsensitive) == 0) {
        targetW = std::max(320, width / 2);
        targetH = std::max(240, height / 2);
        lod = 2;
    }
    qInfo() << "[DistributedService] renderFrame rank" << m_rank
            << "mode" << mode << "req" << width << "x" << height
            << "target" << targetW << "x" << targetH
            << "quality" << quality << "lod" << lod
            << "format" << format;

    // H.264 compositing (slice/contour/volume) – stesso gather path di PNG, poi encode
    if (wantH264) {
        int cmd = CMD_RENDER;
        MPI_Bcast(&cmd, 1, MPI_INT, 0, MPI_COMM_WORLD);
        int dims[2] = {targetW, targetH};
        MPI_Bcast(dims, 2, MPI_INT, 0, MPI_COMM_WORLD);
        int modeInt = 0; // 0 slice, 1 volume, 2 contour
        if (mode == QStringLiteral("volume")) modeInt = 1;
        else if (mode == QStringLiteral("contour")) modeInt = 2;
        MPI_Bcast(&modeInt, 1, MPI_INT, 0, MPI_COMM_WORLD);
        QJsonObject volParams = volumeParams;
        if (lod > 1 && modeInt == 1 && !volParams.contains(QStringLiteral("lod"))) {
            volParams.insert(QStringLiteral("lod"), lod);
        }
        QByteArray paramJson;
        if (modeInt == 1 || modeInt == 2) {
            paramJson = QJsonDocument(volParams).toJson(QJsonDocument::Compact);
        }
        int paramLen = paramJson.size();
        MPI_Bcast(&paramLen, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (paramLen > 0) {
            MPI_Bcast(paramJson.data(), paramLen, MPI_CHAR, 0, MPI_COMM_WORLD);
        }

        const int pixelCount = targetW * targetH;
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
                if (d <= 0.0f) continue;
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
            // fallback locale in base al mode
            if (modeInt == 1)
                return m_localScene->renderH264Volume(targetW, targetH, volParams, quality);
            else if (modeInt == 2)
                return m_localScene->renderH264Contour(targetW, targetH, volParams, quality);
            return m_localScene->renderH264(targetW, targetH, quality);
        }

        QString err;
        QByteArray colorBytes(reinterpret_cast<const char *>(bestColor.data()),
                              static_cast<int>(bestColor.size()));
        QByteArray nal = VideoEncoder::encodeH264(colorBytes, targetW, targetH, quality, err);
        if (nal.isEmpty()) {
            return {{"_error", err.isEmpty() ? QStringLiteral("encode failed") : err}};
        }
        const QString b64 = QString::fromLatin1(nal.toBase64());
        return {{"status", QStringLiteral("ok")},
                {"codec", QStringLiteral("h264")},
                {"data", b64}};
    }
    // Modalità speciali: solo locale (no compositing per ora)
    if (mode == QStringLiteral("volume")) {
        QJsonObject v = volumeParams;
        if (lod > 1 && !v.contains(QStringLiteral("lod"))) v.insert(QStringLiteral("lod"), lod);
        return m_localScene->renderVolumePng(targetW, targetH, v, quality);
    }
    if (mode == QStringLiteral("contour")) {
        return m_localScene->renderContourPng(targetW, targetH, volumeParams, quality);
    }

    // Otherwise MPI compositing
    int cmd = CMD_RENDER;
    MPI_Bcast(&cmd, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int dims[2] = {targetW, targetH};
    MPI_Bcast(dims, 2, MPI_INT, 0, MPI_COMM_WORLD);
    int modeInt = 0;
    if (mode == QStringLiteral("volume")) modeInt = 1;
    else if (mode == QStringLiteral("contour")) modeInt = 2;
    MPI_Bcast(&modeInt, 1, MPI_INT, 0, MPI_COMM_WORLD);
    QJsonObject volParams = volumeParams;
    if (lod > 1 && modeInt == 1 && !volParams.contains(QStringLiteral("lod"))) {
        volParams.insert(QStringLiteral("lod"), lod);
    }
    QByteArray paramJson = QJsonDocument(volParams).toJson(QJsonDocument::Compact);
    int paramLen = paramJson.size();
    MPI_Bcast(&paramLen, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (paramLen > 0) {
        MPI_Bcast(paramJson.data(), paramLen, MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    const int pixelCount = targetW * targetH;
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
        // fallback locale
        if (modeInt == 1) return m_localScene->renderVolumePng(targetW, targetH, volParams, quality);
        if (modeInt == 2) return m_localScene->renderContourPng(targetW, targetH, volParams, quality);
        return m_localScene->renderPng(targetW, targetH, quality);
    }

    const QString b64 = encodeCompositeImage(bestColor, targetW, targetH, quality);
    return {{"status", QStringLiteral("ok")}, {"image", b64}};
}
