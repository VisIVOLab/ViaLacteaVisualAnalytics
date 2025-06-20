#include "usertablewindow.h"
#include "ui_usertablewindow.h"

#include "authwrapper.h"
#include "mcutoutsummary.h"
#include "vialacteainitialquery.h"
#include "VLKBInventoryTree.h"

#include <QCheckBox>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextStream>
#include <QUrlQuery>

UserTableWindow::UserTableWindow(const QString &filepath, const QString &settingsFile,
                                 QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::UserTableWindow),
      filepath(filepath),
      settings(settingsFile, QSettings::IniFormat)
{
    ui->setupUi(this);
    ui->btnDownload->hide();
    setAttribute(Qt::WA_DeleteOnClose);

    ui->tabWidget->setTabEnabled(1, false);
    ui->tabWidget->setTabEnabled(2, false);

    ui->sourcesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->imagesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->cubesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    changeSelectionMode(ui->selectionComboBox->currentText());

    readFile();
    getSurveysData();
}

UserTableWindow::~UserTableWindow()
{
    delete ui;
}

void UserTableWindow::getSurveysData()
{
    QUrlQuery postData;
    postData.addQueryItem("REQUEST", "doQuery");
    postData.addQueryItem("VERSION", "1.0");
    postData.addQueryItem("LANG", "ADQL");
    postData.addQueryItem("FORMAT", "tsv");
    postData.addQueryItem("QUERY",
                          "SELECT name, description, species, transition FROM datasets.surveys");
    QByteArray postDataEncoded = postData.toString(QUrl::FullyEncoded).toUtf8();

    QString tapUrl = settings.value("vlkbtableurl").toString();
    QNetworkRequest req(tapUrl + "/sync");
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    IA2VlkbAuth::Instance().putAccessToken(req);

    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    connect(nam, &QNetworkAccessManager::finished, this, [this, nam](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QString response = reply->readAll();
            QStringList surveysData = response.split("\n");
            surveysData.removeFirst(); // Remove column names line
            buildUI(surveysData);
        } else {
            QMessageBox::critical(this, "Error", reply->errorString());
        }
        reply->deleteLater();
        nam->deleteLater();
    });

    nam->post(req, postDataEncoded);
}

void UserTableWindow::buildUI(const QStringList &surveysData)
{
    foreach (QString survey, surveysData) {
        if (survey.isEmpty())
            continue;

        QStringList surveyInfo = survey.replace(QString("\""), QString()).split('\t');
        QString name = surveyInfo[0], description = surveyInfo[1], species = surveyInfo[2],
                transition = surveyInfo[3];

        if (name == "ExtMaps") {
            // Exclude extinction maps
            continue;
        }

        auto destMap = (species.contains("Continuum") ? &surveys2d : &surveys3d);

        if (destMap->contains(name)) {
            Survey *s = destMap->value(name);
            s->addSpecies(species);
            s->addTransition(transition);
        } else {
            Survey *s = new Survey(name, description, species, transition, this);
            destMap->insert(name, s);
        }
    }

    if (surveys2d.isEmpty() && surveys3d.isEmpty()) {
        QMessageBox::information(this, "No surveys", "No surveys found");
        return;
    }

    buildUI(surveys2d, selectedSurveys2d, ui->imagesTable, ui->imagesScrollAreaContents);
    buildUI(surveys3d, selectedSurveys3d, ui->cubesTable, ui->cubesScrollAreaContents);

    ui->queryButton->setEnabled(true);
}

void UserTableWindow::buildUI(const QMap<QString, Survey *> &surveys,
                              QMultiMap<QString, QPair<QString, QString>> &selectedSurveys,
                              QTableWidget *table, QWidget *scrollArea)
{
    foreach (const auto &survey, surveys) {
        auto name = survey->getName();
        table->insertColumn(table->columnCount());
        auto columnHeader = new QTableWidgetItem(name);
        columnHeader->setToolTip(survey->getDescription());
        table->setHorizontalHeaderItem(table->columnCount() - 1, columnHeader);

        auto groupBox = new QGroupBox(survey->getName(), scrollArea);
        auto layout = new QVBoxLayout(groupBox);
        for (int i = 0; i < survey->count(); ++i) {
            auto species = survey->getSpecies().at(i);
            auto transition = survey->getTransitions().at(i);

            QPair<QString, QString> pair(species, transition);
            selectedSurveys.insert(name, pair);

            auto box = new QCheckBox(groupBox);
            box->setText(QString("%1 - %2").arg(species, transition));
            box->setChecked(true);

            connect(box, &QCheckBox::clicked, this, [&selectedSurveys, name, pair](bool checked) {
                if (checked) {
                    selectedSurveys.insert(name, pair);
                } else {
                    selectedSurveys.remove(name, pair);
                }
            });
            layout->addWidget(box);
        }

        groupBox->setLayout(layout);
        scrollArea->layout()->addWidget(groupBox);
    }
}

