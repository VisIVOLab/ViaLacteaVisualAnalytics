#include "usertablewindow.h"
#include "ui_usertablewindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include "vialacteainitialquery.h"

UserTableWindow::UserTableWindow(QString filepath, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UserTableWindow),
    filepath(filepath)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->tabWidget->setTabEnabled(1, false);
    ui->tabWidget->setTabEnabled(2, false);

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
    loadSourceTable(columns);
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

void UserTableWindow::loadSourceTable(const QStringList &columns)
{
    ui->sourcesTable->setRowCount(0);

    QStringList tableColumns;
    // Prepend a new empty column for the checkbox
    tableColumns << QString() << columns;

    ui->sourcesTable->setColumnCount(tableColumns.count());
    ui->sourcesTable->setHorizontalHeaderLabels(tableColumns);

    foreach (const auto &source, sources) {
        ui->sourcesTable->insertRow(ui->sourcesTable->rowCount());
        int row = ui->sourcesTable->rowCount() - 1;

        auto checkBox = new QTableWidgetItem();
        checkBox->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        checkBox->setCheckState(Qt::Checked);

        ui->sourcesTable->setItem(ui->sourcesTable->rowCount() - 1, 0, checkBox);
        for (int col = 1; col < tableColumns.count(); ++col) {
            ui->sourcesTable->setItem(row, col, new QTableWidgetItem(source[tableColumns[col]]));
        }
    }

    ui->queryButton->setEnabled(true);
}

void UserTableWindow::on_queryButton_clicked()
{
    // Clean up the previous search
    qDeleteAll(selectedSources);
    selectedSources.clear();
    ui->imagesTable->setRowCount(0);
    ui->cubesTable->setRowCount(0);

    // Add 1 to skip the first column (the checkbox column)
    int designationCol = ui->idBox->currentIndex() + 1;
    int glonCol = ui->glonBox->currentIndex() + 1;
    int glatCol = ui->glatBox->currentIndex() + 1;

    for (int i = 0; i < ui->sourcesTable->rowCount(); ++i) {
        if (ui->sourcesTable->item(i, 0)->checkState() == Qt::Checked) {
            QString designation = ui->sourcesTable->item(i, designationCol)->text();
            double glon = ui->sourcesTable->item(i, glonCol)->text().toDouble();
            double glat = ui->sourcesTable->item(i, glatCol)->text().toDouble();
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
            ui->tabWidget->setTabEnabled(1, true);
            ui->tabWidget->setTabEnabled(2, true);
            ui->tabWidget->setCurrentIndex(1);
        }
        vlkb->deleteLater();
    });

    if (ui->selectionComboBox->currentText() == "Point")
        vlkb->searchRequest(source->getGlon(), source->getGlat(), ui->radiusLineEdit->text().toDouble());
    else
        vlkb->searchRequest(source->getGlon(), source->getGlat(), ui->dlLineEdit->text().toDouble(), ui->dbLineEdit->text().toDouble());
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

        // Images Table
        ui->imagesTable->insertRow(ui->imagesTable->rowCount());
        QTableWidgetItem *x = new QTableWidgetItem();
        x->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        x->setCheckState(Qt::Unchecked);
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 0, x);
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 1, new QTableWidgetItem(source->getDesignation()));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 2, new QTableWidgetItem(QString::number(source->getGlon())));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 3, new QTableWidgetItem(QString::number(source->getGlat())));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 4, new QTableWidgetItem(QString::number(source->getImages().count())));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 5, NewSurveyCheckBox(source, &Source::getInfoGlimpse));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 6, NewSurveyCheckBox(source, &Source::getInfoWise));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 7, NewSurveyCheckBox(source, &Source::getInfoMipsgal));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 8, NewSurveyCheckBox(source, &Source::getInfoHiGal));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 9, NewSurveyCheckBox(source, &Source::getInfoAtlasgal));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 10, NewSurveyCheckBox(source, &Source::getInfoBgps));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 11, NewSurveyCheckBox(source, &Source::getInfoCornish));

        // Cubes Table
        ui->cubesTable->insertRow(ui->cubesTable->rowCount());
        x = new QTableWidgetItem();
        x->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        x->setCheckState(Qt::Unchecked);
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 0, x);
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 1, new QTableWidgetItem(source->getDesignation()));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 2, new QTableWidgetItem(QString::number(source->getGlon())));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 3, new QTableWidgetItem(QString::number(source->getGlat())));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 4, NewSurveyCheckBox(source, &Source::getInfoMopra));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 5, NewSurveyCheckBox(source, &Source::getInfoChimps));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 6, NewSurveyCheckBox(source, &Source::getInfoChamp));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 7, NewSurveyCheckBox(source, &Source::getInfoHops));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 8, NewSurveyCheckBox(source, &Source::getInfoGrs));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 9, NewSurveyCheckBox(source, &Source::getInfoMalt));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 10, NewSurveyCheckBox(source, &Source::getInfoThrumms));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 11, NewSurveyCheckBox(source, &Source::getInfoNanten));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 12, NewSurveyCheckBox(source, &Source::getInfoOgs));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 13, NewSurveyCheckBox(source, &Source::getInfoCohrs));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 14, NewSurveyCheckBox(source, &Source::getInfoVgps));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 15, NewSurveyCheckBox(source, &Source::getInfoCgps));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 16, NewSurveyCheckBox(source, &Source::getInfoSgps));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 17, NewSurveyCheckBox(source, &Source::getInfoAro));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 18, NewSurveyCheckBox(source, &Source::getInfoThor));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 19, NewSurveyCheckBox(source, &Source::getInfoSedigism));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 20, NewSurveyCheckBox(source, &Source::getInfoFugin));
    }
}

