#include "worker_loop.h"
#include "scene_manager.h"

#include <mpi.h>
#include <vector>
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
            auto res = m_scene->renderPng(dims[0], dims[1]);
            QByteArray img = res.value(QStringLiteral("image")).toString().toLatin1();
            // Note: img is base64; send raw bytes of decoded base64 to reduce size
            QByteArray raw = QByteArray::fromBase64(img);
            int sz = raw.size();
            MPI_Send(&sz, 1, MPI_INT, 0, FRAME_TAG, MPI_COMM_WORLD);
            if (sz > 0) {
                MPI_Send(raw.data(), sz, MPI_CHAR, 0, FRAME_TAG + 1, MPI_COMM_WORLD);
            }
        }
    }
}
