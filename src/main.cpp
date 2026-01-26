#include "MainWindow.h"
#include "Version.h"

#include <QApplication>
#include <QLocale>

#include <clocale>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set locale to C.UTF-8
    std::setlocale(LC_NUMERIC, "C.UTF-8");
    QLocale::setDefault(QLocale::c());

    // Set application info
    QApplication::setApplicationName(u"VisIVO Visual Analytics"_s);
    QApplication::setApplicationVersion(QStringLiteral(VISIVO_VERSION_STR));
    QApplication::setWindowIcon(QIcon(":/icons/VisIVO_512.png"));
    QApplication::setOrganizationName(u"Osservatorio Astrofisico di Catania"_s);
    QApplication::setOrganizationDomain(u"it.inaf.oact"_s);

    // Customize QDebug message format
    qSetMessagePattern("%{time} "
                       "%{if-debug}D %{endif}"
                       "%{if-info}I %{endif}"
                       "%{if-warning}W %{endif}"
                       "%{if-critical}C %{endif}"
                       "%{if-fatal}F %{endif}"
#ifndef NDEBUG
                       "%{file} %{function}:%{line} "
#endif
                       "%{if-category}%{category} %{endif}%{message}");

    MainWindow window;
    window.show();

    return QApplication::exec();
}