void UserTableWindow::setFilter(const QString &survey, const QString &transition, bool on)
{
    if (on) {
        filters.insert(QPair<QString, QString>(survey, transition));
    } else {
        filters.remove(QPair<QString, QString>(survey, transition));
    }

    qDebug() << "Filters" << filters;
}

void UserTableWindow::downloadImages(const QMap<QString, QStringList> &cutouts, const QDir &dir)
{
    auto vlkb = new VialacteaInitialQuery();
    for (auto i = cutouts.constBegin(); i != cutouts.constEnd(); ++i) {
        dir.mkdir(i.key());
        foreach (const auto &link, i.value()) {
            vlkb->cutoutRequest(link, QDir(dir.absoluteFilePath(i.key())));
        }
    }
}

void UserTableWindow::on_downloadImagesButton_clicked()
{
    QMap<QString, QStringList> cutouts;
    for (int row = 0; row < ui->imagesTable->rowCount(); ++row) {
       QTableWidgetItem *item = ui->imagesTable->item(row, 0);
       if (item->checkState() == Qt::Checked) {
           const Source *source = selectedSources.at(row);
           QStringList downloadLinks;

           foreach (const auto image, source->getImages()) {
               QPair<QString, QString> pair(image.value("Survey"), image.value("Transition"));
               if (filters.contains(pair)) {
                   downloadLinks << image.value("URL");
               }
           }

           if (!downloadLinks.isEmpty()) {
               cutouts.insert(source->getDesignation(), downloadLinks);
           }
       }
    }

    if (cutouts.isEmpty()) {
        // Nothing to do
        return;
    }

    QString tmp = QFileDialog::getExistingDirectory(this, "Select a folder", QDir::homePath(), QFileDialog::ShowDirsOnly);
    if (!tmp.isEmpty()) {
        downloadImages(cutouts, QDir(tmp));
    }
}

void UserTableWindow::on_selectionComboBox_activated(const QString &arg1)
{
    changeSelectionMode(arg1);
}

void UserTableWindow::on_higal_70_checkBox_clicked(bool checked)
{
    setFilter("Hi-GAL Tiles", "70 um", checked);
    setFilter("Hi-GAL Mosaic", "70 um", checked);
}

void UserTableWindow::on_higal_160_checkBox_clicked(bool checked)
{
    setFilter("Hi-GAL Tiles", "160 um", checked);
    setFilter("Hi-GAL Mosaic", "160 um", checked);
}

void UserTableWindow::on_higal_250_checkBox_clicked(bool checked)
{
    setFilter("Hi-GAL Tiles", "250 um", checked);
    setFilter("Hi-GAL Mosaic", "250 um", checked);
}

void UserTableWindow::on_higal_350_checkBox_clicked(bool checked)
{
    setFilter("Hi-GAL Tiles", "350 um", checked);
    setFilter("Hi-GAL Mosaic", "350 um", checked);
}

void UserTableWindow::on_higal_500_checkBox_clicked(bool checked)
{
    setFilter("Hi-GAL Tiles", "500 um", checked);
    setFilter("Hi-GAL Mosaic", "500 um", checked);
}

