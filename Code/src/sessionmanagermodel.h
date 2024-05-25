#ifndef SESSIONMANAGERMODEL_H
#define SESSIONMANAGERMODEL_H

#include <QStandardItemModel>
#include <QWidget>

class SessionManagerModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit SessionManagerModel(QObject *parent = nullptr);
    void addSessionItem(const QString &name, QWidget *window, QStandardItem *parent = nullptr);
    void addSessionItem(const QString &name, QWidget *window, const QString &parentName);
    void removeSessionItem(QWidget *window);
    QStandardItem* findItemByName(const QString &name);

    QStandardItem *rootItem;

private:
    QHash<QWidget *, QStandardItem *> windowItems;
};

#endif // SESSIONMANAGERMODEL_H
