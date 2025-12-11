#include "render_service.h"
#include "distributed_service.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QStringLiteral>

RenderService::RenderService(int rank, int size)
    : m_service(new DistributedService(rank, size)) {}

RenderService::~RenderService() {
    delete m_service;
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
        methods.append(QStringLiteral("setSlice"));
        methods.append(QStringLiteral("setWindowLevel"));
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
        return m_service->loadDataset(source);
    }

    if (method == QStringLiteral("setCamera")) {
        return m_service->setCamera(params);
    }

    if (method == QStringLiteral("setSlice")) {
        const int slice = params.value(QStringLiteral("slice")).toInt();
        return m_service->setSlice(slice);
    }

    if (method == QStringLiteral("setWindowLevel")) {
        const double window = params.value(QStringLiteral("window")).toDouble();
        const double level = params.value(QStringLiteral("level")).toDouble();
        return m_service->setWindowLevel(window, level);
    }

    if (method == QStringLiteral("renderFrame")) {
        const int w = params.value(QStringLiteral("width")).toInt(800);
        const int h = params.value(QStringLiteral("height")).toInt(600);
        const QString mode = params.value(QStringLiteral("mode")).toString();
        const QJsonObject vol = params.value(QStringLiteral("volume")).toObject();
        const QJsonObject contour = params.value(QStringLiteral("contour")).toObject();
        if (mode == QStringLiteral("contour")) {
            return m_service->renderFrame(w, h, mode, contour);
        }
        return m_service->renderFrame(w, h, mode, vol);
    }

    return {{"_error", QStringLiteral("Unknown method")}};
}
