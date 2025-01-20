#include "VLKBInventoryTree.h"
#include "ui_VLKBInventoryTree.h"

#include "vialacteainitialquery.h"
#include "vtkwindowcube.h"

#include <QDebug>
#include <QModelIndex>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QUuid>
#include <QXmlStreamReader>

VLKBInventoryTree::VLKBInventoryTree(const QByteArray &votable, QWidget *parent)
    : QWidget(parent), ui(new Ui::VLKBInventoryTree)
{
    ui->setupUi(this);
    QObject::connect(ui->treeView, &QAbstractItemView::clicked, this, &VLKBInventoryTree::click);
    QObject::connect(ui->treeView, &QAbstractItemView::doubleClicked, this,
                     &VLKBInventoryTree::doubleClick);

    this->setupWavelengthGroups();
    this->setupTopLevels();
    this->parseVOTable(votable);
    this->setTreeStylePreferences(ui->treeView->model()->index(0, 0));
    this->setTreeStylePreferences(ui->treeView->model()->index(1, 0));
}

VLKBInventoryTree::~VLKBInventoryTree()
{
    delete ui;
}

QPointer<vtkwindow_new> VLKBInventoryTree::getLinkedWindow() const
{
    return this->imgWindow;
}

void VLKBInventoryTree::setLinkedWindow(vtkwindow_new *win)
{
    this->imgWindow = win;
}

QList<Cutout> VLKBInventoryTree::getList() const
{
    return this->cutouts.values();
}

void VLKBInventoryTree::click(const QModelIndex &idx)
{
    if (!idx.isValid() || !this->imgWindow) {
        return;
    }

    auto model = qobject_cast<QStandardItemModel *>(ui->treeView->model());
    QUuid id = model->itemFromIndex(idx)->data(Qt::UserRole).toUuid();

    if (this->cutouts.contains(id)) {
        const Cutout c = this->cutouts.value(id);
        auto points = c.region_gal.split(' ');
        double skyPoints[8];
        for (int i = 2; i < 10; ++i) {
            skyPoints[i - 2] = points.at(i).toDouble();
        }

        this->imgWindow->drawRectangleFootprint(skyPoints);
    }
}

void VLKBInventoryTree::doubleClick(const QModelIndex &idx)
{
    if (!idx.isValid() || !this->imgWindow) {
        return;
    }

    auto model = qobject_cast<QStandardItemModel *>(ui->treeView->model());
    QUuid id = model->itemFromIndex(idx)->data(Qt::UserRole).toUuid();

    if (this->cutouts.contains(id)) {
        const Cutout c = this->cutouts.value(id);
        qDebug() << "Double clicked" << c.ivoID << c.region_gal;
        auto vq = new VialacteaInitialQuery;
        vq->cutoutRequest(c.ivoID, QDir::home().absoluteFilePath("VisIVODesktopTemp/tmp_download"),
                          c.region_gal);
        if (c.type == "image") {
            connect(vq, &VialacteaInitialQuery::cutoutDone, this,
                    [this, vq](const QString &path, const Cutout &src) {
                        auto fits = vtkSmartPointer<vtkFitsReader>::New();
                        fits->SetFileName(path.toStdString());
                        fits->setSurvey(src.survey);
                        fits->setSpecies(src.species);
                        fits->setTransition(src.transition);
                        this->imgWindow->addLayerImage(fits, src.survey, src.species,
                                                       src.transition);
                        vq->deleteLater();
                    });
        } else {
            connect(vq, &VialacteaInitialQuery::cutoutDone, this, [this, vq](const QString &path) {
                auto win = new vtkWindowCube(this->imgWindow, path);
                win->show();
                vq->deleteLater();
            });
        }
    }
}

void VLKBInventoryTree::setupWavelengthGroups()
{
    /*
     * Visible (l < 1 um)
     * Near-IR / Mid-IR (1 um <= l <= 25 um)
     * Far-IR (25 um < l <= 500 um)
     * Submm (500 um < l <= 3 mm)
     * Radio (l > 3 mm)
     */

    this->groups.clear();
    this->groups << WavelengthGroup { "Visible", 0., 1.e-6 };
    this->groups << WavelengthGroup { "Near-IR / Mid-IR", 1.e-6, 2.5e-5 };
    this->groups << WavelengthGroup { "Far-IR", 2.5e-5, 5.e-4 };
    this->groups << WavelengthGroup { "Submm", 5.e-4, 3.e-3 };
    this->groups << WavelengthGroup { "Radio", 3.e-3, 1.e6 };
}

void VLKBInventoryTree::setupTopLevels()
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

    auto images = new QStandardItem("Continuum");
    auto cubes = new QStandardItem("Spectroscopy");

    std::for_each(this->groups.cbegin(), this->groups.cend(),
                  [images, cubes](const WavelengthGroup &group) {
                      images->appendRow(new QStandardItem(group.name));
                      cubes->appendRow(new QStandardItem(group.name));
                  });

    auto model = new QStandardItemModel;
    model->appendRow(images);
    model->appendRow(cubes);
    ui->treeView->setModel(model);
}

