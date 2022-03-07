#include "mcutoutsummary.h"
#include "ui_mcutoutsummary.h"

#include "loadingwidget.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>

MCutoutSummary::MCutoutSummary(QWidget *parent, const QStringList &cutouts)
    : QWidget(parent, Qt::Window),
      ui(new Ui::MCutoutSummary),
      mcutoutEndpoint("http://vlkb.neanias.eu:8080/vlkb-datasetstestmcutout/vlkb_mcutout")
{
    ui->setupUi(this);
    ui->textWait->hide();
    ui->progressBar->hide();
    setAttribute(Qt::WA_DeleteOnClose);

    nam = new QNetworkAccessManager(this);

    createRequestBody(cutouts);
    initSummaryTable();
}

MCutoutSummary::~MCutoutSummary()
{
    delete ui;
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

    QJsonArray body;

    foreach (auto &&cutout, cutouts) {
        QUrl url(cutout);
        auto queryItems = QUrlQuery(url).queryItems();
        auto params = listToMap(queryItems);

        QJsonObject obj;
        obj["pubdid"] = params["pubdid"];

        QJsonObject coord;
        coord["lon_deg"] = params["l"].toDouble();
        coord["lat_deg"] = params["b"].toDouble();
        if (params.contains("vl"))
            coord["vl_kmps"] = params["vl"].toDouble();
        if (params.contains("vu"))
            coord["vu_kmps"] = params["vu"].toDouble();
        if (params.contains("r")) {
            coord["dlon_deg"] = params["r"].toDouble();
            coord["dlat_deg"] = params["r"].toDouble();
        } else {
            coord["dlon_deg"] = params["dl"].toDouble();
            coord["dlat_deg"] = params["db"].toDouble();
        }
        obj["coord"] = coord;

        QJsonObject subsurvey;
        subsurvey["survey_name"] = params["surveyname"];
        subsurvey["species"] = params["species"];
        subsurvey["transition"] = params["transition"];
        // obj["subsrv"] = subsurvey;

        body.append(obj);
    }

    requestBody = body;
}

void MCutoutSummary::initSummaryTable()
{
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

void MCutoutSummary::sendRequest()
{
    QNetworkRequest req(mcutoutEndpoint);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument body(requestBody);
    auto res = nam->post(req, body.toJson());
    connect(res, &QNetworkReply::finished, this, [this, res]() -> void {
        if (res->error() == QNetworkReply::NoError) {
            parseXmlResponse(res);
        } else {
            QMessageBox::critical(this, tr("Error"), res->errorString());
        }

        ui->textWait->hide();
        ui->progressBar->hide();
        res->deleteLater();
    });

    ui->textWait->show();
    ui->progressBar->show();
}

void MCutoutSummary::downloadArchive(const QString &absolutePath)
{
    auto loading = new LoadingWidget;
    loading->setText("Download archive...");
    loading->show();

    QNetworkRequest req(downloadUrl);
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
    /// \todo Cleanup
    auto parseURL = [this](QXmlStreamReader &xml) {
        xml.readNext();
        downloadUrl = QUrl(xml.text().trimmed().toString());
        qDebug() << "URL" << downloadUrl;
    };

    auto parseItem = [this](QXmlStreamReader &xml) {
        while (!xml.isEndElement() || xml.name() != "ITEM") {
            xml.readNext();

            if (!xml.isStartElement()) {
                continue;
            }

            if (xml.name() == "INDEX") {
                xml.readNext();
                qDebug() << "INDEX" << xml.text().trimmed();
            }

            if (xml.name() == "CONTENT_CODE") {
                xml.readNext();
                qDebug() << "CONTENT_CODE" << xml.text().trimmed();
            }

            if (xml.name() == "CONTENT") {
                xml.readNext();
                qDebug() << "CONTENT" << xml.text().trimmed();
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

    responseContents.clear();
    QXmlStreamReader xml(body);
    while (!xml.atEnd()) {
        xml.readNext();

        if (!xml.isStartElement()) {
            continue;
        }

        if (xml.isStartElement() && xml.name() == "URL") {
            parseURL(xml);
        }

        if (xml.isStartElement() && xml.name() == "URL_CONTENT") {
            parseURLContent(xml);
        }
    }

    if (xml.hasError()) {
        qDebug() << xml.errorString();
    }

    for (int i = 0; i < responseContents.size(); ++i) {
        QString text =
                responseContents.at(i).contains("vlkb_cutout") ? "SUCCESS" : responseContents.at(i);
        ui->tableSummary->item(i, 1)->setText(text);
    }

    ui->textWait->show();
    ui->textWait->setText("The archive is ready, click confirm to download it.");
}

void MCutoutSummary::on_tableSummary_itemClicked(QTableWidgetItem *item)
{
    int row = ui->tableSummary->row(item);
    auto obj = requestBody.at(row).toObject();
    auto coord = obj["coord"].toObject();
    auto subsrv = obj["subsrv"].toObject();

    ui->textPubDID->setText(obj["pubdid"].toString());
    ui->textSurvey->setText(subsrv["survey_name"].toString());
    ui->textSpecies->setText(subsrv["species"].toString());
    ui->textTransition->setText(subsrv["transition"].toString());
    ui->textLon->setText(QString::number(coord["lon_deg"].toDouble(), 'f', 4));
    ui->textLat->setText(QString::number(coord["lat_deg"].toDouble(), 'f', 4));
    ui->textDLon->setText(QString::number(coord["dlon_deg"].toDouble(), 'f', 4));
    ui->textDLat->setText(QString::number(coord["dlat_deg"].toDouble(), 'f', 4));
    ui->textStatus->setText(ui->tableSummary->item(row, 1)->text());
}

void MCutoutSummary::on_btnSendRequest_clicked()
{
    if (downloadUrl.isEmpty()) {
        sendRequest();
    } else {
        QFileInfo info(downloadUrl.toString());
        qDebug() << info.fileName();
        QString path = QFileDialog::getSaveFileName(this, "Save archive",
                                                    QDir::home().absoluteFilePath(info.fileName()));
        if (path.isEmpty()) {
            return;
        }

        downloadArchive(path);
    }
}
