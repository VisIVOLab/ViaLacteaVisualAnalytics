#include "sessionloader.h"
#include "ui_sessionloader.h"

#include "astroutils.h"
#include "vialactea.h"
#include "vialacteainitialquery.h"
#include "vtkfitsreader.h"
#include "vtkwindow_new.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

SessionLoader::SessionLoader(QWidget *parent, const QString &sessionFilepath)
    : QMainWindow(parent),
      ui(new Ui::SessionLoader),
      sessionFilepath(sessionFilepath),
      sessionRootFolder(QFileInfo(sessionFilepath).dir()),
      originLayerIdx(-1)
{
    ui->setupUi(this);
    ui->tableFits->addAction(ui->actionSetOriginLayer);
    ui->tableFits->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(ui->actionSetOriginLayer, &QAction::triggered, this, &SessionLoader::setOriginLayer);
    setAttribute(Qt::WA_DeleteOnClose);

    QFile sessionFile(sessionFilepath);
    sessionFile.open(QFile::ReadOnly);
    root = QJsonDocument::fromJson(sessionFile.readAll()).object();
    sessionFile.close();

    if (root.isEmpty()) {
        QMessageBox::critical(this, tr("Load a session"),
            tr("Failed to parse the session configuration.\n"
               "Loading aborted."));
        return;
    }

    layers = root["image"].toObject()["layers"].toArray();
    for (int i = 0; i < layers.count(); ++i) {
        if (layers.at(i).toObject()["origin"].toBool()) {
            originLayerIdx = i;
            break;
        }
    }

    datacubes = root["datacubes"].toArray();

    initTable();
}

SessionLoader::~SessionLoader()
{
    delete ui;
}

void SessionLoader::setOriginLayer()
{
    int newIdx = ui->tableFits->currentItem()->row();
    if (newIdx == originLayerIdx) {
        return;
    }

    if (newIdx >= layers.count()) {
        QMessageBox::warning(this, "", "The origin layer must be an image.");
        return;
    }

    foreach (auto &&index, overlapFailures) {
        setFitsRowColor(index);
    }
    overlapFailures.clear();

    setFitsRowBold(originLayerIdx, false);
    setFitsRowBold(newIdx, true);
    originLayerIdx = newIdx;
    ui->tableFits->item(originLayerIdx, 0)->setCheckState(Qt::Checked);
}

void SessionLoader::setFitsRowBold(int row, bool bold)
{
    if (row >= 0 && row < ui->tableFits->rowCount()) {
        auto font = ui->tableFits->item(row, 0)->font();
        font.setBold(bold);
        ui->tableFits->item(row, 1)->setFont(font);
        ui->tableFits->item(row, 2)->setFont(font);
    }
}

void SessionLoader::setFitsRowColor(int row, const QBrush &b)
{
    if (row >= 0 && row < ui->tableFits->rowCount()) {
        ui->tableFits->item(row, 0)->setBackground(b);
        ui->tableFits->item(row, 1)->setBackground(b);
        ui->tableFits->item(row, 2)->setBackground(b);
    }
}

void SessionLoader::initTable()
{
    auto createCheckItem = [](bool checked) -> QTableWidgetItem* {
        auto check = new QTableWidgetItem;
        check->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        if (checked) {
            check->setCheckState(Qt::Checked);
        } else {
            check->setCheckState(Qt::Unchecked);
        }
        return check;
    };

    foreach (auto&& it, layers) {
        QJsonObject layer = it.toObject();
        bool enabled = layer["enabled"].toBool();
        QString type = layer["type"].toString("Continuum");
        QString fits = layer["fits"].toString();

        int row = ui->tableFits->rowCount();
        ui->tableFits->insertRow(row);
        ui->tableFits->setItem(row, 0, createCheckItem(enabled));
        ui->tableFits->setItem(row, 1, new QTableWidgetItem(type));
        ui->tableFits->setItem(row, 2, new QTableWidgetItem(fits));
    }

    foreach (auto &&it, datacubes) {
        auto cube = it.toObject();
        bool enabled = cube["enabled"].toBool();
        QString type = "Datacube";
        QString fits = cube["fits"].toString();

        int row = ui->tableFits->rowCount();
        ui->tableFits->insertRow(row);
        ui->tableFits->setItem(row, 0, createCheckItem(enabled));
        ui->tableFits->setItem(row, 1, new QTableWidgetItem(type));
        ui->tableFits->setItem(row, 2, new QTableWidgetItem(fits));
    }

    setFitsRowBold(originLayerIdx, true);
}