void VLKBInventoryTree::parseVOTable(const QByteArray &votable)
{
    int idx = 0;
    int idxType = -1;
    int idxCollection = -1;
    int idxSurvey = -1;
    int idxSubSurvey = -1;
    int idxPubDID = -1;
    int idxIvoID = -1;
    int idxSpectrumMin = -1;
    int idxSpectrumMax = -1;
    int idxOverlap = -1;
    int idxRegion = -1;

    QXmlStreamReader xml(votable);
    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        // Get indexes
        if (xml.isStartElement() && xml.name() == "FIELD") {
            QString fieldName = xml.attributes().value("name").toString();

            if (fieldName == "dataproduct_type") {
                idxType = idx;
            }

            if (fieldName == "obs_collection") {
                idxSurvey = idx;
            }

            if (fieldName == "obs_title") {
                idxSubSurvey = idx;
            }

            if (fieldName == "bib_reference") {
                idxPubDID = idx;
            }

            if (fieldName == "obs_publisher_did") {
                idxIvoID = idx;
            }

            if (fieldName == "em_min") {
                idxSpectrumMin = idx;
            }

            if (fieldName == "em_max") {
                idxSpectrumMax = idx;
            }

            if (fieldName == "overlapSky") {
                idxOverlap = idx;
            }

            if (fieldName == "s_region_galactic") {
                idxRegion = idx;
            }

            ++idx;
        }

        if (xml.isStartElement() && xml.name() == "TR") {
            Cutout c;
            int idxField = 0;
            while (!(xml.isEndElement() && xml.name() == "TR")) {
                xml.readNext();
                if (xml.isStartElement() && xml.name() == "TD") {
                    QString fieldValue = xml.readElementText();

                    if (idxField == idxType) {
                        c.type = fieldValue;
                    }

                    if (idxField == idxSurvey) {
                        c.survey = fieldValue;
                    }

                    if (idxField == idxSubSurvey) {
                        auto tmp = fieldValue.split('|');
                        c.species = tmp.first().simplified();
                        c.transition = tmp.last().simplified();
                    }

                    if (idxField == idxPubDID) {
                        c.pubDID = fieldValue.simplified();
                    }

                    if (idxField == idxIvoID) {
                        c.ivoID = fieldValue.simplified();
                    }

                    if (idxField == idxSpectrumMin) {
                        c.spectrum_min = fieldValue.toDouble();
                    }

                    if (idxField == idxSpectrumMax) {
                        c.spectrum_max = fieldValue.toDouble();
                    }

                    if (idxField == idxOverlap) {
                        c.overlap = fieldValue.toInt();
                    }

                    if (idxField == idxRegion) {
                        c.region_gal = fieldValue;
                    }

                    ++idxField;
                }
            }

            QUuid id = QUuid::createUuid();
            this->cutouts.insert(id, c);

            // Put in tree
            QString label = c.survey + ' ' + c.transition;

            auto model = qobject_cast<QStandardItemModel *>(ui->treeView->model());
            if (c.type == "image") {
                int idx = this->groupIndexOf(c.spectrum_min);

                auto item = model->item(0)->child(idx);
                auto row = new QStandardItem(label);
                row->setToolTip(c.pubDID);
                row->setData(id, Qt::UserRole);
                if (c.overlap == 3) {
                    row->setBackground(Qt::green);
                }
                item->appendRow(row);
            }

            if (c.type == "cube") {
                int idx = this->groupIndexOf(c.spectrum_min);
                auto item = model->item(1)->child(idx);
                QStandardItem *subItem = nullptr;
                for (int r = 0; r < item->rowCount(); ++r) {
                    if (item->child(r)->text() == c.species) {
                        subItem = item->child(r);
                        break;
                    }
                }
                if (!subItem) {
                    subItem = new QStandardItem(c.species);
                    item->appendRow(subItem);
                }

                auto row = new QStandardItem(label);
                row->setToolTip(c.pubDID);
                row->setData(id, Qt::UserRole);
                if (c.overlap == 3) {
                    row->setBackground(Qt::green);
                }
                subItem->appendRow(row);
            }
        }
    }

    if (xml.hasError()) {
        qCritical() << xml.errorString();
        return;
    }
}

int VLKBInventoryTree::groupIndexOf(double lambda) const
{
    int idx = -1;
    for (int i = 0; i < this->groups.size(); ++i) {
        const WavelengthGroup &group = this->groups.at(i);
        if (lambda <= group.lambdaMax && lambda > group.lambdaMin) {
            return i;
        }
    }
    return idx;
}

void VLKBInventoryTree::setTreeStylePreferences(const QModelIndex &parent)
{
    // Expand 1st level (ie Continuum or Spectroscopy)
    auto model = qobject_cast<QStandardItemModel *>(ui->treeView->model());
    ui->treeView->expand(parent);

    // Adjust style to 2nd level
    for (int i = 0; i < this->groups.length(); ++i) {
        QModelIndex idx = model->index(i, 0, parent);
        if (model->hasChildren(idx)) {
            QFont f = model->itemFromIndex(idx)->font();
            f.setBold(true);
            model->itemFromIndex(idx)->setFont(f);
        } else {
            model->itemFromIndex(idx)->setForeground(QColor(Qt::gray));
        }
    }
}
