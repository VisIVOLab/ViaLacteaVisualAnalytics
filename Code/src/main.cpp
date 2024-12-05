#include "mainwindow.h"
#include "singleton.h"
#include "version.h"

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
    a.setApplicationVersion(VLVA_VERSION_STR);
    a.setWindowIcon(QIcon(":/icons/visivo512.png"));

    std::setlocale(LC_ALL, "C");
    QLocale::setDefault(QLocale::c());

    Singleton<MainWindow>::Instance();

    return a.exec();
}
