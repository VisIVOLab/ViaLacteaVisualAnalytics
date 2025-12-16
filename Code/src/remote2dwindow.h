#pragma once

#include <QWidget>
#include <QImage>
#include <QJsonObject>
#include <QUrl>
#include <QWebSocket>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QElapsedTimer>
#include <QPoint>
#include "h264decoder.h"

class QLabel;
class QEventLoop;

// Semplice viewer 2D che usa VisIVO_remote_server via WebSocket JSON-RPC.
// È pensato per rimpiazzare gradualmente il case 0 locale.
class Remote2DWindow : public QWidget {
    Q_OBJECT
public:
    explicit Remote2DWindow(const QUrl &serverUrl, QWidget *parent = nullptr);
    ~Remote2DWindow() override;

    bool connectServer();
    bool loadFits(const QString &path);
    bool setSlice(int slice);
    bool setWindowLevel(double window, double level);
    bool renderSlice(int width = 0, int height = 0, const QString &quality = QString());

    // Modalità contour: levels (double) e colore RGB opzionale.
    bool renderContour(const QJsonObject &params, int width = 0, int height = 0, const QString &quality = QString());
    // Modalità volume: iso-surface marching cubes (params: iso, color [r,g,b], opacity).
    bool renderVolume(const QJsonObject &params, int width = 0, int height = 0, const QString &quality = QString());
    // Controlli camera base
    bool rotateCamera(double yawDeg, double pitchDeg);
    bool panCamera(double dx, double dy);
    bool zoomCamera(double factor);

    // LUT / scala (per ora solo Gray + Log/Linear)
    void setLutType(const QString &type);   // es. "Gray"
    void setLutScale(const QString &scale); // "Log" o "Linear"
    void setWindowLevelHint(double window, double level);
    void setRange(double min, double max);

signals:
    void imageUpdated(const QImage &img);
    void error(const QString &msg);

protected:
    void mousePressEvent(QMouseEvent *evt) override;
    void mouseMoveEvent(QMouseEvent *evt) override;
    void mouseReleaseEvent(QMouseEvent *evt) override;
    void wheelEvent(QWheelEvent *evt) override;
    void scheduleInteractiveRender();

private:
    void handleBinaryMessage(const QByteArray &bin);
    QJsonObject sendRequest(const QJsonObject &req, int timeoutMs = 120000);
    bool updateImageFromReply(const QJsonObject &reply);
    QImage applyLut(const QImage &src) const;

    QWebSocket m_ws;
    QUrl m_server;
    QLabel *m_label;
    H264Decoder m_decoder;
    int m_lastWidth{800};
    int m_lastHeight{600};
    QString m_lutType{"Gray"};
    QString m_lutScale{"Log"};
    double m_rangeMin{0.0};
    double m_rangeMax{255.0};
    double m_hintWindow{-1.0};
    double m_hintLevel{-1.0};
    QString m_lastMode;
    QJsonObject m_lastVolumeParams;
    QPoint m_lastPos;
    Qt::MouseButton m_dragButton{Qt::NoButton};
    bool m_dragging{false};
    bool m_renderScheduled{false};
    QElapsedTimer m_lastRenderTimer;
};
