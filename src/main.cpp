#include "Version.h"
#include "vtkWindowCube.h"
#include "vtkWindowImage.h"

#include <QVTKOpenGLNativeWidget.h>

#include <QApplication>
#include <QIcon>
#include <QLocale>
#include <QSurfaceFormat>

#include <clocale>
#include <vtkWindow.h>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    // Enable OpenGL shared contexts
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    // Set OpenGL profile needed for VTK
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

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

    auto win1 = new vtkWindowImage("/Users/giuseppe/Misc/images/image1.fits");
    win1->show();

    auto win2 = new vtkWindowCube("/Users/giuseppe/Misc/cubes/TAN_C14.fits");
    win2->show();

    return QApplication::exec();
}