void UserTableWindow::readFile()
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
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
    bool pointMode = selectionMode.compare("Point") == 0;
    ui->radiusLabel->setVisible(pointMode);
    ui->radiusLineEdit->setVisible(pointMode);
    ui->dbLabel->setVisible(!pointMode);
    ui->dbLineEdit->setVisible(!pointMode);
    ui->dlLabel->setVisible(!pointMode);
    ui->dlLineEdit->setVisible(!pointMode);
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

        ui->sourcesTable->setItem(row, 0, checkBox);
        for (int col = 1; col < tableColumns.count(); ++col) {
            ui->sourcesTable->setItem(row, col, new QTableWidgetItem(source[tableColumns[col]]));
        }
    }
}

void UserTableWindow::on_queryButton_clicked()
{
    // Clean up the previous search
    qDeleteAll(selectedSources);
    selectedSources.clear();
    ui->imagesTable->setRowCount(0);
    ui->cubesTable->setRowCount(0);

    // Add 1 to skip the checkbox column in the table
    int designationCol = ui->idBox->currentIndex() + 1;
    int glonCol = ui->glonBox->currentIndex() + 1;
    int glatCol = ui->glatBox->currentIndex() + 1;

    for (int i = 0; i < ui->sourcesTable->rowCount(); ++i) {
        if (ui->sourcesTable->item(i, 0)->checkState() == Qt::Checked) {
            QString designation = ui->sourcesTable->item(i, designationCol)->text();
            double glon = ui->sourcesTable->item(i, glonCol)->text().toDouble();
            double glat = ui->sourcesTable->item(i, glatCol)->text().toDouble();
            selectedSources.append(new SourceCutouts(designation, glon, glat, this));
        }
    }

    if (!selectedSources.isEmpty()) {
        query();
    }
}

void UserTableWindow::query(int index)
{
    SourceCutouts *source = selectedSources.at(index);

    auto vlkb = new VialacteaInitialQuery();

    QString pos;
    if (ui->selectionComboBox->currentText() == "Point") {
        vlkb->searchRequest(source->getGlon(), source->getGlat(),
                            ui->radiusLineEdit->text().toDouble());
        pos = VialacteaInitialQuery::posCutoutString(source->getGlon(), source->getGlat(),
                                                     ui->radiusLineEdit->text().toDouble());
    } else {
        double lon1 = source->getGlon() - ui->dlLineEdit->text().toDouble();
        double lon2 = source->getGlon() + ui->dlLineEdit->text().toDouble();
        double lat1 = source->getGlat() - ui->dbLineEdit->text().toDouble();
        double lat2 = source->getGlat() + ui->dbLineEdit->text().toDouble();
        pos = VialacteaInitialQuery::posCutoutString(lon1, lon2, lat1, lat2);

        vlkb->searchRequest(source->getGlon(), source->getGlat(), ui->dlLineEdit->text().toDouble(),
                            ui->dbLineEdit->text().toDouble());
    }
    connect(vlkb, &VialacteaInitialQuery::searchDoneVO, this,
            [this, vlkb, source, index, pos](const QByteArray &votable) {
                auto tree = new VLKBInventoryTree(votable, pos);
                auto cutouts = tree->getList();
                tree->deleteLater();

                source->parseSearchResults(cutouts);

                int next = index + 1;
                if (next < selectedSources.count()) {
                    query(next);
                } else {
                    updateTables();
                    ui->tabWidget->setTabEnabled(1, true);
                    ui->tabWidget->setTabEnabled(2, true);
                    ui->tabWidget->setCurrentIndex(1);
                }
                vlkb->deleteLater();
            });
}

