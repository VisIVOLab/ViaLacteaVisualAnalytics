#include "mcutoutsummary.h"
#include "ui_mcutoutsummary.h"

#include "authwrapper.h"
#include "loadingwidget.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>

MCutoutSummary::MCutoutSummary(QWidget *parent)
    : QWidget(parent, Qt::Window),
      ui(new Ui::MCutoutSummary),
      settings(QDir::homePath().append("/VisIVODesktopTemp/setting.ini"), QSettings::NativeFormat),
      pollTimeout(2000)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    connect(this, &MCutoutSummary::jobSubmitted, this, &MCutoutSummary::pollJob);
    connect(this, &MCutoutSummary::jobCompleted, this, &MCutoutSummary::getJobReport);

    nam = new QNetworkAccessManager(this);
    mcutoutEndpoint = settings.value("vlkburl", "").toString().append("/uws_mcutout/mcutout");
    qDebug() << "MCutout endpoint" << mcutoutEndpoint;
}

MCutoutSummary::MCutoutSummary(QWidget *parent, const QStringList &cutouts) : MCutoutSummary(parent)
{
    ui->textWait->hide();
    ui->progressBar->hide();

    this->cutouts = cutouts;
    this->pendingFile = QDir::homePath().append("/VisIVODesktopTemp/pending_mcutouts.dat");

    createRequestBody(cutouts);
    initSummaryTable();
}

MCutoutSummary::MCutoutSummary(QWidget *parent, const QString &pendingFile) : MCutoutSummary(parent)
{
    ui->btnSendRequest->setEnabled(false);

    this->pendingFile = pendingFile;

    // Here we get the jobId that was being polled when the user closed
    // the application
    QFile file(pendingFile);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, "Error", file.errorString());
        return;
    }

    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_15);
    QString jobId;
    stream >> jobId >> cutouts;
    file.close();

    createRequestBody(cutouts);
    initSummaryTable();
    pollJob(jobId);
}

MCutoutSummary::~MCutoutSummary()
{
    delete ui;
}

void MCutoutSummary::startJob(const QString &jobId)
{
    QString url = QString("%1/%2/phase").arg(mcutoutEndpoint, jobId);
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("PHASE", "RUN");
    QByteArray body = urlQuery.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    auto auth = settings.value("vlkbtype", "ia2") == "ia2" ? &IA2VlkbAuth::Instance()
                                                           : &NeaniasVlkbAuth::Instance();
    auth->putAccessToken(req);

    auto reply = nam->post(req, body);
    connect(reply, &QNetworkReply::finished, this, [this, reply, jobId]() {
        auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if (statusCode == 303) {
            pollJob(jobId);
        } else {
            QMessageBox::critical(this, tr("Error"), reply->errorString());
        }

        reply->deleteLater();
    });
}

void MCutoutSummary::pollJob(const QString &jobId)
{
    qDebug() << Q_FUNC_INFO << "Started job" << jobId;
    auto timer = new QTimer(this);
    timer->setInterval(pollTimeout);
    connect(timer, &QTimer::timeout, this, [this, timer, jobId]() {
        QString url = QString("%1/%2/phase").arg(mcutoutEndpoint, jobId);
        QNetworkRequest req(url);
        auto auth = settings.value("vlkbtype", "ia2") == "ia2" ? &IA2VlkbAuth::Instance()
                                                               : &NeaniasVlkbAuth::Instance();
        auth->putAccessToken(req);
        auto reply = nam->get(req);

        connect(reply, &QNetworkReply::finished, this, [this, timer, jobId, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QString phase = QString(reply->readAll()).trimmed();

                if (phase == "COMPLETED") {
                    timer->deleteLater();
                    emit jobCompleted(jobId);
                } else if (phase == "HELD") {
                    timer->deleteLater();
                    // Need to restart the job manually
                    startJob(jobId);
                } else if (phase == "ABORTED") {
                    timer->deleteLater();
                    /// \todo
                } else if (phase == "ERROR") {
                    timer->deleteLater();
                    /// \todo
                }
            } else {
                timer->stop();
                timer->deleteLater();
                QMessageBox::critical(this, tr("Error"), reply->errorString());
                ui->btnSendRequest->setEnabled(true);
                ui->textWait->hide();
                ui->progressBar->hide();
            }

            reply->deleteLater();
        });
    });
    timer->start();
}

