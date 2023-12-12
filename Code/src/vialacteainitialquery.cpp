#include "vialacteainitialquery.h"
#include "ui_vialacteainitialquery.h"

#include <QAuthenticator>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QSettings>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>

#include "astroutils.h"
#include "authkeys.h"
#include "authwrapper.h"
#include "downloadmanager.h"
#include "mainwindow.h"
#include "singleton.h"
#include "vtkwindowcube.h"
#include "vialactea.h"

VialacteaInitialQuery::VialacteaInitialQuery(QString fn, QWidget *parent)
    : QWidget(parent), ui(new Ui::VialacteaInitialQuery)
{
    ui->setupUi(this);

    ui->rectGroupBox->hide();

    QSettings settings(QDir::homePath()
                       .append(QDir::separator())
                       .append("VisIVODesktopTemp")
                       .append(QDir::separator())
                       .append("setting.ini"),
                       QSettings::IniFormat);
    vlkbUrl = settings.value("vlkburl", "").toString();

    nam = new QNetworkAccessManager(this);
    QObject::connect(nam, SIGNAL(finished(QNetworkReply *)), this,
                     SLOT(finishedSlot(QNetworkReply *)));
    QObject::connect(nam, &QNetworkAccessManager::authenticationRequired, this,
                     &VialacteaInitialQuery::onAuthenticationRequired);

    vlkbtype = settings.value("vlkbtype", "ia2").toString();
    parser = new xmlparser();
    loading = new LoadingWidget();
    outputFile = fn;
    species = "";
    p = parent;
    myCallingVtkWindow = 0;

    transitions.insert("PLW", "500um");
    transitions.insert("PMW", "350um");
    transitions.insert("PSW", "250um");
    transitions.insert("red", "160um");
    transitions.insert("blue", "70um");
}

VialacteaInitialQuery::~VialacteaInitialQuery()
{
    delete ui;
}

void VialacteaInitialQuery::on_pushButton_clicked()
{
    this->close();
}

void VialacteaInitialQuery::on_pointRadioButton_toggled(bool checked)
{
    if (checked) {
        ui->rectGroupBox->hide();
        ui->pointGroupBox->show();
    } else {
        ui->rectGroupBox->show();
        ui->pointGroupBox->hide();
    }
}

void VialacteaInitialQuery::setL(QString l)
{
    ui->l_lineEdit->setText(l);
}
void VialacteaInitialQuery::setB(QString b)
{
    ui->b_lineEdit->setText(b.replace(" ", ""));
}
void VialacteaInitialQuery::setR(QString r)
{
    isRadius = true;
    ui->r_lineEdit->setText(r);
}

void VialacteaInitialQuery::setDeltaRect(QString dl, QString db)
{
    isRadius = false;
    ui->dlLineEdit->setText(dl);
    ui->dbLineEdit->setText(db);
}

void VialacteaInitialQuery::setSurveyname(QString s)
{
    surveyname = s;
}

void VialacteaInitialQuery::setTransition(QString s)
{
    transition = s;
}

void VialacteaInitialQuery::onAuthenticationRequired(QNetworkReply *r, QAuthenticator *a)
{
    Q_UNUSED(r);
    QSettings settings(QDir::homePath()
                       .append(QDir::separator())
                       .append("VisIVODesktopTemp")
                       .append(QDir::separator())
                       .append("setting.ini"),
                       QSettings::IniFormat);
    if (settings.value("vlkbtype", "ia2") == "ia2") {
        a->setUser(IA2_TAP_USER);
        a->setPassword(IA2_TAP_PASS);
    }
}

void VialacteaInitialQuery::searchRequest(double l, double b, double dl, double db)
{
    QString url = QString(vlkbUrl + "/vlkb_search?l=%1&b=%2&dl=%3&db=%4&vl=-500000&vu=500000")
            .arg(l)
            .arg(b)
            .arg(dl)
            .arg(db);
    searchRequest(url);
}

void VialacteaInitialQuery::searchRequest(double l, double b, double r)
{
    QString url = QString(vlkbUrl + "/vlkb_search?l=%1&b=%2&r=%3&vl=-500000&vu=500000")
            .arg(l)
            .arg(b)
            .arg(r);
    searchRequest(url);
}

