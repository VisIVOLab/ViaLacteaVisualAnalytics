#include <pybind11/embed.h>

#include "mainwindow.h"
#include "singleton.h"
#include "version.h"
#include "qdetachtabwidget.h"
#include "vialactea.h"

#include <QApplication>
#include <QLocale>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>
#include <QWebEngineUrlScheme>

#include <clocale>

namespace py = pybind11;

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

    // Init python interpreter
    QDir appDir(QApplication::applicationDirPath());
    py::scoped_interpreter guard;
    py::module_ sys = py::module_::import("sys");
    py::module_ site = py::module_::import("site");
    sys.attr("path").attr("append")(appDir.absolutePath().toStdString());
    sys.attr("path").attr("append")(appDir.absoluteFilePath("site-packages").toStdString());
    sys.attr("path").attr("append")(site.attr("getusersitepackages")());

    std::setlocale(LC_ALL, "C");
    QLocale::setDefault(QLocale::c());

    Singleton<MainWindow>::Instance();

    QDetachTabWidget* tabWidget = &Singleton<QDetachTabWidget>::Instance();
    tabWidget->setMovable(true);
    tabWidget->setTabsClosable(true);

    tabWidget->addTab(Singleton<ViaLactea>::Instance().centralWidget() , "Main Window");
    tabWidget->show();
    
    return a.exec();
}