void MCutoutSummary::getJobReport(const QString &jobId)
{
    QString url = QString("%1/%2/results/Report").arg(mcutoutEndpoint, jobId);
    QNetworkRequest req(url);
    auto auth = settings.value("vlkbtype", "ia2") == "ia2" ? &IA2VlkbAuth::Instance()
                                                           : &NeaniasVlkbAuth::Instance();
    auth->putAccessToken(req);
    auto reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            parseXmlResponse(reply);
        } else {
            QMessageBox::critical(this, tr("Error"), reply->errorString());
            ui->btnSendRequest->setEnabled(true);
            ui->textWait->hide();
            ui->progressBar->hide();
        }

        reply->deleteLater();
    });
}

void MCutoutSummary::createRequestBody(const QStringList &cutouts)
{
    auto listToMap = [](const QList<QPair<QString, QString>> &list) -> QMap<QString, QString> {
        QMap<QString, QString> map;
        foreach (auto &&el, list) {
            map.insert(el.first, el.second);
        }
        return map;
    };

    foreach (auto &&cutout, cutouts) {
        QUrl url(cutout);
        auto queryItems = QUrlQuery(url).queryItems();
        auto params = listToMap(queryItems);

        QJsonObject obj;
        obj["pubdid"] = params["pubdid"];

        QJsonObject coord;
        coord["l"] = params["l"].toDouble();
        coord["b"] = params["b"].toDouble();
        if (params.contains("vl"))
            coord["vl"] = params["vl"].toDouble();
        if (params.contains("vu"))
            coord["vu"] = params["vu"].toDouble();
        if (params.contains("r")) {
            coord["r"] = params["r"].toDouble();
        } else {
            coord["dl"] = params["dl"].toDouble();
            coord["db"] = params["db"].toDouble();
        }
        obj["coord"] = coord;

        requestBody.append(obj);
    }
}

void MCutoutSummary::initSummaryTable()
{
    QString title = QString("%1 (%2 cutouts)")
                            .arg(ui->tableGroupBox->title(), QString::number(requestBody.count()));
    ui->tableGroupBox->setTitle(title);

    ui->tableSummary->setRowCount(0);
    foreach (auto &&el, requestBody) {
        auto obj = el.toObject();
        int row = ui->tableSummary->rowCount();
        ui->tableSummary->insertRow(row);
        ui->tableSummary->setItem(row, 0, new QTableWidgetItem(obj["pubdid"].toString()));
        ui->tableSummary->setItem(row, 1, new QTableWidgetItem(" - "));
    }
    ui->tableSummary->resizeColumnsToContents();
}

void MCutoutSummary::submitJob()
{
    QJsonDocument body(requestBody);
    QHttpPart mcutout;
    mcutout.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    mcutout.setHeader(QNetworkRequest::ContentDispositionHeader,
                      QVariant("form-data; name=\"mcutout\"; filename=\"mcutout\""));
    mcutout.setBody(body.toJson());

    auto multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    multipart->append(mcutout);

    QUrl url(mcutoutEndpoint);
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("PHASE", "RUN");
    url.setQuery(urlQuery);

    QNetworkRequest req(url);
    auto auth = settings.value("vlkbtype", "ia2") == "ia2" ? &IA2VlkbAuth::Instance()
                                                           : &NeaniasVlkbAuth::Instance();
    auth->putAccessToken(req);
    auto reply = nam->post(req, multipart);
    multipart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if (statusCode == 303) {
            QString location = reply->header(QNetworkRequest::LocationHeader).toString();
            QString jobId = location.split('/').last();
            saveStatus(jobId);
            emit jobSubmitted(jobId);
        } else {
            QMessageBox::critical(this, tr("Error"), reply->errorString());
            ui->btnSendRequest->setEnabled(true);
            ui->textWait->hide();
            ui->progressBar->hide();
        }

        reply->deleteLater();
    });
}

void MCutoutSummary::downloadArchive(const QString &absolutePath)
{
    auto loading = new LoadingWidget;
    loading->setText("Download archive...");
    loading->show();

    QNetworkRequest req(downloadUrl);
    auto auth = settings.value("vlkbtype", "ia2") == "ia2" ? &IA2VlkbAuth::Instance()
                                                           : &NeaniasVlkbAuth::Instance();
    auth->putAccessToken(req);
    auto reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, absolutePath, reply, loading]() {
        if (reply->error() == QNetworkReply::NoError) {
            QFile outputFile(absolutePath);
            if (!outputFile.open(QIODevice::WriteOnly)) {
                QMessageBox::critical(this, tr("Error"), outputFile.errorString());
            } else {
                outputFile.write(reply->readAll());
                outputFile.close();
                QMessageBox::information(this, tr("Success"), "Download completed.");
            }
        } else {
            QMessageBox::critical(this, tr("Error"), reply->errorString());
        }

        reply->deleteLater();
        loading->deleteLater();
    });
    connect(reply, &QNetworkReply::downloadProgress, loading, &LoadingWidget::updateProgressBar);
    loading->setLoadingProcess(reply);
}

