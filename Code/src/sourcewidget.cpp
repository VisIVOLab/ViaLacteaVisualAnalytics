#include "sourcewidget.h"
#include "ui_sourcewidget.h"

#include "catalogue.h"
#include "source.h"
#include "vtkwindow_new.h"

#include <vtkLODActor.h>

#include <QInputDialog>
#include <QListWidgetItem>
#include <QMessageBox>

SourceWidget::SourceWidget(QWidget *parent, Catalogue *catalogue, vtkwindow_new *win)
    : QWidget(parent), ui(new Ui::SourceWidget), catalogue(catalogue), win(win), editing(false)
{
    ui->setupUi(this);

    auto fillListExtracted = [this, catalogue]() -> void {
        ui->listExtracted->clear();
        ui->listExtracted->addItems(catalogue->getExtractedNames().keys());

        if (ui->listExtracted->count() > 0) {
            emit ui->listExtracted->itemClicked(ui->listExtracted->item(0));
        }
    };

    fillListExtracted();
    connect(catalogue, &Catalogue::SourcesExtracted, this, fillListExtracted);
    connect(catalogue, &Catalogue::ExtractedSourcesUpdated, this, fillListExtracted);

    ui->listFiltered->addItems(catalogue->getFilteredIds());
    connect(catalogue, &Catalogue::SourcesFiltered, this, [this](const QStringList &ids) {
        ui->listFiltered->clear();
        ui->listFiltered->addItems(ids);
    });
}

SourceWidget::~SourceWidget()
{
    win->setVtkInteractorStyleImage();
    delete ui;
}

void SourceWidget::showIslandInfo(const Island *island)
{
    // Source box
    Source *source = island->getSource();
    QString pos = QString("(%1, %2)")
                          .arg(QString::number(source->getX0(), 'f', 6),
                               QString::number(source->getY0(), 'f', 6));
    QString pos_wcs = QString("(%1, %2)")
                              .arg(QString::number(source->getRa(), 'f', 6),
                                   QString::number(source->getDec(), 'f', 6));

    ui->labelSourceIau->setText(source->getIauName());
    ui->labelSourceClassId->setText(QString::number(source->getClassid()));
    ui->labelSourceLabel->setText(source->getLabel());
    ui->labelNumIslands->setText(QString::number(source->getNIslands()));
    ui->labelSourcePos->setText(pos);
    ui->labelSourcePosWCS->setText(pos_wcs);
    ui->labelSourceMorphLabel->setText(source->getMorphLabel());
    ui->labelSourceSourcenessLabel->setText(source->getSourcenessLabel());
    ui->labelSourceSourcenessScore->setText(QString::number(source->getSourcenessScore(), 'f', 6));

    // Island box
    pos = QString("(%1, %2)\nbox(%3, %4, %5, %6)")
                  .arg(QString::number(island->getX(), 'f', 6))
                  .arg(QString::number(island->getY(), 'f', 6))
                  .arg(QString::number(island->getXmin(), 'f', 2))
                  .arg(QString::number(island->getXmax(), 'f', 2))
                  .arg(QString::number(island->getYmin(), 'f', 2))
                  .arg(QString::number(island->getYmax(), 'f', 2));
    pos_wcs = QString("(%1, %2)\nbox(%3, %4, %5, %6)")
                      .arg(QString::number(island->getRa(), 'f', 6))
                      .arg(QString::number(island->getDec(), 'f', 6))
                      .arg(QString::number(island->getRaMin(), 'f', 2))
                      .arg(QString::number(island->getRaMax(), 'f', 2))
                      .arg(QString::number(island->getDecMin(), 'f', 2))
                      .arg(QString::number(island->getDecMax(), 'f', 2));
    ui->labelIslandIau->setText(island->getIauName());
    ui->labelIslandNpix->setText(QString::number(island->getNpix()));
    ui->labelIslandPos->setText(pos);
    ui->labelIslandPosWCS->setText(pos_wcs);
    ui->labelIslandFlux->setText(QString("%1 +- %2")
                                         .arg(QString::number(island->getS(), 'f', 6))
                                         .arg(QString::number(island->getSErr(), 'f', 6)));
    ui->labelIslandS->setText(QString("%1, %2")
                                      .arg(QString::number(island->getSmax(), 'f', 6))
                                      .arg(QString::number(island->getStot(), 'f', 6)));
    ui->labelIslandBkgRms->setText(QString("%1, %2")
                                           .arg(QString::number(island->getBkg(), 'f', 6))
                                           .arg(QString::number(island->getRms(), 'f', 6)));
    ui->labelIslandBeam->setText(QString::number(island->getBeamAreaRatioPar(), 'f', 6));
    ui->labelIslandResolved->setText(island->getResolved() ? "true" : "false");
    ui->labelIslandBorder->setText(island->getBorder() ? "true" : "false");
    ui->labelIslandSourcenessLabel->setText(island->getSourcenessLabel());
    ui->labelIslandSourcenessScore->setText(QString::number(island->getSourcenessScore(), 'f', 6));
}

