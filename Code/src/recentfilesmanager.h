#ifndef RECENTFILESMANAGER_H
#define RECENTFILESMANAGER_H

#include <QAbstractListModel>

#include <QSettings>

class RecentFilesManager : public QAbstractListModel
{
    Q_OBJECT

public:
    RecentFilesManager(QObject *parent = nullptr);
    ~RecentFilesManager() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void addRecentFile(const QString &filename);
    QStringList recentFiles() const;
    void clearHistory();

private:
    QStringList history;
    const int maxRecentFiles;
    QSettings settings;
};

#endif // RECENTFILESMANAGER_H
