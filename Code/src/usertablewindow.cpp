#include "usertablewindow.h"
#include "ui_usertablewindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include "vialacteainitialquery.h"

UserTableWindow::UserTableWindow(QString filepath, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UserTableWindow),
    filepath(filepath)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->sourcesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->imagesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->cubesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    changeSelectionMode(ui->selectionComboBox->currentText());

    readFile();
}

UserTableWindow::~UserTableWindow()
{
    delete ui;
}

void UserTableWindow::readFile()
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "UserTableWindow.readFile" << filepath << file.errorString();
        QMessageBox::critical(this, "Error", file.errorString());
        return;
    }

    QTextStream in(&file);
    QStringList columns = in.readLine().split('\t');

    while (!in.atEnd()) {
        QStringList line = in.readLine().split('\t');
        QMap<QString, QString> map;
        for (int i = 0; i < columns.count(); ++i) {
            map.insert(columns[i], line[i]);
        }
        sources << map;
    }

    file.close();

    ui->idBox->addItems(columns);
    ui->glonBox->addItems(columns);
    ui->glatBox->addItems(columns);
    ui->loadButton->setEnabled(true);
}

void UserTableWindow::changeSelectionMode(const QString &selectionMode)
{
    if (selectionMode == "Point") {
        ui->dbLabel->hide();
        ui->dbLineEdit->hide();
        ui->dlLabel->hide();
        ui->dlLineEdit->hide();
        ui->radiusLabel->show();
        ui->radiusLineEdit->show();
    } else {
        ui->dbLabel->show();
        ui->dbLineEdit->show();
        ui->dlLabel->show();
        ui->dlLineEdit->show();
        ui->radiusLabel->hide();
        ui->radiusLineEdit->hide();
    }
}

void UserTableWindow::on_loadButton_clicked()
{
    ui->sourcesTable->setRowCount(0);

    QStringList tableColumns;
    tableColumns << QString() << ui->idBox->currentText() << ui->glonBox->currentText() << ui->glatBox->currentText();
    ui->sourcesTable->setColumnCount(4);
    ui->sourcesTable->setHorizontalHeaderLabels(tableColumns);

    foreach (const auto &source, sources) {
        ui->sourcesTable->insertRow(ui->sourcesTable->rowCount());
        auto checkBox = new QTableWidgetItem();
        checkBox->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        checkBox->setCheckState(Qt::Checked);
        ui->sourcesTable->setItem(ui->sourcesTable->rowCount() - 1, 0, checkBox);
        ui->sourcesTable->setItem(ui->sourcesTable->rowCount() - 1, 1, new QTableWidgetItem(source[ui->idBox->currentText()]));
        ui->sourcesTable->setItem(ui->sourcesTable->rowCount() - 1, 2, new QTableWidgetItem(source[ui->glonBox->currentText()]));
        ui->sourcesTable->setItem(ui->sourcesTable->rowCount() - 1, 3, new QTableWidgetItem(source[ui->glatBox->currentText()]));
    }

    ui->queryButton->setEnabled(true);
}

void UserTableWindow::on_queryButton_clicked()
{   
    qDeleteAll(selectedSources);
    selectedSources.clear();

    ui->imagesTable->setRowCount(0);
    ui->cubesTable->setRowCount(0);

    for (int i = 0; i < ui->sourcesTable->rowCount(); ++i) {
        if (ui->sourcesTable->item(i, 0)->checkState() == Qt::Checked) {
            QString designation = ui->sourcesTable->item(i, 1)->text();
            double glon = ui->sourcesTable->item(i, 2)->text().toDouble();
            double glat = ui->sourcesTable->item(i, 3)->text().toDouble();
            selectedSources.append(new Source(designation, glon, glat, this));
        }
    }

    if (!selectedSources.isEmpty()) {
        query(0);
    }
}

void UserTableWindow::query(int index)
{
    auto source = selectedSources.at(index);

    auto vlkb = new VialacteaInitialQuery();
    connect(vlkb, &VialacteaInitialQuery::searchDone, this, [this, vlkb, source, index](QList<QMap<QString, QString>> results){
        source->parseSearchResults(results);

        if ((index+1) < selectedSources.count()) {
            query(index+1);
        } else {
            updateTables();
            ui->tabWidget->setCurrentIndex(1);
        }
        vlkb->deleteLater();
    });

    if (ui->selectionComboBox->currentText() == "Point")
        vlkb->searchRequest(source->getGlon(), source->getGlat(), ui->radiusLineEdit->text().toDouble());
    else
        vlkb->searchRequest(source->getGlon(), source->getGlat(), ui->dlLineEdit->text().toDouble(), ui->dbLineEdit->text().toDouble());
}

