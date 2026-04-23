#include "Logging.h"
#include "MainWindow.h"
#include "Version.h"

#include <QVTKOpenGLNativeWidget.h>
#include <vtkLogger.h>

#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QLocale>
#include <QSurfaceFormat>
#include <QWebEngineUrlScheme>

#include <clocale>

using namespace Qt::StringLiterals;

static std::string logFile{ };
static QtMessageHandler originalHandler{ };

static void logToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static FILE *f = fopen(logFile.c_str(), "a");
    fprintf(f, "%s\n", qPrintable(qFormatLogMessage(type, context, msg)));
    fflush(f);

    if (originalHandler) {
        originalHandler(type, context, msg);
    }
}

static void vtkLogCallback(void *vtkNotUsed(user_data), const vtkLogger::Message &message)
{
    switch (message.verbosity) {
    case vtkLogger::VERBOSITY_ERROR:
        qCCritical(logVtk) << message.message;
        break;
    case vtkLogger::VERBOSITY_WARNING:
        qCWarning(logVtk) << message.message;
        break;
    case vtkLogger::VERBOSITY_INFO:
        qCInfo(logVtk) << message.message;
        break;
    default:
        qCDebug(logVtk) << message.message;
        break;
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
    QApplication::setOrganizationName(u"Osservatorio Astrofisico di Catania"_s);
    QApplication::setOrganizationDomain(u"it.inaf.oact"_s);

    // No need to set it on macOS
#ifndef Q_OS_APPLE
    QApplication::setWindowIcon(QIcon(u":/icons/VisIVO_512.png"_s));
#endif

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
    QDir logDir = QDir::home().absoluteFilePath(qApp->applicationName());
    const QString timestamp = QDateTime::currentDateTime().toString(u"yyyy-MM-dd_hh.mm.ss"_s);
    if (logDir.mkpath(timestamp) && logDir.cd(timestamp)) {
        app.setProperty("sessionFolder", logDir.absolutePath());
        logFile = logDir.absoluteFilePath(u"visivo.log"_s).toStdString();
        originalHandler = qInstallMessageHandler(logToFile);
    }

    // Print VTK logs via QDebug
    vtkLogger::AddCallback("vtk", vtkLogCallback, nullptr, vtkLogger::VERBOSITY_INFO);
    vtkLogger::SetStderrVerbosity(vtkLogger::VERBOSITY_OFF);

    MainWindow w;
    w.show();

    return QApplication::exec();
}
