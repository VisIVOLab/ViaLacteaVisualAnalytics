#include "sourcewidget.h"
#include "ui_sourcewidget.h"

#include "catalogue.h"
#include "source.h"

#include <QListWidgetItem>

SourceWidget::SourceWidget(QWidget *parent, Catalogue *catalogue)
    : QWidget(parent), ui(new Ui::SourceWidget), catalogue(catalogue)
{
    ui->setupUi(this);

    auto fillList = [this, catalogue]() -> void {
        ui->listExtracted->clear();

        foreach (auto &&name, catalogue->getExtractedNames().keys()) {
            ui->listExtracted->addItem(name);
        }
    };

    fillList();
    connect(catalogue, &Catalogue::SourceExtracted, this, fillList);
}

SourceWidget::~SourceWidget()
{
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
