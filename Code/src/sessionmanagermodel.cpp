#include "sessionmanagermodel.h"

SessionManagerModel::SessionManagerModel(QObject *parent)
    : QStandardItemModel(parent)
{
    setHorizontalHeaderLabels(QStringList() << "Name" << "Type");
    rootItem = new QStandardItem("Open Windows");
    invisibleRootItem()->appendRow(rootItem);
    appendRow(rootItem);
}

void SessionManagerModel::addSessionItem(const QString &name, QWidget *window, QStandardItem *parent)
{
    QStandardItem *item = new QStandardItem(name);
    QStandardItem *typeItem = new QStandardItem(window->metaObject()->className());
    
    QList<QStandardItem *> row;
    row << item << typeItem;
    
    if (parent == nullptr) {
        parent = invisibleRootItem()->child(0);  // The "Open Windows" item
    }
    
    parent->appendRow(row);
    windowItems.insert(window, item);
    
    // Connect the window's close event to update the session manager
    connect(window, &QWidget::destroyed, this, [this, window]() {
        removeSessionItem(window);
    });
}

void SessionManagerModel::addSessionItem(const QString &name, QWidget *window, const QString &parentName)
{
    QStandardItem *parentItem = findItemByName(parentName);
    if (parentItem == nullptr) {
        parentItem = invisibleRootItem()->child(0);  // Default to root if not found
    }
    addSessionItem(name, window, parentItem);
}

QStandardItem* SessionManagerModel::findItemByName(const QString &name)
{
    QList<QStandardItem *> items = findItems(name, Qt::MatchRecursive);
    if (!items.isEmpty()) {
        return items.first();
    }
    return nullptr;
}

void SessionManagerModel::removeSessionItem(QWidget *window)
{
    if (windowItems.contains(window)) {
        QStandardItem *item = windowItems.value(window);
        int row = item->index().row();
        item->parent()->removeRow(row);
        windowItems.remove(window);
    }
}
