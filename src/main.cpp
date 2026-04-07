#include "MainWindow.h"
#include "Version.h"

#include <QVTKOpenGLNativeWidget.h>

#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QLocale>
#include <QSurfaceFormat>
#include <QWebEngineUrlScheme>

#include <clocale>

using namespace Qt::StringLiterals;

std::string logFile{ };
QtMessageHandler originalHandler{ };

void logToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static FILE *f = fopen(logFile.c_str(), "a");
    fprintf(f, "%s\n", qPrintable(qFormatLogMessage(type, context, msg)));
    fflush(f);

    if (originalHandler) {
        originalHandler(type, context, msg);
    }
}

int main(int argc, char *argv[])
{
    // Register custom URL scheme needed for authentication
    QWebEngineUrlScheme scheme("vlva"_ba);
    scheme.setSyntax(QWebEngineUrlScheme::Syntax::Path);
    scheme.setDefaultPort(QWebEngineUrlScheme::PortUnspecified);
    scheme.setFlags(QWebEngineUrlScheme::SecureScheme);
    QWebEngineUrlScheme::registerScheme(scheme);

    // Enable OpenGL shared contexts
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    // Set OpenGL profile needed for VTK
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication app(argc, argv);

    // Set locale to C.UTF-8
    std::setlocale(LC_NUMERIC, "C.UTF-8");
    QLocale::setDefault(QLocale::C);

    // Set application info
    QApplication::setApplicationName(u"VisIVO Visual Analytics"_s);
    QApplication::setApplicationVersion(QStringLiteral(VISIVO_VERSION_STR));
    QApplication::setWindowIcon(QIcon(u":/icons/VisIVO_512.png"_s));
    QApplication::setOrganizationName(u"Osservatorio Astrofisico di Catania"_s);
    QApplication::setOrganizationDomain(u"it.inaf.oact"_s);

    // Customize QDebug message format
    qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss}"
                       "%{if-debug} DEBG%{endif}"
                       "%{if-info} INFO%{endif}"
                       "%{if-warning} WARN%{endif}"
                       "%{if-critical} CRIT%{endif}"
                       "%{if-fatal} FATL%{endif}"
#ifndef NDEBUG
                       " %{function}:%{line}"
#endif
                       "%{if-category} %{category}%{endif}"
                       "] %{message}");

    // Setup logging to file
    const QDir appDir = QDir::home().absoluteFilePath(qApp->applicationName());
    if (appDir.mkpath(u"logs"_s)) {
        const QString timestamp = QDateTime::currentDateTime().toString(u"yyyy-MM-dd_hh.mm.ss"_s);
        logFile = appDir.absoluteFilePath("logs/"_L1 + timestamp + ".log"_L1).toStdString();
        originalHandler = qInstallMessageHandler(logToFile);
    }

    MainWindow w;
    w.show();

    return QApplication::exec();
}