void SourceWidget::on_listExtracted_itemClicked(QListWidgetItem *item)
{
    auto source = catalogue->getSource(item->text());
    auto island = source->getIslands().first();
    showIslandInfo(island);
}

void SourceWidget::on_btnEdit_clicked()
{
    auto item = ui->listExtracted->currentItem();
    if (!item || item->text().isEmpty()) {
        return;
    }

    if (editing) {
        win->setVtkInteractorStyleImage();

        qDebug() << Q_FUNC_INFO << "After editing";
        auto source = catalogue->getExtractedNames().value(iau_name);
        auto p = source.first;
        for (int i = 0; i < p->GetNumberOfPoints(); ++i) {
            double coords[3];
            p->GetPoint(i, coords);
            qDebug() << Q_FUNC_INFO << "P" << i << coords[0] << coords[1];
        }

        catalogue->updatePoints(iau_name, source.first);
    } else {
        iau_name = item->text();
        auto source = catalogue->getExtractedNames().value(iau_name);
        win->setVtkInteractorEditSource(source);

        qDebug() << Q_FUNC_INFO << "Before editing";
        auto p = source.first;
        for (int i = 0; i < p->GetNumberOfPoints(); ++i) {
            double coords[3];
            p->GetPoint(i, coords);
            qDebug() << Q_FUNC_INFO << "P" << i << coords[0] << coords[1];
        }
    }

    editing = !editing;
    ui->btnEdit->setDown(editing);
}

void SourceWidget::on_btnSave_clicked()
{
    if (editing) {
        QMessageBox::warning(this, "Message", "Exit edit mode first!");
        return;
    }

    catalogue->save();
}

void SourceWidget::on_btnDelete_clicked()
{
    if (editing) {
        QMessageBox::warning(this, "Message", "Exit edit mode first!");
        return;
    }

    auto item = ui->listExtracted->currentItem();
    if (!item || item->text().isEmpty()) {
        return;
    }

    catalogue->removeSource(item->text());
}

void SourceWidget::on_btnRename_clicked()
{
    if (editing) {
        QMessageBox::warning(this, "Message", "Exit edit mode first!");
        return;
    }

    auto item = ui->listExtracted->currentItem();
    if (!item || item->text().isEmpty()) {
        return;
    }

    bool ok;
    QString new_name = QInputDialog::getText(this, "Rename",
                                             "Enter a new name for the source: " + item->text(),
                                             QLineEdit::Normal, item->text(), &ok);
    if (!ok || new_name.isEmpty()) {
        return;
    }

    if (catalogue->getSource(new_name) != nullptr) {
        QMessageBox::warning(this, "Rename", "A source with that name already exists!");
        return;
    }

    catalogue->renameSource(item->text(), new_name);
}

void SourceWidget::on_listFiltered_itemClicked(QListWidgetItem *item)
{
    auto source = catalogue->getSource(item->text());
    auto island = source->getIslands().first();
    showIslandInfo(island);
}

void SourceWidget::on_btnMorphLabel_clicked()
{
    QStringList morph_label_val { "UNKNOWN",  "COMPACT", "POINT-LIKE",
                                  "EXTENDED", "DIFFUSE", "COMPACT-EXTENDED" };

    bool ok;
    QString item = QInputDialog::getItem(this, "Set morph_label", "morph_label:", morph_label_val,
                                         0, false, &ok);

    if (ok && !item.isEmpty()) {
        foreach (auto &&iau_name, catalogue->getFilteredIds()) {
            catalogue->updateMorphLabel(iau_name, item);
        }
        QMessageBox::information(this, "Set morph_label", "Field changed to all filtered sources.");
    }
}

void SourceWidget::on_btnDeleteAll_clicked()
{
    catalogue->removeFilteredSources();
}