void UserTableWindow::on_glimpse_36_checkBox_clicked(bool checked)
{
    setFilter("GLIMPSE I", "3.6 um", checked);
    setFilter("GLIMPSE II", "3.6 um", checked);
}

void UserTableWindow::on_glimpse_45_checkBox_clicked(bool checked)
{
    setFilter("GLIMPSE I", "4.5 um", checked);
    setFilter("GLIMPSE II", "4.5 um", checked);
}

void UserTableWindow::on_glimpse_58_checkBox_clicked(bool checked)
{
    setFilter("GLIMPSE I", "5.8 um", checked);
    setFilter("GLIMPSE II", "5.8 um", checked);
}

void UserTableWindow::on_glimpse_80_checkBox_clicked(bool checked)
{
    setFilter("GLIMPSE I", "8.0 um", checked);
    setFilter("GLIMPSE II", "8.0 um", checked);
}

void UserTableWindow::on_wise_34_checkBox_clicked(bool checked)
{
    setFilter("WISE", "3.4 um", checked);
}

void UserTableWindow::on_wise_46_checkBox_clicked(bool checked)
{
    setFilter("WISE", "5.6 um", checked);
}

void UserTableWindow::on_wise_12_checkBox_clicked(bool checked)
{
    setFilter("WISE", "12 um", checked);
}

void UserTableWindow::on_wise_22_checkBox_clicked(bool checked)
{
    setFilter("WISE", "22 um", checked);
}

void UserTableWindow::on_atlasgal_870_checkBox_clicked(bool checked)
{
    setFilter("ATLASGAL", "870 um", checked);
}

void UserTableWindow::on_mipsgal_24_checkBox_clicked(bool checked)
{
    setFilter("MIPSGAL", "24 um", checked);
}

void UserTableWindow::on_bgps_11_checkBox_clicked(bool checked)
{
    setFilter("CSO BGPS", "1.1 mm", checked);
}

