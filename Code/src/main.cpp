#include "mainwindow.h"
#include "singleton.h"
#include "version.h"

#include <QApplication>
#include <QLocale>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>
#include <QWebEngineUrlScheme>
#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>

#include <clocale>
#include "remote2dwindow.h"
#include <QJsonArray>
#include <QJsonObject>


int main(int argc, char *argv[])
{
    QWebEngineUrlScheme vlvaUrlScheme("vlva");
    vlvaUrlScheme.setSyntax(QWebEngineUrlScheme::Syntax::Path);
    vlvaUrlScheme.setDefaultPort(QWebEngineUrlScheme::PortUnspecified);
    vlvaUrlScheme.setFlags(QWebEngineUrlScheme::SecureScheme);
    QWebEngineUrlScheme::registerScheme(vlvaUrlScheme);

    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    // Condividi i contesti GL tra VTK e WebEngine per evitare crash del render process.
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QApplication a(argc, argv);
    a.setApplicationName("Vialactea - Visual Analytics client");
    a.setApplicationVersion(VLVA_VERSION_STR);
    a.setWindowIcon(QIcon(":/icons/visivo512.png"));

    std::setlocale(LC_ALL, "C");
    QLocale::setDefault(QLocale::c());

    Singleton<MainWindow>::Instance();

    // Finestra di test: interazione con isocontour sul datacube fornax via backend remoto
    const QString fitsPath = QStringLiteral("/Users/fvitello/Desktop/tmp_download/fornax.fits");
    qInfo() << "[Main] Launching Remote2DWindow for" << fitsPath;
    auto w = new Remote2DWindow(QUrl(QStringLiteral("ws://localhost:8090")));
    QObject::connect(w, &Remote2DWindow::error, w, [](const QString &msg) {
        qWarning() << "[Remote2DWindow][client]" << msg;
    });
    w->setWindowTitle(QStringLiteral("Remote IsoSurface test - fornax"));
    w->resize(960, 720);
    w->show();
    if (!w->connectServer()) {
        qCritical() << "[Main] Unable to connect to remote renderer";
    } else {
        qInfo() << "[Main] Connected. Loading dataset...";
        if (!w->loadFits(fitsPath)) {
            qCritical() << "[Main] loadFits failed for" << fitsPath;
        } else {
            // Iso-surface 3D (marching cubes) con colore e opacità
            QJsonObject volParams{
                {QStringLiteral("iso"), 0.02},
                {QStringLiteral("color"), QJsonArray{0.8, 0.8, 1.0}},
                {QStringLiteral("opacity"), 1.0}
            };
            qInfo() << "[Main] Requesting remote volume render with params" << volParams;
            if (!w->renderVolume(volParams, 0, 0, QStringLiteral("interactive"))) {
                qWarning() << "[Main] renderVolume request failed";
            }
        }
    }

    return a.exec();
}
