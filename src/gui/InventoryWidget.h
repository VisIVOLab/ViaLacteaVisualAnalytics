#ifndef InventoryWidget_h
#define InventoryWidget_h

#include <QWidget>

#include "VOTable.h"

struct WavelengthGroup
{
    QString name;
    double lambdaMin;
    double lambdaMax;
};

struct Cutout
{
    QString id;
    QString type;
    QString survey;
    QString species;
    QString transition;
    QString pubDID;
    QString skyRegion;
    int overlap;
    double lambdaMin;
    double lambdaMax;
};

QT_BEGIN_NAMESPACE
namespace Ui {
class InventoryWidget;
}
QT_END_NAMESPACE

class InventoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InventoryWidget(const VOTable &votable, QWidget *parent = nullptr);
    ~InventoryWidget() override;

private slots:
    void itemClicked(const QModelIndex &index);
    void itemDoubleClicked(const QModelIndex &index);

private:
    Ui::InventoryWidget *ui;
    VOTable votable;
    QHash<QUuid, Cutout> cutouts;
    QVector<WavelengthGroup> wavelengthGroups;

    void setupWavelengthGroups();
    void setupTopLevels();
    void buildTree();
    void applyStylePreferences(const QModelIndex &parent);

    int findWavelengthGroupIndex(double lambdaMin, double lambdaMax) const;
};

#endif
