#include "mainwindow.h"
#include "singleton.h"

#include <QApplication>
#include <QLocale>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>
#include <QWebEngineUrlScheme>

#include <clocale>

#include <QSurfaceFormat>
#include "pqPVApplicationCore.h"

int main(int argc, char *argv[])
{
    QWebEngineUrlScheme vlvaUrlScheme("vlva");
    vlvaUrlScheme.setSyntax(QWebEngineUrlScheme::Syntax::Path);
    vlvaUrlScheme.setDefaultPort(QWebEngineUrlScheme::PortUnspecified);
    vlvaUrlScheme.setFlags(QWebEngineUrlScheme::SecureScheme);
    QWebEngineUrlScheme::registerScheme(vlvaUrlScheme);

    //QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
    QSurfaceFormat glFormat;
    glFormat.setVersion(3, 3);
    glFormat.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(glFormat);
    
    QApplication a(argc, argv);
    
    //paraview init
    pqPVApplicationCore appCore(argc, argv);
  
    a.setApplicationName("Vialactea - Visual Analytics client");
    a.setApplicationVersion("1.6-beta.1");
    a.setWindowIcon(QIcon(":/icons/logo_256.png"));

    setlocale(LC_NUMERIC, "C");
    QLocale::setDefault(QLocale::c());

    Singleton<MainWindow>::Instance();

    return a.exec();
}
