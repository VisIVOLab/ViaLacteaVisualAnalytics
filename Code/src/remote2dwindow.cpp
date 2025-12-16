#include "remote2dwindow.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QBuffer>
#include <QPixmap>
#include <QtMath>
#include <QElapsedTimer>
#include <QDebug>

Remote2DWindow::Remote2DWindow(const QUrl &serverUrl, QWidget *parent)
    : QWidget(parent), m_server(serverUrl) {
    auto *layout = new QVBoxLayout(this);
    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setMinimumSize(320, 240);
    setMouseTracking(true);
    m_label->setMouseTracking(true);
    layout->addWidget(m_label);
    setLayout(layout);
}

Remote2DWindow::~Remote2DWindow() {
    m_ws.abort();
}

bool Remote2DWindow::connectServer() {
    if (m_ws.state() == QAbstractSocket::ConnectedState) return true;
    qInfo() << "[Remote2DWindow] Connecting to" << m_server;
    QEventLoop loop;
    connect(&m_ws, &QWebSocket::connected, &loop, &QEventLoop::quit);
    connect(&m_ws, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            &loop, &QEventLoop::quit);
    connect(&m_ws, &QWebSocket::binaryMessageReceived, this, &Remote2DWindow::handleBinaryMessage);
    m_ws.open(m_server);
    QTimer::singleShot(3000, &loop, &QEventLoop::quit);
    loop.exec();
    const bool ok = m_ws.state() == QAbstractSocket::ConnectedState;
    if (ok) {
        qInfo() << "[Remote2DWindow] Connected to" << m_server;
    } else {
        qWarning() << "[Remote2DWindow] Connection failed to" << m_server;
    }
    return ok;
}

QJsonObject Remote2DWindow::sendRequest(const QJsonObject &req, int timeoutMs) {
    if (!connectServer()) {
        emit error(tr("Cannot connect to %1").arg(m_server.toString()));
        return {{"_error", QStringLiteral("connect failed")}};
    }
    const QString method = req.value(QStringLiteral("method")).toString();
    qInfo() << "[Remote2DWindow] ->" << method << req.value(QStringLiteral("params")).toObject();
    QEventLoop loop;
    QJsonObject reply;
    auto h = connect(&m_ws, &QWebSocket::textMessageReceived, this, [&](const QString &msg) {
        reply = QJsonDocument::fromJson(msg.toUtf8()).object();
        loop.quit();
    });
    m_ws.sendTextMessage(QString::fromUtf8(QJsonDocument(req).toJson(QJsonDocument::Compact)));
    QTimer::singleShot(timeoutMs, &loop, &QEventLoop::quit);
    loop.exec();
    disconnect(h);
    if (reply.isEmpty()) {
        emit error(tr("Request timeout"));
        qWarning() << "[Remote2DWindow] <- timeout for" << method;
        return {{"_error", QStringLiteral("timeout")}};
    }
    qInfo() << "[Remote2DWindow] <-" << method << reply;
    return reply.value(QStringLiteral("result")).toObject();
}

bool Remote2DWindow::updateImageFromReply(const QJsonObject &reply) {
    if (reply.contains(QStringLiteral("_error"))) {
        emit error(reply.value(QStringLiteral("_error")).toString());
        return false;
    }
    const QString b64 = reply.value(QStringLiteral("image")).toString();
    if (b64.isEmpty()) return false;
    const QByteArray png = QByteArray::fromBase64(b64.toLatin1());
    QImage img;
    if (!img.loadFromData(png, "PNG")) {
        emit error(tr("Failed to decode image"));
        return false;
    }
    QImage out = applyLut(img);
    m_label->setPixmap(QPixmap::fromImage(out));
    m_lastWidth = img.width();
    m_lastHeight = img.height();
    emit imageUpdated(out);
    return true;
}

bool Remote2DWindow::loadFits(const QString &path) {
    qInfo() << "[Remote2DWindow] Loading FITS" << path;
    QJsonObject req{
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "loadDataset"},
        {"params", QJsonObject{{"source", path}}}
    };
    auto res = sendRequest(req, 120000); // large cubes may take a while to load
    if (res.contains(QStringLiteral("_error"))) {
        qWarning() << "[Remote2DWindow] loadDataset error" << res;
        return false;
    }

    // Aggiorna range e slice iniziale come nel case 0 di vtkwindow_new
    const QJsonArray dims = res.value(QStringLiteral("dimensions")).toArray();
    const QJsonArray range = res.value(QStringLiteral("range")).toArray();
    qInfo() << "[Remote2DWindow] FITS dimensions" << dims << "range" << range;
    if (range.size() == 2) {
        const double rmin = range[0].toDouble();
        const double rmax = range[1].toDouble();
        setRange(rmin, rmax);
        double window = rmax - rmin;
        double level = rmin + 0.5 * window;
        setWindowLevelHint(window, level);
    }
    return true;
}

