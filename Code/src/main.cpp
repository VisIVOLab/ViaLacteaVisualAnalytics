#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>
#include <QWebEngineUrlScheme>

#include "mainwindow.h"
#include "singleton.h"

int main(int argc, char *argv[])
{
    QWebEngineUrlScheme vlvaUrlScheme("vlva");
    vlvaUrlScheme.setSyntax(QWebEngineUrlScheme::Syntax::Path);
    vlvaUrlScheme.setDefaultPort(QWebEngineUrlScheme::PortUnspecified);
    vlvaUrlScheme.setFlags(QWebEngineUrlScheme::SecureScheme);
    QWebEngineUrlScheme::registerScheme(vlvaUrlScheme);

    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication a(argc, argv);
    a.setApplicationName("Vialactea - Visual Analytics client");
    a.setApplicationVersion("1.4");
    a.setWindowIcon(QIcon(":/icons/logo_256.png"));

    Singleton<MainWindow>::Instance();

    return a.exec();
}
