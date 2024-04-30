#ifndef RECENTFILESMANAGER_H
#define RECENTFILESMANAGER_H

#include <QObject>
#include <QSettings>

class RecentFilesManager : public QObject {
    Q_OBJECT

public:
    RecentFilesManager(QObject *parent = nullptr);

    void addRecentFile(const QString& filename);
    QStringList recentFiles() const;

private:
    QSettings *settings;
    const int maxRecentFiles = 10;
};

#endif // RECENTFILESMANAGER_H