void VialacteaInitialQuery::cutoutRequest(const QString &url, const QDir &dir)
{
    QSettings settings(QDir::homePath()
                       .append(QDir::separator())
                       .append("VisIVODesktopTemp")
                       .append(QDir::separator())
                       .append("setting.ini"),
                       QSettings::IniFormat);

    loading->setText("Querying cutout service");
    loading->show();
    loading->activateWindow();

    QUrl url_enc(url);
    QUrlQuery urlQuery(url_enc);
    auto queryItems = urlQuery.queryItems();
    for (auto i = queryItems.begin(); i != queryItems.end(); ++i) {
        i->second = QUrl::toPercentEncoding(i->second);
    }
    urlQuery.setQueryItems(queryItems);
    url_enc.setQuery(urlQuery);

    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    connect(nam, &QNetworkAccessManager::authenticationRequired, this,
            &VialacteaInitialQuery::onAuthenticationRequired);

    QNetworkRequest req(url_enc);
    IA2VlkbAuth::Instance().putAccessToken(req);
    QNetworkReply *reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, nam, reply, dir]() {
        if (reply->error() == QNetworkReply::NoError) {
            QXmlStreamReader xml(reply);
            QString downloadStr;
            xmlparser parser;
            parser.parseXML(xml, downloadStr);

            if (!downloadStr.contains("NULL")) {
                QStringList list = downloadStr.trimmed().split("/");
                QString fn = list.takeLast();
                QString filePath = dir.absoluteFilePath(fn);
                list.push_back(fn.replace("+", "%2B"));
                QUrl downloadUrl = QUrl(list.join("/"));

                DownloadManager *manager = new DownloadManager();
                manager->doDownload(downloadUrl, filePath);
                connect(manager, &DownloadManager::downloadCompleted, manager,
                        &DownloadManager::deleteLater);
            } else {
                QStringList result_splitted = downloadStr.split(" ");
                QString msg = (result_splitted.length() > 1)
                        ? "Null values percentage is " + result_splitted[1] + " (greater than 95%)"
                        : "Inconsistent data (PubDID vs Region only partially overlap)";

                QMessageBox::critical(nullptr, "Error", msg);
            }
        } else {
            QMessageBox::critical(nullptr, "Error", reply->errorString());
        }

        loading->loadingEnded();
        loading->hide();
        reply->deleteLater();
        nam->deleteLater();
    });
    loading->setLoadingProcess(reply);
}

void VialacteaInitialQuery::searchRequest(const QString &url)
{
    QSettings settings(QDir::homePath()
                       .append(QDir::separator())
                       .append("VisIVODesktopTemp")
                       .append(QDir::separator())
                       .append("setting.ini"),
                       QSettings::IniFormat);
    loading->setText("Querying search service");
    loading->show();
    loading->activateWindow();

    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    connect(nam, &QNetworkAccessManager::authenticationRequired, this,
            &VialacteaInitialQuery::onAuthenticationRequired);
    connect(nam, &QNetworkAccessManager::finished, this, [this, nam](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QXmlStreamReader xml(reply);
            auto results = parser->parseXmlAndGetList(xml);
            emit searchDone(results);
        } else {
            QMessageBox::critical(nullptr, QObject::tr("Error"),
                                  QObject::tr(qPrintable(reply->errorString())));
        }

        loading->loadingEnded();
        loading->hide();
        reply->deleteLater();
        nam->deleteLater();
    });

    QNetworkRequest req(url);
    IA2VlkbAuth::Instance().putAccessToken(req);

    QNetworkReply *reply = nam->get(req);
    loading->setLoadingProcess(reply);
}

void VialacteaInitialQuery::cutoutRequest(QString url, QList<QMap<QString, QString>> el, int pos)
{
    loading->show();
    loading->setText("Querying cutout services");
    elementsOnDb = el;
    species = elementsOnDb.at(pos).value("Species");
    pubdid = elementsOnDb.at(pos).value("PublisherDID");
    surveyname = elementsOnDb.at(pos).value("Survey");
    descriptionFromDB = elementsOnDb.at(pos).value("Description");
    transition = elementsOnDb.at(pos).value("Transition");
    velfrom = elementsOnDb.at(pos).value("from");
    velto = elementsOnDb.at(pos).value("to");
    velocityUnit = elementsOnDb.at(pos).value("VelocityUnit");

    QUrl url_enc = QUrl(url);
    QList<QPair<QString, QString>> urlQuery = QUrlQuery(url_enc).queryItems();
    QList<QPair<QString, QString>>::iterator j;
    for (j = urlQuery.begin(); j != urlQuery.end(); ++j) {
        (*j).second = QUrl::toPercentEncoding((*j).second);
    }

    QUrlQuery q;
    q.setQueryItems(urlQuery);
    url_enc.setQuery(q);

    QNetworkRequest req(url_enc);

    QSettings settings(QDir::homePath()
                               .append(QDir::separator())
                               .append("VisIVODesktopTemp")
                               .append(QDir::separator())
                               .append("setting.ini"),
                       QSettings::IniFormat);

    IA2VlkbAuth::Instance().putAccessToken(req);

    QNetworkReply *reply = nam->get(req);
    loading->setLoadingProcess(reply);
}

