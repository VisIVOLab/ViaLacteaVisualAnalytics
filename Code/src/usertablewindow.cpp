#include "usertablewindow.h"
#include "ui_usertablewindow.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include "vialacteainitialquery.h"

UserTableWindow::UserTableWindow(QString filepath, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UserTableWindow),
    filepath(filepath)
{
    ui->setupUi(this);
    ui->sourcesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->imagesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->cubesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    changeSelectionMode(ui->selectionComboBox->currentText());
    setAttribute(Qt::WA_DeleteOnClose);

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
        qDebug() << "UserTableWindow.readFile" << "Error: could not open file" << filepath;
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
        checkBox->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
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
    auto coordsList = new QList<QPair<double, double>>();

    for (int i = 0; i < ui->sourcesTable->rowCount(); ++i) {
        auto item = ui->sourcesTable->item(i, 0);
        if (item->checkState() == Qt::Checked) {
            double lon = ui->sourcesTable->item(i, 2)->text().toDouble();
            double lat = ui->sourcesTable->item(i, 3)->text().toDouble();
            coordsList->append(QPair<double, double>(lon, lat));
        }
    }

    if (!coordsList->isEmpty()) {
        images.clear();
        cubes.clear();
        set.clear();
        ui->imagesTable->setRowCount(0);
        ui->cubesTable->setRowCount(0);
        query(coordsList);
    } else {
        delete coordsList;
    }
}

void UserTableWindow::query(QList<QPair<double, double>> *coordsList)
{
    auto vlkb = new VialacteaInitialQuery;
    connect(vlkb, &VialacteaInitialQuery::searchDone, this, [this, vlkb, coordsList](QList<QMap<QString, QString>> results){
        parseResults(results);

        if (!coordsList->isEmpty()) {
            query(coordsList);
        } else {
            delete coordsList;
            updateTables();
            ui->tabWidget->setCurrentIndex(1);
        }
        vlkb->deleteLater();
    });

    auto coords = coordsList->takeFirst();
    if (ui->selectionComboBox->currentText() == "Point")
        vlkb->searchRequest(coords.first, coords.second, ui->radiusLineEdit->text().toDouble());
    else
        vlkb->searchRequest(coords.first, coords.second, ui->dlLineEdit->text().toDouble(), ui->dbLineEdit->text().toDouble());
}

void UserTableWindow::parseResults(const QList<QMap<QString, QString>> &results)
{
    foreach (const auto &item, results) {
        QString x = item.value("Survey") + item.value("Species") + item.value("Transition") + \
                item.value("longitudeP1") + item.value("longitudeP2") + item.value("longitudeP3") + item.value("longitudeP4") + \
                item.value("latitudeP1") + item.value("latitudeP2") + item.value("latitudeP3") + item.value("latitudeP4");

        if (!set.contains(x)) {
            auto &targetList = (item.value("Species").compare("Continuum") == 0) ? images : cubes;
            targetList.append(item);
            set.insert(x);
        }
    }
}

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
    auto overlapText = [](QString desc){
        return desc.isEmpty() ? "Merge" : desc;
    };

    foreach (const auto &image, images) {
        if (filters.isEmpty() || filters.contains(image.value("Transition"))) {
            auto description = image.value("Description");
            auto survey = new QTableWidgetItem(image.value("Survey"));
            auto species = new QTableWidgetItem(image.value("Species"));
            auto transition = new QTableWidgetItem(image.value("Transition"));
            auto overlap = new QTableWidgetItem(overlapText(image.value("overlapDescription")));
            survey->setToolTip(description);
            species->setToolTip(description);
            transition->setToolTip(description);
            overlap->setToolTip(description);

            ui->imagesTable->insertRow(ui->imagesTable->rowCount());
            ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 0, survey);
            ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 1, species);
            ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 2, transition);
            ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 3, overlap);
        }
    }

    foreach (const auto &cube, cubes) {
        auto description = cube.value("Description");
        auto survey = new QTableWidgetItem(cube.value("Survey"));
        auto species = new QTableWidgetItem(cube.value("Species"));
        auto transition = new QTableWidgetItem(cube.value("Transition"));
        auto overlap = new QTableWidgetItem(overlapText(cube.value("overlapDescription")));
        survey->setToolTip(description);
        species->setToolTip(description);
        transition->setToolTip(description);
        overlap->setToolTip(description);

        ui->cubesTable->insertRow(ui->cubesTable->rowCount());
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 0, survey);
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 1, species);
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 2, transition);
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 3, overlap);
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

void UserTableWindow::on_glimpse_24_checkBox_clicked()
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

