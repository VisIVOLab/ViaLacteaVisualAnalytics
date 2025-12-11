#include "rpc_server.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTextStream>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("VisIVO_remote_server");
    QCoreApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("VisIVO Remote Rendering Server (skeleton)");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption portOpt(QStringList() << "p" << "port",
                               "WebSocket port (default: 8090)", "port", "8090");
    parser.addOption(portOpt);
    parser.process(app);

    const quint16 port = parser.value(portOpt).toUShort();

    RpcServer server(port);
    QObject::connect(&server, &RpcServer::fatalError, [&](const QString &msg) {
        QTextStream(stderr) << msg << Qt::endl;
        QCoreApplication::exit(1);
    });

    if (!server.start()) {
        return 1;
    }

    QTextStream(stdout) << "VisIVO remote server listening on port " << port << Qt::endl;
    return app.exec();
}
