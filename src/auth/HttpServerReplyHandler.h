#ifndef HttpServerReplyHandler_H
#define HttpServerReplyHandler_H

#include <QOAuthHttpServerReplyHandler>

class HttpServerReplyHandler : public QOAuthHttpServerReplyHandler
{
    Q_OBJECT

public:
    explicit HttpServerReplyHandler(QObject *parent = nullptr);

    QString callback() const override;
};

#endif