#ifndef VLKBINVENTORYTREE_H
#define VLKBINVENTORYTREE_H

#include <QWidget>

#include "vtkwindow_new.h"

namespace Ui {
class VLKBInventoryTree;
}

class vtkwindow_new;

struct WavelengthGroup
{
    QString name;
    double lambdaMin;
    double lambdaMax;
};

struct Cutout
{
    QString type; // image | cube
    QString survey;
    QString species;
    QString transition;
    QString pubDID;
    QString ivoID;
    double spectrum_min;
    double spectrum_max;
    int overlap;
    QString region_gal;
};

class VLKBInventoryTree : public QWidget
{
    Q_OBJECT

public:
    explicit VLKBInventoryTree(const QByteArray &votable, QWidget *parent = nullptr);
    ~VLKBInventoryTree();

    QPointer<vtkwindow_new> getLinkedWindow() const;
    void setLinkedWindow(vtkwindow_new *win);

    QList<Cutout> getList() const;

private slots:
    void click(const QModelIndex &idx);
    void doubleClick(const QModelIndex &idx);

private:
    Ui::VLKBInventoryTree *ui;
    QList<WavelengthGroup> groups;
    QHash<QUuid, Cutout> cutouts;
    QPointer<vtkwindow_new> imgWindow;

    void setupWavelengthGroups();
    void setupTopLevels();
    void parseVOTable(const QByteArray &votable);

    int groupIndexOf(double lambda) const;
    void setTreeStylePreferences(const QModelIndex &parent);
};

#endif // VLKBINVENTORYTREE_H
