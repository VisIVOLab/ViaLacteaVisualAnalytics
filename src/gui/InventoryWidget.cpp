#include "InventoryWidget.h"
#include "ui_InventoryWidget.h"

#include "Logging.h"

#include <QStandardItem>
#include <QStandardItemModel>
#include <QUuid>

using namespace Qt::StringLiterals;

InventoryWidget::InventoryWidget(const VOTable &votable, QWidget *parent)
    : QWidget(parent), ui(new Ui::InventoryWidget), votable(votable)
{
    ui->setupUi(this);
    QObject::connect(ui->treeInventory, &QAbstractItemView::clicked, this,
                     &InventoryWidget::itemClicked);
    QObject::connect(ui->treeInventory, &QAbstractItemView::doubleClicked, this,
                     &InventoryWidget::itemDoubleClicked);

    this->setupWavelengthGroups();
    this->setupTopLevels();
    this->buildTree();
    this->applyStylePreferences(ui->treeInventory->model()->index(0, 0));
    this->applyStylePreferences(ui->treeInventory->model()->index(1, 0));
}

InventoryWidget::~InventoryWidget()
{
    delete ui;
}

void InventoryWidget::itemClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    auto *model = qobject_cast<QStandardItemModel *>(ui->treeInventory->model());
    const auto uuid = model->itemFromIndex(index)->data(Qt::UserRole).toUuid();
    if (this->cutouts.contains(uuid)) {
        const auto cutout = this->cutouts.value(uuid);
        qCInfo(logApp) << cutout.type << cutout.skyRegion;
    }
}

void InventoryWidget::itemDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    auto *model = qobject_cast<QStandardItemModel *>(ui->treeInventory->model());
    const auto uuid = model->itemFromIndex(index)->data(Qt::UserRole).toUuid();
    if (this->cutouts.contains(uuid)) {
        const auto cutout = this->cutouts.value(uuid);
        qCInfo(logApp) << cutout.type << cutout.id;
    }
}

void InventoryWidget::setupWavelengthGroups()
{
    /*
     * Visible (l < 1 um)
     * Near-IR / Mid-IR (1 um <= l <= 25 um)
     * Far-IR (25 um < l <= 500 um)
     * Submm (500 um < l <= 3 mm)
     * Radio (l > 3 mm)
     */

    this->wavelengthGroups.append(WavelengthGroup{ u"Visible"_s, 0., 1.e-6 });
    this->wavelengthGroups.append(WavelengthGroup{ u"Near-IR / Mid-IR"_s, 1.e-6, 25.e-6 });
    this->wavelengthGroups.append(WavelengthGroup{ u"Far-IR"_s, 25.e-6, 500.e-6 });
    this->wavelengthGroups.append(WavelengthGroup{ u"Submm"_s, 500.e-6, 3.e-3 });
    this->wavelengthGroups.append(
            WavelengthGroup{ u"Radio"_s, 3.e-3, std::numeric_limits<double>::max() });
}

void InventoryWidget::setupTopLevels()
{
    /* Continuum
     *  |-Visible
     *    |-Cutouts
     *  |-Near-IR / Mid-IR
     *  |-Far-IR
     *  |-Submm
     *  |-Radio
     * Spectroscopy
     *  |-Visible
     *    |-Species
     *      |-Cutouts
     *  |-Near-IR / Mid-IR
     *  |-Far-IR
     *  |-Submm
     *  |-Radio
     */

    auto *model = new QStandardItemModel(ui->treeInventory);
    auto *root = model->invisibleRootItem();

    auto *images = new QStandardItem(u"Continuum"_s);
    auto *cubes = new QStandardItem(u"Spectroscopy"_s);

    std::for_each(this->wavelengthGroups.cbegin(), this->wavelengthGroups.cend(),
                  [images, cubes](const WavelengthGroup &wl) {
                      images->appendRow(new QStandardItem(wl.name));
                      cubes->appendRow(new QStandardItem(wl.name));
                  });

    root->appendRow(images);
    root->appendRow(cubes);
    ui->treeInventory->setModel(model);
}

