#ifndef SESSIONMANAGERMODEL_H
#define SESSIONMANAGERMODEL_H

#include <QStandardItemModel>
#include <QWidget>

class SessionManagerModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit SessionManagerModel(QObject *parent = nullptr);
    void addSessionItem(const QString &name, QWidget *window);
    void removeSessionItem(QWidget *window);

private:
    QStandardItem *rootItem;
    QHash<QWidget *, QStandardItem *> windowItems;
};

#endif // SESSIONMANAGERMODEL_H
