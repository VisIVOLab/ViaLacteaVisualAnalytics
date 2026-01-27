#include "VisIVOUrlSchemeHandler.h"

#include <QBuffer>
#include <QUrlQuery>
#include <QVariant>
#include <QWebEngineUrlRequestJob>

using namespace Qt::StringLiterals;

VisIVOUrlSchemeHandler::VisIVOUrlSchemeHandler(QObject *parent) : QWebEngineUrlSchemeHandler(parent)
{
}

void VisIVOUrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)
{
    const QUrlQuery urlQuery(request->requestUrl());
    QVariantMap data;
    for (const auto &[key, value] : urlQuery.queryItems()) {
        data.insert(key, value);
    }
    emit this->callbackReceived(data);

    auto buffer = new QBuffer(request);
    buffer->open(QIODevice::WriteOnly);
    buffer->write("<html><body>"
                  "Callback received. Feel free to close this page."
                  "</body></html>"_ba);
    buffer->close();

    request->reply("text/html; charset=utf-8"_ba, buffer);
}
