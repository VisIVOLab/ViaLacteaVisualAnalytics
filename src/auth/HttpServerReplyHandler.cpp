#include "HttpServerReplyHandler.h"

using namespace Qt::StringLiterals;

HttpServerReplyHandler::HttpServerReplyHandler(QObject *parent)
    : QOAuthHttpServerReplyHandler(parent)
{
}

QString HttpServerReplyHandler::callback() const
{
    return u"vlva://callback"_s;
}
