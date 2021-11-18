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
    ui->imagesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->cubesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
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

    QStringList imagesCols;
    for (int i = 0; i < ui->imagesTable->columnCount(); ++i) {
        imagesCols << ui->imagesTable->horizontalHeaderItem(i)->text();
    }
    foreach (const auto &s, surveys2d) {
        imagesCols << s->getName();

        auto groupBox = new QGroupBox(s->getName(), ui->imagesScrollAreaContents);
        auto layout = new QVBoxLayout(groupBox);

        for (int i = 0; i < s->getTransitions().count(); ++i) {
            auto box = new QCheckBox(groupBox);

            auto species = s->getSpecies().at(i);
            auto transition = s->getTransitions().at(i);
            box->setText(QString("%1 - %2").arg(species, transition));

            connect(box, &QCheckBox::clicked, this, [this, s, species, transition](bool checked) {
                QPair<QString, QString> pair(species, transition);
                if (checked) {
                    filters.insert(s->getName(), pair);
                } else {
                    filters.remove(s->getName(), pair);
                }
            });
            layout->addWidget(box);
        }

        groupBox->setLayout(layout);
        ui->imagesScrollAreaContents->layout()->addWidget(groupBox);
    }

    QStringList cubesCols;
    for (int i = 0; i < ui->cubesTable->columnCount(); ++i) {
        cubesCols << ui->cubesTable->horizontalHeaderItem(i)->text();
    }
    foreach (const auto &s, surveys3d) {
        cubesCols << s->getName();

        auto groupBox = new QGroupBox(s->getName(), ui->cubesScrollAreaContents);
        auto layout = new QVBoxLayout(groupBox);

        for (int i = 0; i < s->getSpecies().count(); ++i) {
            auto box = new QCheckBox(groupBox);

            auto species = s->getSpecies().at(i);
            auto transition = s->getTransitions().at(i);
            box->setText(QString("%1 - %2").arg(species, transition));

            connect(box, &QCheckBox::clicked, this, [this, s, species, transition](bool checked) {
                QPair<QString, QString> pair(species, transition);
                if (checked) {
                    filters.insert(s->getName(), pair);
                } else {
                    filters.remove(s->getName(), pair);
                }
            });
            layout->addWidget(box);
        }

        groupBox->setLayout(layout);
        ui->cubesScrollAreaContents->layout()->addWidget(groupBox);
    }

    ui->imagesTable->setColumnCount(imagesCols.size());
    ui->imagesTable->setHorizontalHeaderLabels(imagesCols);
    ui->cubesTable->setColumnCount(cubesCols.size());
    ui->cubesTable->setHorizontalHeaderLabels(cubesCols);
    ui->queryButton->setEnabled(true);
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

    const auto NewSurveyCheckBox =
        [this](const QString &surveyName, const Source *source, bool is3D) {
            Survey *survey = (is3D ? surveys3d.value(surveyName) : surveys2d.value(surveyName));

            QString tooltip;
            Qt::CheckState state = source->surveyInfo(tooltip, survey, is3D);

            auto tmp = new QTableWidgetItem();
            tmp->setFlags(Qt::ItemIsUserTristate | Qt::ItemIsEnabled | Qt::ItemIsEditable
                          | Qt::ItemIsSelectable);
            tmp->setCheckState(state);
            tmp->setToolTip(tooltip);
            return tmp;
        };

    foreach (const auto &source, selectedSources) {
        // Images Table
        ui->imagesTable->insertRow(ui->imagesTable->rowCount());
        QTableWidgetItem *x = new QTableWidgetItem();
        x->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        x->setCheckState(Qt::Unchecked);
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1, 0, x);
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1,
                                 1,
                                 new QTableWidgetItem(source->getDesignation()));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1,
                                 2,
                                 new QTableWidgetItem(QString::number(source->getGlon())));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1,
                                 3,
                                 new QTableWidgetItem(QString::number(source->getGlat())));
        ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1,
                                 4,
                                 new QTableWidgetItem(QString::number(source->getImages().count())));

        for (int i = 5; i < ui->imagesTable->columnCount(); ++i) {
            auto survey = ui->imagesTable->horizontalHeaderItem(i)->text();
            ui->imagesTable->setItem(ui->imagesTable->rowCount() - 1,
                                     i,
                                     NewSurveyCheckBox(survey, source, false));
        }

        // Cubes table
        ui->cubesTable->insertRow(ui->cubesTable->rowCount());
        x = new QTableWidgetItem();
        x->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        x->setCheckState(Qt::Unchecked);
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1, 0, x);
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1,
                                1,
                                new QTableWidgetItem(source->getDesignation()));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1,
                                2,
                                new QTableWidgetItem(QString::number(source->getGlon())));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1,
                                3,
                                new QTableWidgetItem(QString::number(source->getGlat())));
        ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1,
                                4,
                                new QTableWidgetItem(QString::number(source->getCubes().count())));

        for (int i = 5; i < ui->cubesTable->columnCount(); ++i) {
            auto survey = ui->cubesTable->horizontalHeaderItem(i)->text();
            ui->cubesTable->setItem(ui->cubesTable->rowCount() - 1,
                                    i,
                                    NewSurveyCheckBox(survey, source, true));
        }
    }
}

//void UserTableWindow::setFilter(const QString &survey, const QString &transition, bool on)
//{
//    if (on) {
//        filters2d.insert(QPair<QString, QString>(survey, transition));
//    } else {
//        filters2d.remove(QPair<QString, QString>(survey, transition));
//    }

//    qDebug() << "Filters" << filters2d;
//}

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
                QPair<QString, QString> pair(image.value("Species"), image.value("Transition"));
                if (filters.contains(image.value("Survey"), pair)) {
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

    QString tmp = QFileDialog::getExistingDirectory(this,
                                                    "Select a folder",
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly);
    if (!tmp.isEmpty()) {
        downloadImages(cutouts, QDir(tmp));
    }
}

void UserTableWindow::on_downloadCubesButton_clicked()
{
    QMap<QString, QStringList> cubes;
    for (int row = 0; row < ui->cubesTable->rowCount(); ++row) {
        QTableWidgetItem *item = ui->cubesTable->item(row, 0);
        if (item->checkState() == Qt::Checked) {
            const Source *source = selectedSources.at(row);
            QStringList downloadLinks;

            foreach (const auto cube, source->getCubes()) {
                QPair<QString, QString> pair(cube.value("Species"), cube.value("Transition"));
                if (filters.contains(cube.value("Survey"), pair)) {
                    downloadLinks << cube.value("URL");
                }
            }

            if (!downloadLinks.isEmpty()) {
                cubes.insert(source->getDesignation(), downloadLinks);
            }
        }
    }

    if (cubes.isEmpty()) {
        // Nothing to do
        return;
    }

    QString tmp = QFileDialog::getExistingDirectory(this,
                                                    "Select a folder",
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly);
    if (!tmp.isEmpty()) {
        downloadImages(cubes, QDir(tmp));
    }
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

const QList<QMap<QString, QString>> &Source::getCubes() const
{
    return cubes;
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

    tooltipText.clear();
    foreach (QString el, expected) {
        tooltipText += el + " : " + (actual->contains(el) ? "Yes" : "No") + "\n";
    }
    tooltipText.chop(1);

    if (*actual == expected) {
        return Qt::Checked;
    }

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
