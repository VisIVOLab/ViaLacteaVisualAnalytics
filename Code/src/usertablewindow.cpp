#include "usertablewindow.h"
#include "ui_usertablewindow.h"

#include "authwrapper.h"
#include "singleton.h"
#include "vialacteainitialquery.h"
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextStream>
#include <QUrlQuery>

UserTableWindow::UserTableWindow(const QString &filepath,
                                 const QString &settingsFile,
                                 QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::UserTableWindow)
    , filepath(filepath)
    , settings(settingsFile, QSettings::NativeFormat)
{
    ui->setupUi(this);
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
    QString vlkbType = settings.value("vlkbtype", "public").toString();

    QString table;
    if (vlkbType == "public") {
        table = "vlkb_datasets_public.surveys";
    } else if (vlkbType == "private") {
        table = "vlkb_radiocubes.surveys";
    } else /* neanias */ {
        table = "datasets.surveys";
    }

    QUrlQuery postData;
    postData.addQueryItem("REQUEST", "doQuery");
    postData.addQueryItem("VERSION", "1.0");
    postData.addQueryItem("LANG", "ADQL");
    postData.addQueryItem("FORMAT", "tsv");
    postData.addQueryItem("QUERY", "SELECT name, species, transition FROM " + table);
    QByteArray postDataEncoded = postData.toString(QUrl::FullyEncoded).toUtf8();

    QString tapUrl = settings.value("vlkbtableurl").toString();
    QNetworkRequest req(tapUrl + "/sync");
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    Singleton<AuthWrapper>::Instance().putAccessToken(req);

    QNetworkAccessManager *nam = new QNetworkAccessManager(this);

    connect(nam,
            &QNetworkAccessManager::authenticationRequired,
            this,
            [this](QNetworkReply *reply, QAuthenticator *authenticator) {
                Q_UNUSED(reply);
                authenticator->setUser(settings.value("vlkbuser", "").toString());
                authenticator->setPassword(settings.value("vlkbpass", "").toString());
            });

    connect(nam, &QNetworkAccessManager::finished, this, [this, nam](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QString response = reply->readAll();
            QStringList surveysData = response.split("\n");
            surveysData.removeFirst(); // Remove column names line
            buildUI(surveysData);
        } else {
            qDebug() << "UserTableWindow.getSurveysData.Error" << reply->errorString();
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
        QString name = surveyInfo[0], species = surveyInfo[1], transition = surveyInfo[2];

        auto destMap = (species.contains("Continuum") ? &surveys2d : &surveys3d);

        if (destMap->contains(name)) {
            Survey *s = destMap->value(name);
            s->addSpecies(species);
            s->addTransition(transition);
        } else {
            Survey *s = new Survey(name, species, transition, this);
            destMap->insert(name, s);
        }
    }

    if (surveys2d.isEmpty() && surveys3d.isEmpty()) {
        QMessageBox::information(this, "No surveys", "No surveys found");
        return;
    }

    buildUI(surveys2d, ui->imagesTable, ui->imagesScrollAreaContents);
    buildUI(surveys3d, ui->cubesTable, ui->cubesScrollAreaContents);

    ui->queryButton->setEnabled(true);
}

void UserTableWindow::buildUI(const QMap<QString, Survey *> &surveys,
                              QTableWidget *table,
                              QWidget *scrollArea)
{
    QStringList cols;
    for (int i = 0; i < table->columnCount(); ++i) {
        cols << table->horizontalHeaderItem(i)->text();
    }

    foreach (const auto &survey, surveys) {
        auto name = survey->getName();
        cols << name;

        auto groupBox = new QGroupBox(survey->getName(), scrollArea);
        auto layout = new QVBoxLayout(groupBox);

        for (int i = 0; i < survey->count(); ++i) {
            auto box = new QCheckBox(groupBox);
            auto species = survey->getSpecies().at(i);
            auto transition = survey->getTransitions().at(i);
            box->setText(QString("%1 - %2").arg(species, transition));

            connect(box, &QCheckBox::clicked, this, [this, name, species, transition](bool checked) {
                QPair<QString, QString> pair(species, transition);
                if (checked) {
                    filters.insert(name, pair);
                } else {
                    filters.remove(name, pair);
                }
            });
            layout->addWidget(box);
        }

        groupBox->setLayout(layout);
        scrollArea->layout()->addWidget(groupBox);
    }

    table->setColumnCount(cols.size());
    table->setHorizontalHeaderLabels(cols);
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
            selectedSources.append(new Source(designation, glon, glat, this));
        }
    }

    if (!selectedSources.isEmpty()) {
        query();
    }
}

