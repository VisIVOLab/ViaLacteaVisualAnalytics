#include <pybind11/embed.h>

#include "mainwindow.h"
#include "singleton.h"
#include "version.h"

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

    setlocale(LC_NUMERIC, "C");
    QLocale::setDefault(QLocale::c());

    QApplication a(argc, argv);
    a.setApplicationName("Vialactea - Visual Analytics client");
    a.setApplicationVersion(VLVA_VERSION_STR);
    a.setWindowIcon(QIcon(":/icons/logo_256.png"));

    // Init python interpreter
    py::scoped_interpreter guard;
    py::module_ sys = py::module_::import("sys");
    sys.attr("path").attr("append")(QApplication::applicationDirPath().toStdString());

    Singleton<MainWindow>::Instance();

    return a.exec();
}