void VialacteaInitialQuery::selectedStartingLayersRequest(QUrl url)
{
    loading->show();
    loading->setText("Querying cutout services");

    QSettings settings(QDir::homePath()
                       .append(QDir::separator())
                       .append("VisIVODesktopTemp")
                       .append(QDir::separator())
                       .append("setting.ini"),
                       QSettings::IniFormat);

    QNetworkRequest req(url);

    IA2VlkbAuth::Instance().putAccessToken(req);

    QNetworkReply *reply = nam->get(req);
    loading->setLoadingProcess(reply);
}

void VialacteaInitialQuery::on_queryPushButton_clicked()
{
    loading->show();
    loading->activateWindow();
    loading->setText("Querying cutout services");

    QString urlString =
            vlkbUrl + "/vlkb_search?l=" + ui->l_lineEdit->text() + "&b=" + ui->b_lineEdit->text();
    if (isRadius) {
        urlString += "&r=" + ui->r_lineEdit->text();
    } else
        urlString += "&dl=" + ui->dlLineEdit->text() + "&db=" + ui->dbLineEdit->text();

    urlString += "&vl=-500000&vu=500000";

    QUrl url2(urlString);
    QNetworkRequest req(url2);
    QSettings settings(QDir::homePath()
                               .append(QDir::separator())
                               .append("VisIVODesktopTemp")
                               .append(QDir::separator())
                               .append("setting.ini"),
                       QSettings::IniFormat);

    IA2VlkbAuth::Instance().putAccessToken(req);
    QNetworkReply *reply = nam->get(req);
    loading->setLoadingProcess(reply);
}

void VialacteaInitialQuery::finishedSlot(QNetworkReply *reply)
{
    QString string;
    QString file;
    QSettings settings(QDir::homePath()
                       .append(QDir::separator())
                       .append("VisIVODesktopTemp")
                       .append(QDir::separator())
                       .append("setting.ini"),
                       QSettings::IniFormat);

    if (reply->error() == QNetworkReply::NoError) {
        QXmlStreamReader xml(reply);
        QString url = reply->request().url().toString();

        if (url.contains("vlkb_cutout") || url.contains("vlkb_merge")) {
            parser->parseXML(xml, string);

            if (!string.contains("NULL")) {
                loading->setText("Datacube found");
                DownloadManager *manager = new DownloadManager();
                QString urlString = string.trimmed();

                // Encode '+' sign in filename
                QStringList list = urlString.split("/");
                QString fn = list.takeLast().replace("+", "%2B");
                list.push_back(fn);

                QUrl url3(list.join("/"));

                connect(manager, SIGNAL(downloadCompleted()), this, SLOT(onDownloadCompleted()));

                file = manager->doDownload(url3, outputFile);
                loading->loadingEnded();
                loading->hide();
                downloadedFile = file;
            } else {
                QStringList result_splitted = string.split(" ");
                QString msg = "Inconsistent data (PubDID vs Region only partially overlap)";
                if (result_splitted.length() > 1)
                    msg = "Null values percentage is " + result_splitted[1] + " (greater than 95%)";
                QMessageBox::critical(NULL, QObject::tr("Error"),
                                      QObject::tr(msg.toStdString().c_str()));

                loading->loadingEnded();
                loading->hide();
            }
        }

        // La query iniziale
        if (url.contains("vlkb_search") && !url.contains("surveyname")) {
            elementsOnDb = parser->parseXmlAndGetList(xml);
            QString best_url = "";
            int best_code = 4;
            QList<QMap<QString, QString>>::iterator j;
            for (j = elementsOnDb.begin(); j != elementsOnDb.end(); ++j) {
                if ((*j).value("Species").compare(species) == 0
                        && (*j).value("Survey").contains(surveyname)
                        && (*j).value("Transition").compare(transition) == 0
                        && (*j).value("code").toInt() < best_code) {
                    best_url = (*j).value("URL");
                    best_code = (*j).value("code").toInt();
                }
            }

            if (best_url == "") {
                // best_url not found => do not continue
                loading->loadingEnded();
                loading->hide();
                QMessageBox::information(NULL, QObject::tr("No results"),
                                         QObject::tr("The query did not produce any results,\ntry "
                                                     "again with different parameters."));
            } else {
                QUrl url(best_url);
                QList<QPair<QString, QString>> urlQuery = QUrlQuery(url).queryItems();
                QList<QPair<QString, QString>>::iterator i;
                for (i = urlQuery.begin(); i != urlQuery.end(); ++i) {
                    (*i).second = QUrl::toPercentEncoding((*i).second);
                }

                QUrlQuery q;
                q.setQueryItems(urlQuery);
                url.setQuery(q);
                QNetworkRequest req(url);
                IA2VlkbAuth::Instance().putAccessToken(req);

                QNetworkReply *res = nam->get(req);
                loading->setLoadingProcess(res);
            }
        }

        if (url.contains("vlkb_search") && url.contains("surveyname")) {
            QList<QMap<QString, QString>> elementsOnDb_tmp = parser->parseXmlAndGetList(xml);

            QString best_url = "";
            int best_code = 4;


            QList<QMap<QString, QString>>::iterator j;
            for (j = elementsOnDb_tmp.begin(); j != elementsOnDb_tmp.end(); ++j) {
                if ((*j).value("Species").compare(species) == 0
                        && (*j).value("Survey").contains(surveyname)
                        && (*j).value("Transition").compare(transition) == 0
                        && (*j).value("code").toInt() < best_code) {
                    best_url = (*j).value("URL");
                    best_code = (*j).value("code").toInt();
                }
            }

            if (best_url == "") {
                // best_url not found => do not continue
                loading->loadingEnded();
                loading->hide();
                QMessageBox::information(NULL, QObject::tr("No results"),
                                         QObject::tr("The query did not produce any results,\ntry "
                                                     "again with different parameters."));
            } else {
                QUrl url(best_url);
                QList<QPair<QString, QString>> urlQuery = QUrlQuery(url).queryItems();
                QList<QPair<QString, QString>>::iterator i;
                for (i = urlQuery.begin(); i != urlQuery.end(); ++i) {
                    (*i).second = QUrl::toPercentEncoding((*i).second);
                }

                QUrlQuery q;
                q.setQueryItems(urlQuery);
                url.setQuery(q);

                QNetworkRequest req(url);
                IA2VlkbAuth::Instance().putAccessToken(req);

                QNetworkReply *res = nam->get(req);
                loading->setLoadingProcess(res);
            }
        }
    }

    else {
        // NetworkReplyError
        loading->loadingEnded();
        loading->hide();
        QMessageBox::critical(NULL, QObject::tr("Error"),
                              QObject::tr(qPrintable(reply->errorString())));
    }
    reply->deleteLater();
}

