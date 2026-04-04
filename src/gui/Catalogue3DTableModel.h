#ifndef Catalogue3DTableModel_h
#define Catalogue3DTableModel_h

#include "Catalogue3DParser.h"

#include <QAbstractTableModel>

#include <vector>

class Catalogue3DTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit Catalogue3DTableModel(QObject *parent = nullptr);

    void setEntries(const std::vector<Catalogue3DEntry> *entries);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    const std::vector<Catalogue3DEntry> *entries{ nullptr };
};

#endif
