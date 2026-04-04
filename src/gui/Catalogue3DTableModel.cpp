#include "Catalogue3DTableModel.h"

Catalogue3DTableModel::Catalogue3DTableModel(QObject *parent) : QAbstractTableModel(parent) {}

void Catalogue3DTableModel::setEntries(const std::vector<Catalogue3DEntry> *newEntries)
{
    beginResetModel();
    this->entries = newEntries;
    endResetModel();
}

int Catalogue3DTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !this->entries) {
        return 0;
    }
    return static_cast<int>(this->entries->size());
}

int Catalogue3DTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return 6;
}

QVariant Catalogue3DTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !this->entries || index.row() < 0
        || index.row() >= static_cast<int>(this->entries->size())) {
        return {};
    }

    const Catalogue3DEntry &entry = this->entries->at(static_cast<std::size_t>(index.row()));
    if (role == Qt::TextAlignmentRole) {
        return static_cast<int>((index.column() < 2) ? (Qt::AlignLeft | Qt::AlignVCenter)
                                                    : (Qt::AlignRight | Qt::AlignVCenter));
    }
    if (role != Qt::DisplayRole) {
        return {};
    }

    switch (index.column()) {
    case 0:
        return entry.name;
    case 1:
        return entry.morphology;
    case 2:
        return QString::number(entry.raDeg, 'f', 4);
    case 3:
        return QString::number(entry.decDeg, 'f', 4);
    case 4:
        if (entry.llsMajorKpc > 0.0) {
            return entry.llsMinorKpc > 0.0
                    ? QStringLiteral("%1 x %2")
                              .arg(QString::number(entry.llsMinorKpc, 'f', 1),
                                   QString::number(entry.llsMajorKpc, 'f', 1))
                    : QString::number(entry.llsMajorKpc, 'f', 1);
        }
        if (entry.majorAxisArcmin > 0.0) {
            return entry.minorAxisArcmin > 0.0
                    ? QStringLiteral("%1 x %2")
                              .arg(QString::number(entry.minorAxisArcmin, 'f', 2),
                                   QString::number(entry.majorAxisArcmin, 'f', 2))
                    : QString::number(entry.majorAxisArcmin, 'f', 2);
        }
        return {};
    case 5:
        if (entry.redshift > 0.0) {
            return QString::number(entry.redshift, 'f', 4);
        }
        return QString::number(entry.distanceMpc, 'f', 1);
    default:
        return {};
    }
}

QVariant Catalogue3DTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    switch (section) {
    case 0:
        return QStringLiteral("Name");
    case 1:
        return QStringLiteral("Type");
    case 2:
        return QStringLiteral("RA");
    case 3:
        return QStringLiteral("Dec");
    case 4:
        return QStringLiteral("Size");
    case 5:
        return QStringLiteral("z / d");
    default:
        return {};
    }
}