bool SessionLoader::testOverlaps()
{
    foreach (auto &&index, overlapFailures) {
        setFitsRowColor(index);
    }
    overlapFailures.clear();

    bool ok = true;

    auto origin = layers.at(originLayerIdx).toObject();
    auto originFits = sessionRootFolder.absoluteFilePath(origin["fits"].toString());

    for (int i = 0; i < ui->tableFits->rowCount(); ++i) {
        if (i == originLayerIdx) {
            continue;
        }

        if (ui->tableFits->item(i, 0)->checkState() == Qt::Unchecked) {
            continue;
        }

        QString fits;
        if (i < layers.count()) {
            fits = layers.at(i).toObject()["fits"].toString();
        } else {
            fits = datacubes.at(i - layers.count()).toObject()["fits"].toString();
        }
        QString fitsPath = sessionRootFolder.absoluteFilePath(fits);

        if (!AstroUtils::CheckOverlap(originFits.toStdString(), fitsPath.toStdString())) {
            ok = false;
            overlapFailures.insert(i);
        }
    }

    foreach (auto &&index, overlapFailures) {
        setFitsRowColor(index, Qt::red);
    }

    return ok;
}

void SessionLoader::on_btnLoad_clicked()
{
    if (originLayerIdx < 0) {
        QMessageBox::warning(this, "Incomplete session",
            "Please select an origin layer (right click) to continue.");
        return;
    }
    ui->tableFits->item(originLayerIdx, 0)->setCheckState(Qt::Checked);

    if (!testOverlaps()) {
        QMessageBox::warning(
            this, "Overlaps",
            "Some layers do not overlap.\n"
            "Remove them or change the origin layer (right click).");
        return;
    }

    for (int i = 0; i < ui->tableFits->rowCount(); ++i) {
        auto enabled = (ui->tableFits->item(i, 0)->checkState() == Qt::Checked);
        if (i < layers.count()) {
            auto layer = layers.at(i).toObject();
            if (i == originLayerIdx) {
                layer["origin"] = true;
                layer["show"] = true;
            }
            layer["enabled"] = enabled;
            layers[i] = layer;
        } else {
            auto dc = datacubes.at(i - layers.count()).toObject();
            dc["enabled"] = enabled;
            datacubes[i - layers.count()] = dc;
        }
    }

    root["datacubes"] = datacubes;
    auto image = root["image"].toObject();
    image["layers"] = layers;
    root["image"] = image;
    QJsonDocument doc(root);
    QFile f(sessionFilepath);
    f.open(QFile::WriteOnly);
    f.write(doc.toJson());
    f.close();

    auto originLayer = layers.at(originLayerIdx).toObject();
    QString fits = sessionRootFolder.absoluteFilePath(originLayer["fits"].toString());
    auto fitsReader = vtkSmartPointer<vtkFitsReader>::New();
    fitsReader->SetFileName(fits.toStdString());
    if (originLayer["type"].toString() == "Moment") {
        fitsReader->isMoment3D = true;
        fitsReader->setMomentOrder(originLayer["moment_order"].toInt(0));
    }

    auto vl = qobject_cast<ViaLactea*>(parent());
    if (ui->checkQuery->isChecked()) {
        double coords[2], rectSize[2];
        AstroUtils::GetCenterCoords(fits.toStdString(), coords);
        AstroUtils::GetRectSize(fits.toStdString(), rectSize);
        auto vq = new VialacteaInitialQuery;
        vq->setParent(this);
        connect(vq, &VialacteaInitialQuery::searchDone, this,
            [this, vl, fitsReader](QList<QMap<QString, QString>> results) {
                auto win = new vtkwindow_new(vl, fitsReader);
                win->setDbElements(results);
                win->loadSession(sessionFilepath, sessionRootFolder);
                vl->setMasterWin(win);
                this->close();
            });
        vq->searchRequest(coords[0], coords[1], rectSize[0], rectSize[1]);
    } else {
        auto win = new vtkwindow_new(vl, fitsReader);
        win->loadSession(sessionFilepath, sessionRootFolder);
        auto vl = qobject_cast<ViaLactea*>(parent());
        vl->setMasterWin(win);
        this->close();
    }
}
