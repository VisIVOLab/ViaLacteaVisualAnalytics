#include "recentfilesmanager.h"

RecentFilesManager::RecentFilesManager(QObject *parent)
    : QAbstractListModel(parent), maxRecentFiles(10), settings("MyCompany", "MyApp", this)
{
    this->history = this->settings.value("recentFiles").toStringList();
}

RecentFilesManager::~RecentFilesManager()
{
    this->settings.setValue("recentFiles", this->history);
    this->settings.sync();
}

int RecentFilesManager::rowCount(const QModelIndex &parent) const
{
    return this->history.count();
}

QVariant RecentFilesManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return this->history.at(index.row());
    }

    return QVariant();
}

void RecentFilesManager::addRecentFile(const QString &filename)
{
    int idx = this->history.indexOf(filename);
    if (idx > 0) {
        // Item exists and is not the most recent one -> move to top
        this->history.removeAt(idx);
        this->history.prepend(filename);
        emit this->dataChanged(this->index(0), this->index(idx));
        return;
    }

    while (this->history.count() >= this->maxRecentFiles) {
        // Drop least recent one
        this->history.removeLast();
    }
    this->history.prepend(filename);
    emit this->dataChanged(this->index(0), this->index(this->history.count() - 1));
}

QStringList RecentFilesManager::recentFiles() const
{
    return this->history;
}

void RecentFilesManager::clearHistory()
{
    this->history.clear();
    emit this->dataChanged(this->index(0), this->index(0));
}
