#include "recentfilesmanager.h"

RecentFilesManager::RecentFilesManager(QObject *parent) : QObject(parent) {
    settings = new QSettings("MyCompany", "MyApp", this);
}

void RecentFilesManager::addRecentFile(const QString& filename) {
    QStringList recentFiles = settings->value("recentFiles").toStringList();
    recentFiles.removeAll(filename);
    recentFiles.prepend(filename);
    while (recentFiles.size() > maxRecentFiles) {
        recentFiles.removeLast();
    }
    settings->setValue("recentFiles", recentFiles);
}

QStringList RecentFilesManager::recentFiles() const {
    return settings->value("recentFiles").toStringList();
}