void UserTableWindow::updateTables()
{
    ui->imagesTable->setRowCount(0);
    ui->cubesTable->setRowCount(0);

    const auto NewSurveyCell = [this](const QString &surveyName, const SourceCutouts *source,
                                      bool is3D) {
        Survey *survey = (is3D ? surveys3d.value(surveyName) : surveys2d.value(surveyName));

        QString tooltip;
        Qt::CheckState state = source->surveyInfo(tooltip, survey, is3D);

        Qt::GlobalColor color =
                (state == Qt::Checked ? Qt::green
                                      : (state == Qt::Unchecked ? Qt::red : Qt::yellow));

        auto tmp = new QTableWidgetItem();
        tmp->setToolTip(tooltip);
        tmp->setBackground(color);
        return tmp;
    };

    foreach (const auto &source, selectedSources) {
        // Images Table
        ui->imagesTable->insertRow(ui->imagesTable->rowCount());
        int rowI = ui->imagesTable->rowCount() - 1;

        QTableWidgetItem *x = new QTableWidgetItem();
        x->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        x->setCheckState(Qt::Unchecked);
        ui->imagesTable->setItem(rowI, 0, x);

        ui->imagesTable->setItem(rowI, 1, new QTableWidgetItem(source->getDesignation()));
        ui->imagesTable->setItem(rowI, 2, new QTableWidgetItem(QString::number(source->getGlon())));
        ui->imagesTable->setItem(rowI, 3, new QTableWidgetItem(QString::number(source->getGlat())));
        ui->imagesTable->setItem(rowI, 4,
                                 new QTableWidgetItem(QString::number(source->getImagesCount())));

        for (int i = 5; i < ui->imagesTable->columnCount(); ++i) {
            auto surveyName = ui->imagesTable->horizontalHeaderItem(i)->text();
            ui->imagesTable->setItem(rowI, i, NewSurveyCell(surveyName, source, false));
        }

        // Cubes table
        ui->cubesTable->insertRow(ui->cubesTable->rowCount());
        int rowC = ui->cubesTable->rowCount() - 1;

        x = new QTableWidgetItem();
        x->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        x->setCheckState(Qt::Unchecked);
        ui->cubesTable->setItem(rowC, 0, x);

        ui->cubesTable->setItem(rowC, 1, new QTableWidgetItem(source->getDesignation()));
        ui->cubesTable->setItem(rowC, 2, new QTableWidgetItem(QString::number(source->getGlon())));
        ui->cubesTable->setItem(rowC, 3, new QTableWidgetItem(QString::number(source->getGlat())));
        ui->cubesTable->setItem(rowC, 4,
                                new QTableWidgetItem(QString::number(source->getCubesCount())));

        for (int i = 5; i < ui->cubesTable->columnCount(); ++i) {
            auto survey = ui->cubesTable->horizontalHeaderItem(i)->text();
            ui->cubesTable->setItem(rowC, i, NewSurveyCell(survey, source, true));
        }
    }
}

QList<Cutout> UserTableWindow::getCutoutsList(int t)
{
    // t = 0 -> images; t = 1 -> cubes
    const QTableWidget *table = (t == 0) ? ui->imagesTable : ui->cubesTable;
    const auto &surveys = (t == 0) ? selectedSurveys2d : selectedSurveys3d;

    QList<Cutout> cutouts;

    for (int row = 0; row < table->rowCount(); ++row) {
        if (table->item(row, 0)->checkState() == Qt::Checked) {
            const SourceCutouts *source = selectedSources.at(row);

            for (auto it = surveys.constBegin(); it != surveys.constEnd(); ++it) {
                auto survey = it.key();
                auto species = it.value().first;
                auto transition = it.value().second;

                Cutout c;
                if (source->getBestCutout(survey, species, transition, c)) {
                    cutouts << c;
                }
            }
        }
    }

    return cutouts;
}

void UserTableWindow::initMCutoutRequest()
{
    QList<Cutout> cutouts;
    cutouts << getCutoutsList(0) << getCutoutsList(1);

    if (cutouts.isEmpty()) {
        // Nothing to do
        return;
    }

    auto mcutoutWin = new MCutoutSummary(this, cutouts);
    mcutoutWin->show();
    mcutoutWin->activateWindow();
    mcutoutWin->raise();
}

