#pragma once

#include <QObject>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QSet>

class RenderService;

class RpcServer : public QObject {
    Q_OBJECT
public:
    explicit RpcServer(quint16 port, QObject *parent = nullptr);
    bool start();

signals:
    void fatalError(const QString &message);

private slots:
    void onNewConnection();
    void onTextMessageReceived(const QString &message);
    void onSocketDisconnected();

private:
    QWebSocketServer m_server;
    QSet<QWebSocket *> m_clients;
    quint16 m_port;
    RenderService *m_service;

    void sendError(QWebSocket *socket, const QJsonValue &id, const QString &message);
    void sendResult(QWebSocket *socket, const QJsonValue &id, const QJsonObject &result);
};