void MCutoutSummary::parseXmlResponse(QNetworkReply *body)
{
    // Start sub-parsers
    auto parseURL = [this](QXmlStreamReader &xml) {
        xml.readNext();
        downloadUrl = QUrl(xml.text().trimmed().toString());
    };

    auto parseItem = [this](QXmlStreamReader &xml) {
        while (!xml.isEndElement() || xml.name() != "ITEM") {
            xml.readNext();

            if (!xml.isStartElement()) {
                continue;
            }

            if (xml.name() == "CONTENT") {
                xml.readNext();
                responseContents << xml.text().trimmed().toString();
            }
        }
    };

    auto parseURLContent = [parseItem](QXmlStreamReader &xml) {
        while (!xml.isEndElement() || xml.name() != "URL_CONTENT") {
            xml.readNext();

            if (!xml.isStartElement()) {
                continue;
            }

            if (xml.name() == "ITEM") {
                parseItem(xml);
            }
        }
    };
    // End sub-parsers
    responseContents.clear();
    QXmlStreamReader xml(body);
    while (!xml.atEnd()) {
        xml.readNext();

        if (!xml.isStartElement()) {
            continue;
        }

        if (xml.name() == "URL") {
            parseURL(xml);
        }

        if (xml.name() == "URL_CONTENT") {
            parseURLContent(xml);
        }
    }

    if (xml.hasError()) {
        qDebug() << Q_FUNC_INFO << xml.errorString();
        QMessageBox::critical(this, tr("Error"), xml.errorString());
    }

    for (int i = 0; i < responseContents.size(); ++i) {
        QString text =
                responseContents.at(i).contains("vlkb_cutout") ? "Success" : responseContents.at(i);
        ui->tableSummary->item(i, 1)->setText(text);
    }

    ui->progressBar->hide();
    ui->textWait->setText("You can now download the archive.");
    ui->btnSendRequest->setText("Download");
    ui->btnSendRequest->setEnabled(true);
}

void MCutoutSummary::on_tableSummary_itemClicked(QTableWidgetItem *item)
{
    int row = ui->tableSummary->row(item);
    auto obj = requestBody.at(row).toObject();
    auto coord = obj["coord"].toObject();

    ui->textPubDID->setText(obj["pubdid"].toString());
    ui->textLon->setText(QString::number(coord["l"].toDouble(), 'f', 4));
    ui->textLat->setText(QString::number(coord["b"].toDouble(), 'f', 4));
    ui->textRadius->setText(QString::number(coord["r"].toDouble(), 'f', 4));
    ui->textDLon->setText(QString::number(coord["dl"].toDouble(), 'f', 4));
    ui->textDLat->setText(QString::number(coord["dl"].toDouble(), 'f', 4));
    ui->textStatus->setText(ui->tableSummary->item(row, 1)->text());
}

void MCutoutSummary::on_btnSendRequest_clicked()
{
    if (downloadUrl.isEmpty()) {
        submitJob();
        ui->btnSendRequest->setEnabled(false);
        ui->textWait->show();
        ui->progressBar->show();
    } else {
        QFileInfo info(downloadUrl.toString());
        QString path = QFileDialog::getSaveFileName(this, "Save archive",
                                                    QDir::home().absoluteFilePath(info.fileName()),
                                                    "Archive (*.tar.gz)");
        if (path.isEmpty()) {
            return;
        }

        downloadArchive(path);
        if (QFileInfo::exists(pendingFile)) {
            QFile::remove(pendingFile);
        }
    }
}

void MCutoutSummary::saveStatus(const QString &jobId)
{
    // Here we save the jobId and the cutouts requested to a binary file so that it can be polled
    // again if the user closes the application before the job is completed
    QFile file(pendingFile);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::warning(this, "Error", file.errorString());
        return;
    }

    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_15);
    stream << jobId << cutouts;

    file.close();
}
