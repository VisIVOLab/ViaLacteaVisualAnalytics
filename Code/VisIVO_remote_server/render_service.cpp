#include "render_service.h"
#include "scene_manager.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QStringLiteral>

RenderService::RenderService()
    : m_scene(new SceneManager) {}

RenderService::~RenderService() {
    delete m_scene;
}

QJsonObject RenderService::handleRequest(const QJsonObject &request) {
    const QString method = request.value(QStringLiteral("method")).toString();
    const QJsonObject params = request.value(QStringLiteral("params")).toObject();

    if (method == QStringLiteral("ping")) {
        return {{"message", QStringLiteral("pong")}};
    }

    if (method == QStringLiteral("getCapabilities")) {
        QJsonArray methods;
        methods.append(QStringLiteral("ping"));
        methods.append(QStringLiteral("getCapabilities"));
        methods.append(QStringLiteral("loadDataset"));
        methods.append(QStringLiteral("setCamera"));
        methods.append(QStringLiteral("renderFrame"));
        return {
                {"protocol", QStringLiteral("jsonrpc2")},
                {"transport", QStringLiteral("websocket")},
                {"methods", methods},
                {"rendering", QStringLiteral("slice2d/offscreen")}
        };
    }

    if (method == QStringLiteral("loadDataset")) {
        const QString source = params.value(QStringLiteral("source")).toString();
        return m_scene->loadFits(source);
    }

    if (method == QStringLiteral("setCamera")) {
        return m_scene->setCamera(params);
    }

    if (method == QStringLiteral("renderFrame")) {
        const int w = params.value(QStringLiteral("width")).toInt(800);
        const int h = params.value(QStringLiteral("height")).toInt(600);
        return m_scene->renderPng(w, h);
    }

    return {{"_error", QStringLiteral("Unknown method")}};
}
