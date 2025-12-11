#include "rpc_server.h"
#include "render_service.h"

#include <QJsonParseError>
#include <QHostAddress>
#include <QByteArray>

RpcServer::RpcServer(quint16 port, int rank, int size, QObject *parent)
    : QObject(parent),
      m_server(QStringLiteral("VisIVO Remote Server"), QWebSocketServer::NonSecureMode),
      m_clients(),
      m_port(port),
      m_service(new RenderService(rank, size)) {
}

bool RpcServer::start() {
    const bool ok = m_server.listen(QHostAddress::Any, m_port);
    if (!ok) {
        emit fatalError(QStringLiteral("Unable to start WebSocket server on port %1: %2")
                                .arg(m_port)
                                .arg(m_server.errorString()));
        return false;
    }
    connect(&m_server, &QWebSocketServer::newConnection, this, &RpcServer::onNewConnection);
    return true;
}

void RpcServer::onNewConnection() {
    auto *socket = m_server.nextPendingConnection();
    if (!socket) {
        return;
    }

    m_clients.insert(socket);
    connect(socket, &QWebSocket::textMessageReceived, this, &RpcServer::onTextMessageReceived);
    connect(socket, &QWebSocket::disconnected, this, &RpcServer::onSocketDisconnected);
}

void RpcServer::onTextMessageReceived(const QString &message) {
    auto *socket = qobject_cast<QWebSocket *>(sender());
    if (!socket) return;

    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        sendError(socket, QJsonValue(), QStringLiteral("Invalid JSON"));
        return;
    }

    const QJsonObject request = doc.object();
    QJsonObject response;
    response["jsonrpc"] = QStringLiteral("2.0");
    response["id"] = request.value("id");

    const QJsonObject result = m_service->handleRequest(request);
    if (result.contains(QStringLiteral("_error"))) {
        sendError(socket, request.value("id"), result.value(QStringLiteral("_error")).toString());
    } else {
        sendResult(socket, request.value("id"), result);
    }
}

void RpcServer::onSocketDisconnected() {
    auto *socket = qobject_cast<QWebSocket *>(sender());
    if (!socket) return;
    m_clients.remove(socket);
    socket->deleteLater();
}

void RpcServer::sendError(QWebSocket *socket, const QJsonValue &id, const QString &message) {
    if (!socket) return;
    QJsonObject obj;
    obj["jsonrpc"] = QStringLiteral("2.0");
    obj["id"] = id;
    QJsonObject err;
    err["code"] = -32603;
    err["message"] = message;
    obj["error"] = err;
    socket->sendTextMessage(QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact)));
}

void RpcServer::sendResult(QWebSocket *socket, const QJsonValue &id, const QJsonObject &result) {
    if (!socket) return;
    QJsonObject obj;
    obj["jsonrpc"] = QStringLiteral("2.0");
    obj["id"] = id;
    obj["result"] = result;
    socket->sendTextMessage(QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact)));
}
