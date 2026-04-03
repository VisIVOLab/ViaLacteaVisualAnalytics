#ifndef CatalogueTableModel_h
#define CatalogueTableModel_h

#include "CatalogueOverlayUtils.h"

#include <QAbstractTableModel>

#include <vector>

class CatalogueTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit CatalogueTableModel(QObject *parent = nullptr);

    void setEntries(const std::vector<CatalogueOverlayEntry> *entries);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    enum class Column
    {
        Name = 0,
        Type,
        X,
        Y,
        Size,
        Angle,
    };

    const std::vector<CatalogueOverlayEntry> *entries{ nullptr };
};

#endif
