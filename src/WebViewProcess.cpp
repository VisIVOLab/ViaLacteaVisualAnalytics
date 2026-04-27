#include "WebViewProcess.h"

using namespace Qt::StringLiterals;

const QString WebViewProcess::ActivatePointSelection = u"activatePointSelection(%1)"_s;
const QString WebViewProcess::ActivateRectangularSelection = u"activateRectangularSelection(%1)"_s;

WebViewProcess::WebViewProcess(QObject *parent) : QObject(parent) { }

void WebViewProcess::jsCall(const QString &point, const QString &area)
{
    emit this->processJavascript(point, area);
}
