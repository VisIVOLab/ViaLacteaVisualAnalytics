#ifndef VisIVOUrlSchemeHandler_h
#define VisIVOUrlSchemeHandler_h

#include <QWebEngineUrlSchemeHandler>

class VisIVOUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT

public:
    explicit VisIVOUrlSchemeHandler(QObject *parent = nullptr);

    void requestStarted(QWebEngineUrlRequestJob *request) override;

signals:
    void callbackReceived(const QVariantMap &data);
};

#endif