//void UserTableWindow::parseResults(const QList<QMap<QString, QString>> &results)
//{
//    foreach (const auto &item, results) {
//        QString x = item.value("Survey") + item.value("Species") + item.value("Transition") + \
//                item.value("longitudeP1") + item.value("longitudeP2") + item.value("longitudeP3") + item.value("longitudeP4") + \
//                item.value("latitudeP1") + item.value("latitudeP2") + item.value("latitudeP3") + item.value("latitudeP4");

//        if (!set.contains(x)) {
//            auto &targetList = (item.value("Species").compare("Continuum") == 0) ? images : cubes;
//            targetList.append(item);
//            set.insert(x);
//        }
//    }
//}

void UserTableWindow::toggleFilter(QString transition)
{
    if (filters.contains(transition)) {
        filters.remove(transition);
    } else {
        filters.insert(transition);
    }

    ui->imagesTable->setRowCount(0);
    ui->cubesTable->setRowCount(0);
    updateTables();
}

void UserTableWindow::updateTables()
{
    ui->imagesTable->setRowCount(0);
    ui->cubesTable->setRowCount(0);

    const auto NewSurveyCheckBox = [](const Source *s, Qt::CheckState (Source::* f)(QString &tooltip) const) {
        QString tooltip;
        Qt::CheckState state = (s->*f)(tooltip);

        auto tmp = new QTableWidgetItem();
        tmp->setFlags(Qt::ItemIsUserTristate | Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
        tmp->setCheckState(state);
        tmp->setToolTip(tooltip);
        return tmp;
    };

    foreach (const auto &source, selectedSources) {
        qDebug() << "UserTableWindow.updateTables" << source->getDesignation() << source->getImages().count() << source->getCubes().count();

        ui->imagesTable->insertRow(ui->imagesTable->rowCount());

        auto x = new QTableWidgetItem();
        x->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        x->setCheckState(Qt::Unchecked);
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 0, x);
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 1, new QTableWidgetItem(source->getDesignation()));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 2, new QTableWidgetItem(QString::number(source->getGlon())));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 3, new QTableWidgetItem(QString::number(source->getGlat())));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 4, NewSurveyCheckBox(source, &Source::getInfoGlimpse));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 5, NewSurveyCheckBox(source, &Source::getInfoWise));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 6, NewSurveyCheckBox(source, &Source::getInfoMipsgal));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 7, NewSurveyCheckBox(source, &Source::getInfoHiGal));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 8, NewSurveyCheckBox(source, &Source::getInfoAtlasgal));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 9, NewSurveyCheckBox(source, &Source::getInfoBgps));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 10, NewSurveyCheckBox(source, &Source::getInfoCornish));
    }
}

void UserTableWindow::on_higal_70_checkBox_clicked()
{
    toggleFilter("70 um");
}

void UserTableWindow::on_higal_160_checkBox_clicked()
{
    toggleFilter("160 um");
}

void UserTableWindow::on_higal_250_checkBox_clicked()
{
    toggleFilter("250 um");
}

void UserTableWindow::on_higal_350_checkBox_clicked()
{
    toggleFilter("350 um");
}

void UserTableWindow::on_higal_500_checkBox_clicked()
{
    toggleFilter("500 um");
}

void UserTableWindow::on_mipsgal_24_checkBox_clicked()
{
    toggleFilter("24 um");
}

void UserTableWindow::on_glimpse_8_checkBox_clicked()
{
    toggleFilter("8.0 um");
}

void UserTableWindow::on_glimpse_58_checkBox_clicked()
{
    toggleFilter("5.8 um");
}

void UserTableWindow::on_glimpse_45_checkBox_clicked()
{
    toggleFilter("4.5 um");
}

void UserTableWindow::on_glimpse_36_checkBox_clicked()
{
    toggleFilter("3.6 um");
}

void UserTableWindow::on_wise_22_checkBox_clicked()
{
    toggleFilter("22 um");
}

void UserTableWindow::on_wise_12_checkBox_clicked()
{
    toggleFilter("12 um");
}

void UserTableWindow::on_wise_46_checkBox_clicked()
{
    toggleFilter("4.6 um");
}

void UserTableWindow::on_wise_34_checkBox_clicked()
{
    toggleFilter("3.4 um");
}

void UserTableWindow::on_atlasgal_870_checkBox_clicked()
{
    toggleFilter("870 um");
}

void UserTableWindow::on_bolocam_11_checkBox_clicked()
{
    toggleFilter("1.1 mm");
}

