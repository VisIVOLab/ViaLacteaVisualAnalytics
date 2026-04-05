#include "Catalogue3DTableModel.h"

Catalogue3DTableModel::Catalogue3DTableModel(QObject *parent) : QAbstractTableModel(parent) {}

void Catalogue3DTableModel::setCatalogue(const std::vector<Catalogue3DEntry> *newEntries,
                                         const QStringList *newHeaders)
{
    beginResetModel();
    this->entries = newEntries;
    this->headers = newHeaders;
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
    return FixedColumnCount + (this->headers ? this->headers->size() : 0);
}

QVariant Catalogue3DTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !this->entries || index.row() < 0
        || index.row() >= static_cast<int>(this->entries->size())) {
        return {};
    }

    const Catalogue3DEntry &entry = this->entries->at(static_cast<std::size_t>(index.row()));
    if (role == Qt::TextAlignmentRole) {
        return static_cast<int>((index.column() < 2 || index.column() >= FixedColumnCount)
                                        ? (Qt::AlignLeft | Qt::AlignVCenter)
                                                    : (Qt::AlignRight | Qt::AlignVCenter));
    }
    if (role != Qt::DisplayRole) {
        return {};
    }

    switch (index.column()) {
    case NameColumn:
        return entry.name;
    case TypeColumn:
        return entry.morphology;
    case RaColumn:
        return QString::number(entry.raDeg, 'f', 4);
    case DecColumn:
        return QString::number(entry.decDeg, 'f', 4);
    case DistanceColumn:
        return QString::number(entry.distanceMpc, 'f', 1);
    case XColumn:
        return QString::number(entry.sceneX, 'f', 3);
    case YColumn:
        return QString::number(entry.sceneY, 'f', 3);
    case ZColumn:
        return QString::number(entry.sceneZ, 'f', 3);
    default:
        break;
    }

    const int rawColumn = index.column() - FixedColumnCount;
    if (!this->headers || rawColumn < 0 || rawColumn >= this->headers->size()) {
        return {};
    }
    if (rawColumn >= 0 && rawColumn < entry.rawFieldValues.size()) {
        return entry.rawFieldValues.at(rawColumn);
    }
    return {};
}

QVariant Catalogue3DTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    switch (section) {
    case NameColumn:
        return QStringLiteral("Name");
    case TypeColumn:
        return QStringLiteral("Type");
    case RaColumn:
        return QStringLiteral("RA");
    case DecColumn:
        return QStringLiteral("Dec");
    case DistanceColumn:
        return QStringLiteral("Distance");
    case XColumn:
        return QStringLiteral("X");
    case YColumn:
        return QStringLiteral("Y");
    case ZColumn:
        return QStringLiteral("Z");
    default:
        break;
    }

    const int rawColumn = section - FixedColumnCount;
    if (this->headers && rawColumn >= 0 && rawColumn < this->headers->size()) {
        return this->headers->at(rawColumn);
    }
    return {};
}