void UserTableWindow::on_selectionComboBox_activated(const QString &arg1)
{
    changeSelectionMode(arg1);
}

void UserTableWindow::on_tabWidget_currentChanged(int index)
{
    ui->btnDownload->setVisible(index != 0);
}

void UserTableWindow::on_btnSendRequest_clicked()
{
    initMCutoutRequest();
}

// ----------------------------------------------------------------------------------
SourceCutouts::SourceCutouts(const QString &designation, double glon, double glat, QObject *parent)
    : QObject(parent), designation(designation), glon(glon), glat(glat)
{
}

const QString &SourceCutouts::getDesignation() const
{
    return designation;
}

double SourceCutouts::getGlon() const
{
    return glon;
}

double SourceCutouts::getGlat() const
{
    return glat;
}

bool SourceCutouts::getBestCutout(const QString &survey, const QString &species,
                                  const QString &transition, Cutout &cutout) const
{
    Cutout current;
    const auto &container = species.contains("Continuum") ? images : cubes;

    foreach (const auto &cutout, container) {
        auto _survey = cutout.survey;
        auto _species = cutout.species;
        auto _transition = cutout.transition;

        if (survey != _survey || species != _species || transition != _transition) {
            continue;
        }

        if (current.ivoID.isEmpty() || cutout.overlap == 3) {
            current = cutout;
        }
    }

    if (current.ivoID.isEmpty()) {
        return false;
    }

    cutout = current;
    return true;
}

void SourceCutouts::parseSearchResults(const QList<Cutout> &results)
{
    foreach (const auto &cutout, results) {
        auto survey = cutout.survey;
        auto species = cutout.species;
        auto transition = cutout.transition;

        if (cutout.overlap == 0) {
            // Skip merge entries
            continue;
        }

        if (cutout.type == "image") {
            images.append(cutout);

            if (!transitions.contains(survey)) {
                transitions.insert(survey, new QSet<QString>({ transition }));
            } else {
                transitions.value(survey)->insert(transition);
            }

        } else {
            cubes.append(cutout);

            if (!this->species.contains(survey)) {
                this->species.insert(survey, new QSet<QString>({ species }));
            } else {
                this->species.value(survey)->insert(species);
            }
        }
    }
}

const QList<Cutout> &SourceCutouts::getImages() const
{
    return images;
}

int SourceCutouts::getImagesCount() const
{
    return images.count();
}

const QList<Cutout> &SourceCutouts::getCubes() const
{
    return cubes;
}

int SourceCutouts::getCubesCount() const
{
    return cubes.count();
}

Qt::CheckState SourceCutouts::surveyInfo(QString &tooltipText, const Survey *survey,
                                         bool is3D) const
{
    QSet<QString> expected;
    QSet<QString> *actual;

    if (is3D) {
        expected =
                QSet<QString>(survey->getSpecies().constBegin(), survey->getSpecies().constEnd());
        actual = species.value(survey->getName());
    } else {
        expected = QSet<QString>(survey->getTransitions().constBegin(),
                                 survey->getTransitions().constEnd());
        actual = transitions.value(survey->getName());
    }

    if (!actual) {
        return Qt::Unchecked;
    }

    if (*actual == expected) {
        return Qt::Checked;
    }

    tooltipText.clear();
    foreach (QString el, expected) {
        tooltipText += el + " : " + (actual->contains(el) ? "Yes" : "No") + "\n";
    }
    tooltipText.chop(1);

    return Qt::PartiallyChecked;
}

// --------------------------------------
Survey::Survey(const QString &name, const QString &desc, const QString &species,
               const QString &transition, QObject *parent)
    : QObject(parent), name(name), description(desc), species(species), transitions(transition)
{
}

const QString &Survey::getName() const
{
    return name;
}

const QString &Survey::getDescription() const
{
    return description;
}

const QStringList &Survey::getSpecies() const
{
    return species;
}

const QStringList &Survey::getTransitions() const
{
    return transitions;
}

void Survey::addSpecies(const QString &s)
{
    species.append(s);
}

void Survey::addTransition(const QString &t)
{
    transitions.append(t);
}

int Survey::count() const
{
    return transitions.count();
}
