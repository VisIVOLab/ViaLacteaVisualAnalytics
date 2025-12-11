#pragma once

#include <QString>

class SceneManager;

class WorkerLoop {
public:
    WorkerLoop(int rank, int size);
    ~WorkerLoop();

    // Blocking loop; returns when receives CMD_EXIT.
    void run();

private:
    int m_rank;
    int m_size;
    SceneManager *m_scene;
};
