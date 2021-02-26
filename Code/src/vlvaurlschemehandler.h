#ifndef VLVAURLSCHEMEHANDLER_H
#define VLVAURLSCHEMEHANDLER_H

#include <QWebEngineUrlSchemeHandler>

class VLVAUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT
public:
    VLVAUrlSchemeHandler(QObject *parent = nullptr);
    void requestStarted(QWebEngineUrlRequestJob *job);

signals:
    void codeReceived(const QString &code);
};

#endif // VLVAURLSCHEMEHANDLER_H
