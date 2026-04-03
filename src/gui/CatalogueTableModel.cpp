#include "CatalogueTableModel.h"

#include <QString>

namespace {
QString catalogueShapeName(CatalogueOverlayEntry::Shape shape)
{
    return shape == CatalogueOverlayEntry::Shape::Ellipse ? QStringLiteral("Ellipse")
                                                          : QStringLiteral("Point");
}
}

CatalogueTableModel::CatalogueTableModel(QObject *parent) : QAbstractTableModel(parent) {}

void CatalogueTableModel::setEntries(const std::vector<CatalogueOverlayEntry> *newEntries)
{
    beginResetModel();
    this->entries = newEntries;
    endResetModel();
}

int CatalogueTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !this->entries) {
        return 0;
    }
    return static_cast<int>(this->entries->size());
}

int CatalogueTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return 6;
}

QVariant CatalogueTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !this->entries || index.row() < 0
        || index.row() >= static_cast<int>(this->entries->size())) {
        return {};
    }

    const CatalogueOverlayEntry &entry = this->entries->at(static_cast<std::size_t>(index.row()));
    const auto column = static_cast<Column>(index.column());

    if (role == Qt::TextAlignmentRole) {
        switch (column) {
        case Column::Name:
        case Column::Type:
            return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
        default:
            return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        }
    }

    if (role != Qt::DisplayRole) {
        return {};
    }

    switch (column) {
    case Column::Name:
        return entry.label.trimmed().isEmpty() ? QStringLiteral("Source %1").arg(index.row() + 1)
                                               : entry.label.trimmed();
    case Column::Type:
        return catalogueShapeName(entry.shape);
    case Column::X:
        return QString::number(entry.pixelX, 'f', 2);
    case Column::Y:
        return QString::number(entry.pixelY, 'f', 2);
    case Column::Size:
        if (entry.shape == CatalogueOverlayEntry::Shape::Ellipse) {
            return QStringLiteral("%1, %2")
                    .arg(QString::number(entry.radiusX, 'f', 2),
                         QString::number(entry.radiusY, 'f', 2));
        }
        return {};
    case Column::Angle:
        if (entry.shape == CatalogueOverlayEntry::Shape::Ellipse) {
            return QString::number(entry.angleDeg, 'f', 1);
        }
        return {};
    }

    return {};
}

QVariant CatalogueTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    switch (static_cast<Column>(section)) {
    case Column::Name:
        return QStringLiteral("Name");
    case Column::Type:
        return QStringLiteral("Type");
    case Column::X:
        return QStringLiteral("X");
    case Column::Y:
        return QStringLiteral("Y");
    case Column::Size:
        return QStringLiteral("Size");
    case Column::Angle:
        return QStringLiteral("Angle");
    }

    return {};
}
