#include "vlvaurlschemehandler.h"

#include <QUrlQuery>
#include <QWebEngineUrlRequestJob>

VLVAUrlSchemeHandler::VLVAUrlSchemeHandler(QObject *parent) : QWebEngineUrlSchemeHandler(parent) { }

void VLVAUrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob *job)
{
    QUrlQuery query(job->requestUrl().query());

    QString code;
    foreach (auto q, query.queryItems()) {
        if (q.first.compare(QString("code")) == 0) {
            code = q.second;
            break;
        }
    }

    if (!code.isEmpty()) {
        emit codeReceived(code);
    }
}
