#include "mainwindow.h"
#include "singleton.h"

#include <QApplication>
#include <QLocale>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>
#include <QWebEngineUrlScheme>

#include <clocale>

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
    a.setApplicationVersion("1.5.3");
    a.setWindowIcon(QIcon(":/icons/logo_256.png"));

    setlocale(LC_NUMERIC, "C");
    QLocale::setDefault(QLocale::c());

    Singleton<MainWindow>::Instance();

    return a.exec();
}