void InventoryWidget::buildTree()
{
    int idxIvoId = -1;
    int idxType = -1;
    int idxSurvey = -1;
    int idxSubSurvey = -1;
    int idxPubDID = -1;
    int idxSkyRegion = -1;
    int idxOverlap = -1;
    int idxLambdaMin = -1;
    int idxLambdaMax = -1;

    for (int idx = 0; idx < this->votable.fields.size(); ++idx) {
        if (this->votable.fields.at(idx).name == u"obs_publisher_did"_s) {
            idxIvoId = idx;
        }

        if (this->votable.fields.at(idx).name == u"dataproduct_type"_s) {
            idxType = idx;
        }

        if (this->votable.fields.at(idx).name == u"obs_collection"_s) {
            idxSurvey = idx;
        }

        if (this->votable.fields.at(idx).name == u"obs_title"_s) {
            idxSubSurvey = idx;
        }

        if (this->votable.fields.at(idx).name == u"bib_reference"_s) {
            idxPubDID = idx;
        }

        if (this->votable.fields.at(idx).name == u"s_region_galactic"_s) {
            idxSkyRegion = idx;
        }

        if (this->votable.fields.at(idx).name == u"overlapSky"_s) {
            idxOverlap = idx;
        }

        if (this->votable.fields.at(idx).name == u"em_min"_s) {
            idxLambdaMin = idx;
        }

        if (this->votable.fields.at(idx).name == u"em_max"_s) {
            idxLambdaMax = idx;
        }
    }

    for (const auto &row : this->votable.rows) {
        const auto subSurvey = row.at(idxSubSurvey).toString().split('|');

        Cutout c;
        c.id = row.at(idxIvoId).toString();
        c.type = row.at(idxType).toString();
        c.survey = row.at(idxSurvey).toString();
        c.species = subSurvey.first().simplified();
        c.transition = subSurvey.last().simplified();
        c.pubDID = row.at(idxPubDID).toString();
        c.skyRegion = row.at(idxSkyRegion).toString();
        c.overlap = row.at(idxOverlap).toInt();
        c.lambdaMin = row.at(idxLambdaMin).toDouble();
        c.lambdaMax = row.at(idxLambdaMax).toDouble();

        const auto uuid = QUuid::createUuid();
        this->cutouts.insert(uuid, c);

        const int idx = findWavelengthGroupIndex(c.lambdaMin, c.lambdaMax);

        const auto label = u"%1 %2"_s.arg(c.survey, c.transition);
        auto *w = new QStandardItem(label);
        w->setToolTip(c.pubDID);
        w->setData(uuid, Qt::UserRole);
        if (c.overlap == 3) {
            w->setBackground(Qt::green);
        }

        auto *model = qobject_cast<QStandardItemModel *>(ui->treeInventory->model());
        if (c.type.compare("image"_L1) == 0) {
            model->item(0)->child(idx)->appendRow(w);
        } else {
            // c.type == "cube"
            auto *item = model->item(1)->child(idx);
            QStandardItem *subItem = nullptr;
            for (int r = 0; r < item->rowCount() && !subItem; ++r) {
                if (item->child(r)->text().compare(c.species) == 0) {
                    subItem = item->child(r);
                }
            }

            if (!subItem) {
                subItem = new QStandardItem(c.species);
                item->appendRow(subItem);
            }

            subItem->appendRow(w);
        }
    }
}

void InventoryWidget::applyStylePreferences(const QModelIndex &parent)
{
    // Expand 1st level
    ui->treeInventory->expand(parent);

    // If element has children -> apply bold font style
    // otherwise, apply gray color
    auto *model = qobject_cast<QStandardItemModel *>(ui->treeInventory->model());
    for (int r = 0; r < this->wavelengthGroups.size(); ++r) {
        auto index = model->index(r, 0, parent);
        if (model->hasChildren(index)) {
            auto f = model->itemFromIndex(index)->font();
            f.setBold(true);
            model->itemFromIndex(index)->setFont(f);
        } else {
            model->itemFromIndex(index)->setForeground(Qt::gray);
        }
    }
}

int InventoryWidget::findWavelengthGroupIndex(double lambdaMin, double lambdaMax) const
{
    for (int i = 0; i < this->wavelengthGroups.size(); ++i) {
        const auto &group = this->wavelengthGroups.at(i);

        if (lambdaMin >= group.lambdaMin && lambdaMax < group.lambdaMax) {
            return i;
        }
    }

    return -1;
}
