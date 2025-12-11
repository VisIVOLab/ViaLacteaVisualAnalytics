#pragma once

#include <QJsonObject>
#include <QJsonArray>

class SceneManager;

class RenderService {
public:
    RenderService();
    ~RenderService();

    QJsonObject handleRequest(const QJsonObject &request);

private:
    SceneManager *m_scene;
};