bool Remote2DWindow::setSlice(int slice) {
    QJsonObject req{
        {"jsonrpc", "2.0"},
        {"id", 2},
        {"method", "setSlice"},
        {"params", QJsonObject{{"slice", slice}}}
    };
    auto res = sendRequest(req);
    if (res.contains(QStringLiteral("_error"))) return false;
    return renderSlice(m_lastWidth, m_lastHeight);
}

bool Remote2DWindow::setWindowLevel(double window, double level) {
    QJsonObject req{
        {"jsonrpc", "2.0"},
        {"id", 3},
        {"method", "setWindowLevel"},
        {"params", QJsonObject{{"window", window}, {"level", level}}}
    };
    auto res = sendRequest(req);
    if (res.contains(QStringLiteral("_error"))) return false;
    // Non renderizziamo automaticamente per evitare frame “flash” indesiderati:
    // il chiamante decide quando richiedere un nuovo frame (slice/volume/contour).
    return true;
}

bool Remote2DWindow::renderSlice(int width, int height, const QString &quality) {
    if (width <= 0) width = m_label->width();
    if (height <= 0) height = m_label->height();
    const int timeoutMs = (quality == QStringLiteral("interactive")) ? 15000 : 60000;
    QJsonObject req{
        {"jsonrpc", "2.0"},
        {"id", 4},
        {"method", "renderFrame"},
        {"params", QJsonObject{{"width", width}, {"height", height},
                               {"quality", quality},
                               {"format", QStringLiteral("h264")}}}
    };
    auto res = sendRequest(req, timeoutMs);
    // Se h264, decodifica non implementata: fallback al campo "data" base64 NAL
    const QString codec = res.value(QStringLiteral("codec")).toString();
    if (codec == QStringLiteral("h264")) {
        return true; // frame arriverà via binaryMessageReceived
    }
    return updateImageFromReply(res);
}

void Remote2DWindow::setLutType(const QString &type) {
    m_lutType = type;
    QJsonObject req{
        {"jsonrpc", "2.0"},
        {"id", 6},
        {"method", "setLut"},
        {"params", QJsonObject{{"type", m_lutType}, {"scale", m_lutScale}}}
    };
    sendRequest(req);
}

void Remote2DWindow::setLutScale(const QString &scale) {
    m_lutScale = scale;
    QJsonObject req{
        {"jsonrpc", "2.0"},
        {"id", 6},
        {"method", "setLut"},
        {"params", QJsonObject{{"type", m_lutType}, {"scale", m_lutScale}}}
    };
    sendRequest(req);
}

void Remote2DWindow::setWindowLevelHint(double window, double level) {
    m_hintWindow = window;
    m_hintLevel = level;
    if (window > 0) {
        setWindowLevel(window, level);
    }
}

void Remote2DWindow::setRange(double min, double max) {
    m_rangeMin = min;
    m_rangeMax = max;
    QJsonObject req{
        {"jsonrpc", "2.0"},
        {"id", 7},
        {"method", "setRange"},
        {"params", QJsonObject{{"min", min}, {"max", max}}}
    };
    sendRequest(req);
}

QImage Remote2DWindow::applyLut(const QImage &src) const {
    // LUT gestita dal server: restituiamo l'immagine così com'è
    Q_UNUSED(m_rangeMin);
    Q_UNUSED(m_rangeMax);
    Q_UNUSED(m_lutType);
    Q_UNUSED(m_lutScale);
    return src;
}

void Remote2DWindow::handleBinaryMessage(const QByteArray &bin) {
    // Header: 'H264' + uint32 width + uint32 height (big endian)
    qInfo() << "[Remote2DWindow] binary message size" << bin.size();
    if (bin.size() < 12) return;
    if (!(bin[0] == 'H' && bin[1] == '2' && bin[2] == '6' && bin[3] == '4')) return;
    const auto be32 = [](const uchar *p) -> int {
        return (int(p[0]) << 24) | (int(p[1]) << 16) | (int(p[2]) << 8) | int(p[3]);
    };
    const int w = be32(reinterpret_cast<const uchar *>(bin.constData() + 4));
    const int h = be32(reinterpret_cast<const uchar *>(bin.constData() + 8));
    const QByteArray nal = bin.mid(12);
    QImage img;
    if (m_decoder.decode(nal, img)) {
        m_lastWidth = w > 0 ? w : img.width();
        m_lastHeight = h > 0 ? h : img.height();
        m_label->setPixmap(QPixmap::fromImage(img));
        emit imageUpdated(img);
        qInfo() << "[Remote2DWindow] received H264 frame" << m_lastWidth << "x" << m_lastHeight;
    } else {
        qWarning() << "[Remote2DWindow] failed to decode H264 frame (size" << nal.size() << ")";
    }
}

