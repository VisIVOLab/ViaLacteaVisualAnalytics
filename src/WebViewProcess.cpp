#include "WebViewProcess.h"

WebViewProcess::WebViewProcess(QObject *parent) : QObject(parent) { }

void WebViewProcess::jsCall(const QString &point, const QString &area)
{
    emit this->processJavascript(point, area);
}