void VialacteaInitialQuery::onDownloadCompleted()
{
    int test_flag_nanten = -1;
    if (pubdid.compare("") != 0)
        test_flag_nanten = pubdid.split("_", QString::SkipEmptyParts).last().toInt();

    QString currentPath = downloadedFile;

    this->close();

    auto vl = &Singleton<ViaLactea>::Instance();
    vl->showMinimized();

    if ((velfrom.compare("0.0") == 0 && velto.compare("0.0") == 0) || species.compare("dust") == 0
            || species.compare("Continuum") == 0
            || (surveyname.compare("NANTEN") == 0 && test_flag_nanten == 2)
            || QString::compare("cornish", surveyname, Qt::CaseInsensitive) == 0
            || QString::compare("cornish2d", surveyname, Qt::CaseInsensitive) == 0
            || QString::compare("magpis", surveyname, Qt::CaseInsensitive) == 0) {
        bool l = myCallingVtkWindow != 0;

        auto fits = vtkSmartPointer<vtkFitsReader>::New();
        fits->SetFileName(currentPath.toStdString());
        fits->setSurvey(surveyname);
        fits->setSpecies(species);
        fits->setTransition(transition);

        if (l) {
            myCallingVtkWindow->addLayerImage(fits, surveyname, species, transition);
        } else {
            auto win = new vtkwindow_new(this, fits, 0, myCallingVtkWindow);
            win->setCallingL(ui->l_lineEdit->text());
            win->setCallingB(ui->b_lineEdit->text());
            win->setCallingR(ui->r_lineEdit->text());
            win->setCallingDl(ui->dlLineEdit->text());
            win->setCallingDb(ui->dbLineEdit->text());
            win->setDbElements(elementsOnDb);
            if (selectedSurvey.length() > 1)
                win->downloadStartingLayers(selectedSurvey);

            vl->setMasterWin(win);
        }
    } else {
        //auto fits = vtkSmartPointer<vtkFitsReader>::New();
        //fits->SetFileName(currentPath.toStdString());
        //fits->is3D = true;

        myCallingVtkWindow->setSelectedCubeVelocityUnit(velocityUnit);
        //new vtkwindow_new(myCallingVtkWindow, fits, 1, myCallingVtkWindow);
        QSettings settings(QDir::homePath()
                                   .append(QDir::separator())
                                   .append("VisIVODesktopTemp")
                                   .append(QDir::separator())
                                   .append("setting.ini"),
                           QSettings::IniFormat);

        long maxSize = settings.value("downscaleSize", 1024).toInt() * 1024;
        long size = QFileInfo(currentPath).size() / 1024;
        int ScaleFactor = AstroUtils::calculateResizeFactor(size, maxSize);

        auto win = new vtkWindowCube(myCallingVtkWindow, currentPath, ScaleFactor, velocityUnit);
        win->show();
        win->raise();
        win->activateWindow();
    }
}