void UserTableWindow::on_cornish_5_checkBox_clicked(bool checked)
{
    setFilter("CORNISH", "5 GHz", checked);
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
        auto survey = item.value("Survey").toUpper();
        auto species = item.value("Species");

        if (species.compare("Continuum") == 0) {
            images.append(item);

            auto transition = item.value("Transition");

            if (survey == "HI-GAL TILES" || survey == "HI-GAL MOSAIC") {
                actualHiGal << transition;
                continue;
            }

            if (survey == "GLIMPSE I" || survey == "GLIMPSE II") {
                actualGlimpse << transition;
                continue;
            }

            if (survey == "WISE") {
                actualWise << transition;
                continue;
            }

            if (survey == "MIPSGAL") {
                actualMipsgal = true;
                continue;
            }

            if (survey == "ATLASGAL") {
                actualAtlasgal = true;
                continue;
            }

            if (survey == "CSO BGPS") {
                actualBgps = true;
                continue;
            }

            if (survey == "CORNISH") {
                actualCornish = true;
                continue;
            }
        } else {
            cubes.append(item);

            if (survey.contains("MOPRA")) {
                actualMopra << species;
                continue;
            }

            if (survey.contains("CHIMPS")) {
                actualChimps << species;
                continue;
            }

            if (survey.contains("CHAMP")) {
                actualChamp = true;
                continue;
            }

            if (survey.contains("HOPS")) {
                actualHops << species;
                continue;
            }

            if (survey.contains("GRS")) {
                actualGrs = true;
                continue;
            }

            if (survey.contains("MALT90")) {
                actualMalt << species;
                continue;
            }

            if (survey.contains("THRUMMS")) {
                actualThrumms << species;
                continue;
            }

            if (survey.contains("NANTEN")) {
                actualNanten = true;
                continue;
            }

            if (survey.contains("OGS")) {
                actualOgs << species;
                continue;
            }

            if (survey.contains("COHRS")) {
                actualCohrs = true;
                continue;
            }

            if (survey.contains("VGPS")) {
                actualVgps = true;
                continue;
            }

            if (survey.contains("CGPS")) {
                actualCgps = true;
                continue;
            }

            if (survey.contains("SGPS")) {
                actualSgps = true;
                continue;
            }

            if (survey.contains("ARO")) {
                actualAro << species;
                continue;
            }

            if (survey.contains("THOR") && !species.contains("Continuum")) {
                actualThor << species;
                continue;
            }

            if (survey.contains("SEDIGISM")) {
                actualSedigism << species;
                continue;
            }

            if (survey.contains("FUGIN")) {
                actualFugin << species;
                continue;
            }
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

Qt::CheckState Source::testSet(QString &tooltipText, const QSet<QString> &expected, const QSet<QString> &actual) const
{
    tooltipText.clear();
    foreach (const QString &el, expected) {
        tooltipText += el + " : " + (actual.contains(el) ? "Yes" : "No") + "\n";
    }
    tooltipText.chop(1);

    if (actual == expected)
        return Qt::Checked;

    if (actual.count() == 0)
        return Qt::Unchecked;

    return Qt::PartiallyChecked;
}

Qt::CheckState Source::testSingle(QString &tooltipText, const QString &expected, bool actual) const
{
    if (actual) {
        tooltipText = expected + " : Yes";
        return Qt::Checked;
    } else {
        tooltipText = expected + " : No";
        return Qt::Unchecked;
    }
}

Qt::CheckState Source::getInfoHiGal(QString &tooltipText) const
{
    return testSet(tooltipText, expectedHiGal, actualHiGal);
}

Qt::CheckState Source::getInfoGlimpse(QString &tooltipText) const
{
    return testSet(tooltipText, expectedGlimpse, actualGlimpse);
}

Qt::CheckState Source::getInfoWise(QString &tooltipText) const
{
    return testSet(tooltipText, expectedWise, actualWise);
}

Qt::CheckState Source::getInfoMipsgal(QString &tooltipText) const
{
    return testSingle(tooltipText, expectedMipsgal, actualMipsgal);
}

Qt::CheckState Source::getInfoAtlasgal(QString &tooltipText) const
{
    return testSingle(tooltipText, expectedAtlasgal, actualAtlasgal);
}

Qt::CheckState Source::getInfoBgps(QString &tooltipText) const
{
    return testSingle(tooltipText, expectedBgps, actualBgps);
}

Qt::CheckState Source::getInfoCornish(QString &tooltipText) const
{
    return testSingle(tooltipText, expectedCornish, actualCornish);
}

Qt::CheckState Source::getInfoMopra(QString &tooltipText) const
{
    return testSet(tooltipText, expectedMopra, actualMopra);
}

Qt::CheckState Source::getInfoChimps(QString &tooltipText) const
{
    return testSet(tooltipText, expectedChimps, actualChimps);
}

Qt::CheckState Source::getInfoChamp(QString &tooltipText) const
{
    return testSingle(tooltipText, expectedChamp, actualChamp);
}

Qt::CheckState Source::getInfoHops(QString &tooltipText) const
{
    return testSet(tooltipText, expectedHops, actualHops);
}

Qt::CheckState Source::getInfoGrs(QString &tooltipText) const
{
    return testSingle(tooltipText, expectedGrs, actualGrs);
}

Qt::CheckState Source::getInfoMalt(QString &tooltipText) const
{
    return testSet(tooltipText, expectedMalt, actualMalt);
}

Qt::CheckState Source::getInfoThrumms(QString &tooltipText) const
{
    return testSet(tooltipText, expectedThrumms, actualThrumms);
}

Qt::CheckState Source::getInfoNanten(QString &tooltipText) const
{
    return testSingle(tooltipText, expectedNanten, actualNanten);
}

Qt::CheckState Source::getInfoOgs(QString &tooltipText) const
{
    return testSet(tooltipText, expectedOgs, actualOgs);
}

Qt::CheckState Source::getInfoCohrs(QString &tooltipText) const
{
    return testSingle(tooltipText, expectedCohrs, actualCohrs);
}

Qt::CheckState Source::getInfoVgps(QString &tooltipText) const
{
    return testSingle(tooltipText, expectedVgps, actualVgps);
}

Qt::CheckState Source::getInfoCgps(QString &tooltipText) const
{
    return testSingle(tooltipText, expectedCgps, actualCgps);
}

Qt::CheckState Source::getInfoSgps(QString &tooltipText) const
{
    return testSingle(tooltipText, expectedSgps, actualSgps);
}

Qt::CheckState Source::getInfoAro(QString &tooltipText) const
{
    return testSet(tooltipText, expectedAro, actualAro);
}

Qt::CheckState Source::getInfoThor(QString &tooltipText) const
{
    return testSet(tooltipText, expectedThor, actualThor);
}

Qt::CheckState Source::getInfoSedigism(QString &tooltipText) const
{
    return testSet(tooltipText, expectedSedigism, actualSedigism);
}

Qt::CheckState Source::getInfoFugin(QString &tooltipText) const
{
    return testSet(tooltipText, expectedFugin, actualFugin);
}
