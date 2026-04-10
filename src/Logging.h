#ifndef Logging_h
#define Logging_h

#include <QLoggingCategory>

static const QLoggingCategory &logApp()
{
    static const QLoggingCategory category("main");
    return category;
}

static const QLoggingCategory &logVtk()
{
    static const QLoggingCategory category("vtk");
    return category;
}

static const QLoggingCategory &logSettings()
{
    static const QLoggingCategory category("settings");
    return category;
}

#endif
