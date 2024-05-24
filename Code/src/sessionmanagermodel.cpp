#include "sessionmanagermodel.h"

SessionManagerModel::SessionManagerModel(QObject *parent)
    : QStandardItemModel(parent)
{
    setHorizontalHeaderLabels(QStringList() << "Name" << "Type");
    rootItem = new QStandardItem("Open Windows");
    invisibleRootItem()->appendRow(rootItem);
    appendRow(rootItem);
}

void SessionManagerModel::addSessionItem(const QString &name, QWidget *window)
{
    QStandardItem *item = new QStandardItem(name);
    QStandardItem *typeItem = new QStandardItem(window->metaObject()->className());
    
    QList<QStandardItem *> row;
    row << item << typeItem;
    
    rootItem->appendRow(row);
    windowItems.insert(window, item);
    
    // Connect the window's close event to update the session manager
    connect(window, &QWidget::destroyed, this, [this, window]() {
        removeSessionItem(window);
    });
}

void SessionManagerModel::removeSessionItem(QWidget *window)
{
    if (windowItems.contains(window)) {
        QStandardItem *item = windowItems.value(window);
        int row = item->index().row();  // Ottenere l'indice e poi la riga
        rootItem->removeRow(row);
        windowItems.remove(window);
    }
}
