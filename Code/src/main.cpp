#include "mainwindow.h"
#include <QApplication>
#include "singleton.h"
#include <QSplashScreen>
#include <QBitmap>
#include <QDebug>
//#include "vosamp.h"
#include <QSurfaceFormat>
#include "QVTKOpenGLNativeWidget.h"
#include <QWebEngineUrlScheme>

int main(int argc, char *argv[])
{
    QWebEngineUrlScheme vlvaUrlScheme("vlva");
    vlvaUrlScheme.setSyntax(QWebEngineUrlScheme::Syntax::Path);
    vlvaUrlScheme.setDefaultPort(QWebEngineUrlScheme::PortUnspecified);
    vlvaUrlScheme.setFlags(QWebEngineUrlScheme::SecureScheme);
    QWebEngineUrlScheme::registerScheme(vlvaUrlScheme);

    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication a(argc, argv);
    MainWindow *w = &Singleton<MainWindow>::Instance();
    a.setApplicationName("Vialactea - Visual Analytics client");


 //  vosamp *samp = &Singleton<vosamp>::Instance();

   // w->show();


    return a.exec();

}
