#include "worker_loop.h"
#include "scene_manager.h"

#include <mpi.h>
#include <vector>
#include <QJsonDocument>
#include <QByteArray>

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

WorkerLoop::WorkerLoop(int rank, int size)
    : m_rank(rank), m_size(size), m_scene(new SceneManager) {}

WorkerLoop::~WorkerLoop() { delete m_scene; }

void WorkerLoop::run() {
    int cmd = CMD_NONE;
    while (true) {
        MPI_Bcast(&cmd, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (cmd == CMD_EXIT) {
            break;
        } else if (cmd == CMD_LOAD) {
            int len = 0;
            MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
            std::vector<char> buf(static_cast<size_t>(len));
            if (len > 0) MPI_Bcast(buf.data(), len, MPI_CHAR, 0, MPI_COMM_WORLD);
            QString path = QString::fromStdString(std::string(buf.data(), buf.size()));
            auto res = m_scene->loadFits(path);
            int status = res.contains("_error") ? -1 : 0;
            MPI_Send(&status, 1, MPI_INT, 0, CMD_LOAD, MPI_COMM_WORLD);
        } else if (cmd == CMD_RENDER) {
            int dims[2] = {800, 600};
            MPI_Bcast(dims, 2, MPI_INT, 0, MPI_COMM_WORLD);
            int modeInt = 0; // 0 slice, 1 volume, 2 contour
            MPI_Bcast(&modeInt, 1, MPI_INT, 0, MPI_COMM_WORLD);
            int paramLen = 0;
            MPI_Bcast(&paramLen, 1, MPI_INT, 0, MPI_COMM_WORLD);
            QByteArray rgba, depth;
            QJsonObject params;
            if (paramLen > 0) {
                std::vector<char> pbuf(static_cast<size_t>(paramLen));
                MPI_Bcast(pbuf.data(), paramLen, MPI_CHAR, 0, MPI_COMM_WORLD);
                params = QJsonDocument::fromJson(QByteArray(pbuf.data(), paramLen)).object();
            }
            QJsonObject res;
            if (modeInt == 1) {
                res = m_scene->renderRawVolume(dims[0], dims[1], params, rgba, depth);
            } else if (modeInt == 2) {
                res = m_scene->renderRawContour(dims[0], dims[1], params, rgba, depth);
            } else {
                res = m_scene->renderRaw(dims[0], dims[1], rgba, depth);
            }
            int rgbaSize = rgba.size();
            int depthSize = depth.size();
            MPI_Send(&rgbaSize, 1, MPI_INT, 0, FRAME_TAG, MPI_COMM_WORLD);
            MPI_Send(&depthSize, 1, MPI_INT, 0, FRAME_TAG + 1, MPI_COMM_WORLD);
            if (rgbaSize > 0) {
                MPI_Send(rgba.data(), rgbaSize, MPI_CHAR, 0, FRAME_TAG + 2, MPI_COMM_WORLD);
            }
            if (depthSize > 0) {
                MPI_Send(depth.data(), depthSize, MPI_CHAR, 0, FRAME_TAG + 3, MPI_COMM_WORLD);
            }
        } else if (cmd == CMD_SET_SLICE) {
            int slice = 0;
            MPI_Bcast(&slice, 1, MPI_INT, 0, MPI_COMM_WORLD);
            m_scene->setSlice(slice);
        }
    }
}