void UserTableWindow::query(int index)
{
    Source *source = selectedSources.at(index);

    auto vlkb = new VialacteaInitialQuery();
    connect(vlkb,
            &VialacteaInitialQuery::searchDone,
            this,
            [this, vlkb, source, index](QList<QMap<QString, QString>> results) {
                source->parseSearchResults(results);

                if ((index + 1) < selectedSources.count()) {
                    query(index + 1);
                } else {
                    updateTables();
                    ui->tabWidget->setTabEnabled(1, true);
                    ui->tabWidget->setTabEnabled(2, true);
                    ui->tabWidget->setCurrentIndex(1);
                }
                vlkb->deleteLater();
            });

    if (ui->selectionComboBox->currentText() == "Point") {
        vlkb->searchRequest(source->getGlon(),
                            source->getGlat(),
                            ui->radiusLineEdit->text().toDouble());
    } else {
        vlkb->searchRequest(source->getGlon(),
                            source->getGlat(),
                            ui->dlLineEdit->text().toDouble(),
                            ui->dbLineEdit->text().toDouble());
    }
}

void UserTableWindow::updateTables()
{
    ui->imagesTable->setRowCount(0);
    ui->cubesTable->setRowCount(0);

    const auto NewSurveyCell = [this](const QString &surveyName, const Source *source, bool is3D) {
        Survey *survey = (is3D ? surveys3d.value(surveyName) : surveys2d.value(surveyName));

        QString tooltip;
        Qt::CheckState state = source->surveyInfo(tooltip, survey, is3D);

        Qt::GlobalColor color = (state == Qt::Checked
                                     ? Qt::green
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
        ui->imagesTable->setItem(rowI,
                                 4,
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
        ui->cubesTable->setItem(rowC,
                                4,
                                new QTableWidgetItem(QString::number(source->getCubesCount())));

        for (int i = 5; i < ui->cubesTable->columnCount(); ++i) {
            auto survey = ui->cubesTable->horizontalHeaderItem(i)->text();
            ui->cubesTable->setItem(rowC, i, NewSurveyCell(survey, source, true));
        }
    }
}

void UserTableWindow::downloadCutouts(int t)
{
    // t = 0 -> images; t = 1 -> cubes
    const QTableWidget *table = (t == 0) ? ui->imagesTable : ui->cubesTable;
    QMap<QString, QStringList> cutouts;

    for (int row = 0; row < table->rowCount(); ++row) {
        if (table->item(row, 0)->checkState() == Qt::Checked) {
            const Source *source = selectedSources.at(row);
            QStringList downloadLinks;

            const auto &container = (t == 0) ? source->getImages() : source->getCubes();
            foreach (const auto &cutout, container) {
                QPair<QString, QString> pair(cutout.value("Species"), cutout.value("Transition"));
                if (filters.contains(cutout.value("Survey"), pair)) {
                    downloadLinks << cutout.value("URL");
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

    QString tmp = QFileDialog::getExistingDirectory(this,
                                                    "Select a folder",
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly);

    if (!tmp.isEmpty()) {
        auto vlkb = new VialacteaInitialQuery();
        QDir dir(tmp);

        for (auto i = cutouts.constBegin(); i != cutouts.constEnd(); ++i) {
            dir.mkdir(i.key());
            foreach (const auto &link, i.value()) {
                vlkb->cutoutRequest(link, QDir(dir.absoluteFilePath(i.key())));
            }
        }
    }
}

void UserTableWindow::on_downloadImagesButton_clicked()
{
    downloadCutouts(0);
}

void UserTableWindow::on_downloadCubesButton_clicked()
{
    downloadCutouts(1);
}

void UserTableWindow::on_selectionComboBox_activated(const QString &arg1)
{
    changeSelectionMode(arg1);
}

// ----------------------------------------------------------------------------------
Source::Source(const QString &designation, double glon, double glat, QObject *parent)
    : QObject(parent)
    , designation(designation)
    , glon(glon)
    , glat(glat)
{}

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
    foreach (const auto &item, results) {
        auto survey = item.value("Survey");
        auto species = item.value("Species");
        auto transition = item.value("Transition");

        if (species.contains("Continuum")) {
            images.append(item);

            if (!transitions.contains(survey)) {
                transitions.insert(survey, new QSet<QString>({transition}));
            } else {
                transitions.value(survey)->insert(transition);
            }

        } else {
            cubes.append(item);

            if (!this->species.contains(survey)) {
                this->species.insert(survey, new QSet<QString>({species}));
            } else {
                this->species.value(survey)->insert(species);
            }
        }
    }
}

const QList<QMap<QString, QString>> &Source::getImages() const
{
    return images;
}

int Source::getImagesCount() const
{
    return images.count();
}

const QList<QMap<QString, QString>> &Source::getCubes() const
{
    return cubes;
}

int Source::getCubesCount() const
{
    return cubes.count();
}

Qt::CheckState Source::surveyInfo(QString &tooltipText, const Survey *survey, bool is3D) const
{
    QSet<QString> expected;
    QSet<QString> *actual;

    if (is3D) {
        expected = QSet<QString>(survey->getSpecies().constBegin(), survey->getSpecies().constEnd());
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
Survey::Survey(const QString &name,
               const QString &species,
               const QString &transition,
               QObject *parent)
    : QObject(parent)
    , name(name)
    , species(species)
    , transitions(transition)
{}

const QString &Survey::getName() const
{
    return name;
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