void UserTableWindow::on_cornish_5_checkBox_clicked()
{
    toggleFilter("5 GHz");
}

void UserTableWindow::on_selectionComboBox_activated(const QString &arg1)
{
    changeSelectionMode(arg1);
}

// -----------------------------------------------------------------------------------
Source::Source(const QString &designation, double glon, double glat, QObject *parent):
    QObject(parent),
    designation(designation),
    glon(glon),
    glat(glat)
{

}

const QString &Source::getDesignation() const
{
    return designation;
}

double Source::getGlon() const
{
    return glon;
}

double Source::getGlat() const
{
    return glat;
}

void Source::parseSearchResults(const QList<QMap<QString, QString>> &results)
{
    searchResults = results;

    foreach (const auto &item, results) {
        if (item.value("Species").compare("Continuum") == 0) {
            images.append(item);

            auto survey = item.value("Survey").toUpper();
            auto transition = item.value("Transition");

            if (survey.contains("HI-GAL")) {
                actualHiGal << transition;
                continue;
            }

            if (survey.contains("GLIMPSE")) {
                actualGlimpse << transition;
                continue;
            }

            if (survey.contains("WISE")) {
                actualWise << transition;
                continue;
            }

            if (survey.contains("MIPSGAL")) {
                actualMipsgal = true;
                continue;
            }

            if (survey.contains("ATLASGAL")) {
                actualAtlasgal = true;
                continue;
            }

            if (survey.contains("BGPS")) {
                actualBgps = true;
                continue;
            }

            if (survey.contains("CORNISH")) {
                actualCornish = true;
                continue;
            }
        } else {
            cubes.append(item);
        }
    }
}

const QList<QMap<QString, QString>> &Source::getImages() const
{
    return images;
}

const QList<QMap<QString, QString>> &Source::getCubes() const
{
    return cubes;
}

Qt::CheckState Source::getInfoHiGal(QString &tooltipText) const
{
    tooltipText.clear();
    foreach (const QString &vw, expectedHiGal) {
        tooltipText += vw + " : " + (actualHiGal.contains(vw) ? "Yes" : "No") + "\n";
    }
    tooltipText.chop(1);

    if (actualHiGal == expectedHiGal)
        return Qt::Checked;

    if (actualHiGal.count() == 0)
        return Qt::Unchecked;

    return Qt::PartiallyChecked;
}

Qt::CheckState Source::getInfoGlimpse(QString &tooltipText) const
{
    tooltipText.clear();
    foreach (const QString &vw, expectedGlimpse) {
        tooltipText += vw + " : " + (actualGlimpse.contains(vw) ? "Yes" : "No") + "\n";
    }
    tooltipText.chop(1);

    if (actualGlimpse == expectedGlimpse)
        return Qt::Checked;

    if (actualGlimpse.count() == 0)
        return Qt::Unchecked;

    return Qt::PartiallyChecked;
}

Qt::CheckState Source::getInfoWise(QString &tooltipText) const
{
    tooltipText.clear();
    foreach (const QString &vw, expectedWise) {
        tooltipText += vw + " : " + (actualWise.contains(vw) ? "Yes" : "No") + "\n";
    }
    tooltipText.chop(1);

    if (actualWise == expectedWise)
        return Qt::Checked;

    if (actualWise.count() == 0)
        return Qt::Unchecked;

    return Qt::PartiallyChecked;
}

Qt::CheckState Source::getInfoMipsgal(QString &tooltipText) const
{
    if (actualMipsgal) {
        tooltipText = expectedMipsgal + " : Yes";
        return Qt::Checked;
    } else {
        tooltipText = expectedMipsgal + " : No";
        return Qt::Unchecked;
    }
}

Qt::CheckState Source::getInfoAtlasgal(QString &tooltipText) const
{
    if (actualAtlasgal) {
        tooltipText = expectedAtlasgal + " : Yes";
        return Qt::Checked;
    } else {
        tooltipText = expectedAtlasgal + " : No";
        return Qt::Unchecked;
    }
}

Qt::CheckState Source::getInfoBgps(QString &tooltipText) const
{
    if (actualBgps) {
        tooltipText = expectedBgps + " : Yes";
        return Qt::Checked;
    } else {
        tooltipText = expectedBgps + " : No";
        return Qt::Unchecked;
    }
}

Qt::CheckState Source::getInfoCornish(QString &tooltipText) const
{
    if (actualCornish) {
        tooltipText = expectedCornish + " : Yes";
        return Qt::Checked;
    } else {
        tooltipText = expectedCornish + " : No";
        return Qt::Unchecked;
    }
}