bool Remote2DWindow::renderContour(const QJsonObject &params, int width, int height, const QString &quality) {
    if (width <= 0) width = m_label->width();
    if (height <= 0) height = m_label->height();
    const int timeoutMs = (quality == QStringLiteral("interactive")) ? 15000 : 60000;
    QJsonObject req{
        {"jsonrpc", "2.0"},
        {"id", 5},
        {"method", "renderFrame"},
        {"params", QJsonObject{
            {"mode", "contour"},
            {"width", width},
            {"height", height},
            {"contour", params},
            {"quality", quality},
            {"format", QStringLiteral("h264")}
        }}
    };
    qInfo() << "[Remote2DWindow] renderContour quality" << quality;
    auto res = sendRequest(req, timeoutMs);
    const QString codec = res.value(QStringLiteral("codec")).toString();
    if (codec == QStringLiteral("h264")) {
        return true; // frame via binary
    }
    return updateImageFromReply(res);
}

bool Remote2DWindow::renderVolume(const QJsonObject &params, int width, int height, const QString &quality) {
    if (width <= 0) width = m_label->width();
    if (height <= 0) height = m_label->height();
    const int timeoutMs = (quality == QStringLiteral("interactive")) ? 60000 : 180000;
    m_lastMode = QStringLiteral("volume");
    m_lastVolumeParams = params;
    qInfo() << "[Remote2DWindow] renderVolume quality" << quality << "params" << params
            << "size" << width << "x" << height;
    QJsonObject req{
        {"jsonrpc", "2.0"},
        {"id", 8},
        {"method", "renderFrame"},
        {"params", QJsonObject{
            {"mode", "volume"},
            {"width", width},
            {"height", height},
            {"volume", params},
            {"quality", quality},
            {"format", QStringLiteral("h264")}
        }}
    };
    auto res = sendRequest(req, timeoutMs);
    const QString codec = res.value(QStringLiteral("codec")).toString();
    if (codec == QStringLiteral("h264")) {
        return true; // frame via binary
    }
    return updateImageFromReply(res);
}

bool Remote2DWindow::rotateCamera(double yawDeg, double pitchDeg) {
    if (!connectServer()) return false;
    QJsonObject req{
        {"jsonrpc", "2.0"},
        {"id", 9},
        {"method", "rotateCamera"},
        {"params", QJsonObject{{"yaw", yawDeg}, {"pitch", pitchDeg}}}
    };
    m_ws.sendTextMessage(QString::fromUtf8(QJsonDocument(req).toJson(QJsonDocument::Compact)));
    scheduleInteractiveRender();
    return true;
}

bool Remote2DWindow::panCamera(double dx, double dy) {
    if (!connectServer()) return false;
    QJsonObject req{
        {"jsonrpc", "2.0"},
        {"id", 10},
        {"method", "panCamera"},
        {"params", QJsonObject{{"dx", dx}, {"dy", dy}}}
    };
    m_ws.sendTextMessage(QString::fromUtf8(QJsonDocument(req).toJson(QJsonDocument::Compact)));
    scheduleInteractiveRender();
    return true;
}

bool Remote2DWindow::zoomCamera(double factor) {
    if (!connectServer()) return false;
    QJsonObject req{
        {"jsonrpc", "2.0"},
        {"id", 11},
        {"method", "zoomCamera"},
        {"params", QJsonObject{{"factor", factor}}}
    };
    m_ws.sendTextMessage(QString::fromUtf8(QJsonDocument(req).toJson(QJsonDocument::Compact)));
    scheduleInteractiveRender();
    return true;
}

void Remote2DWindow::mousePressEvent(QMouseEvent *evt) {
    m_dragButton = evt->button();
    m_lastPos = evt->pos();
    m_dragging = true;
}

void Remote2DWindow::mouseMoveEvent(QMouseEvent *evt) {
    if (!m_dragging) return;
    QPoint delta = evt->pos() - m_lastPos;
    m_lastPos = evt->pos();
    if (m_lastMode == QStringLiteral("volume")) {
        if (m_dragButton == Qt::LeftButton) {
            rotateCamera(delta.x() * 0.5, delta.y() * 0.5);
        } else if (m_dragButton == Qt::RightButton || m_dragButton == Qt::MiddleButton) {
            panCamera(delta.x() * 0.01, -delta.y() * 0.01);
        }
    }
}

void Remote2DWindow::mouseReleaseEvent(QMouseEvent *evt) {
    Q_UNUSED(evt);
    m_dragging = false;
    m_dragButton = Qt::NoButton;
}

void Remote2DWindow::wheelEvent(QWheelEvent *evt) {
    if (m_lastMode == QStringLiteral("volume")) {
        double numDeg = evt->angleDelta().y() / 8.0;
        double numSteps = numDeg / 15.0;
        double factor = std::pow(1.1, numSteps);
        zoomCamera(factor);
    }
    evt->accept();
}

void Remote2DWindow::scheduleInteractiveRender() {
    if (m_lastMode != QStringLiteral("volume")) return;
    if (!m_lastRenderTimer.isValid()) {
        m_lastRenderTimer.start();
    }
    // Throttle a ~60 ms
    if (m_lastRenderTimer.elapsed() < 60 && m_renderScheduled) return;
    m_renderScheduled = true;
    QTimer::singleShot(30, this, [this]() {
        m_renderScheduled = false;
        m_lastRenderTimer.restart();
        renderVolume(m_lastVolumeParams, m_lastWidth, m_lastHeight, QStringLiteral("interactive"));
    });
}
