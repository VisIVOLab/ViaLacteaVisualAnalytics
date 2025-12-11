#pragma once

#include <QJsonObject>
#include <QJsonArray>

class DistributedService;

class RenderService {
public:
    RenderService(int rank, int size);
    ~RenderService();

    QJsonObject handleRequest(const QJsonObject &request);

private:
    DistributedService *m_service;
};
