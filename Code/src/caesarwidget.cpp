#include "caesarwidget.h"
#include "ui_caesarwidget.h"

#include <QJsonDocument>
#include <QMessageBox>
#include <QFileDialog>
#include "loadingwidget.h"

CaesarWidget::CaesarWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CaesarWidget)
{
    ui->setupUi(this);
    ui->dataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->dataTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->jobsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->jobsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->jobsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    nam = new QNetworkAccessManager(this);
    auth = &NeaniasCaesarAuth::Instance();

    emit ui->dataRefreshButton->clicked();
    emit ui->jobRefreshButton->clicked();
}

CaesarWidget::~CaesarWidget()
{
    delete ui;
}

QString CaesarWidget::baseUrl()
{
    return QString("http://caesar-api.dev.neanias.eu/caesar/api/v1.0");
}

void CaesarWidget::updateDataTable(const QJsonArray &files)
{
    ui->dataTable->clearContents();
    ui->dataTable->setRowCount(0);

    foreach (const auto &it, files) {
        const auto file = it.toObject();
        auto id = file["fileid"].toString();
        auto name = file["filename_orig"].toString();
        auto tag = file["tag"].toString();
        auto date = file["filedate"].toString();

        ui->dataTable->insertRow(ui->dataTable->rowCount());
        ui->dataTable->setItem(ui->dataTable->rowCount()-1, 0, new QTableWidgetItem(id));
        ui->dataTable->setItem(ui->dataTable->rowCount()-1, 1, new QTableWidgetItem(name));
        ui->dataTable->setItem(ui->dataTable->rowCount()-1, 2, new QTableWidgetItem(tag));
        ui->dataTable->setItem(ui->dataTable->rowCount()-1, 3, new QTableWidgetItem(date));
    }
}

void CaesarWidget::updateJobsTable(const QJsonArray &jobs)
{
    ui->jobsTable->clearContents();
    ui->jobsTable->setRowCount(0);

    foreach (const auto &it, jobs) {
        const auto job = it.toObject();
        auto id = job["job_id"].toString();
        auto tag = job["tag"].toString();
        auto date = job["submit_date"].toString();
        auto status = job["state"].toString();
        auto elapsed_time = QString::number(job["elapsed_time"].toInt());

        ui->jobsTable->insertRow(ui->jobsTable->rowCount());
        ui->jobsTable->setItem(ui->jobsTable->rowCount()-1, 0, new QTableWidgetItem(id));
        ui->jobsTable->setItem(ui->jobsTable->rowCount()-1, 1, new QTableWidgetItem(tag));
        ui->jobsTable->setItem(ui->jobsTable->rowCount()-1, 2, new QTableWidgetItem(date));
        ui->jobsTable->setItem(ui->jobsTable->rowCount()-1, 3, new QTableWidgetItem(status));
        ui->jobsTable->setItem(ui->jobsTable->rowCount()-1, 4, new QTableWidgetItem(elapsed_time));
    }
}

bool CaesarWidget::selectedItemId(const QTableWidget *table, QString &id)
{
    auto items = table->selectedItems();
    if (items.count() < 1) {
        return false;
    }
    id = items[0]->text();
    return true;
}

bool CaesarWidget::selectedItemName(QString &name)
{
    auto items = ui->dataTable->selectedItems();
    if (items.count() < 1) {
        return false;
    }
    name = items[1]->text();
    return true;
}

void CaesarWidget::on_dataRefreshButton_clicked()
{
    ui->dataRefreshButton->setEnabled(false);

    auto url = baseUrl() + "/fileids";
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    auto reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, [this, reply](){
        if (reply->error() == QNetworkReply::NoError) {
            auto res = QJsonDocument::fromJson(reply->readAll()).array();
            updateDataTable(res);
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "CaesarWidget.on_dataRefreshButton_clicked.error" << statusCode << reply->errorString();
            QMessageBox::critical(this, tr("Error"), tr(qPrintable(reply->errorString())));
        }
        reply->deleteLater();

        ui->dataRefreshButton->setEnabled(true);
    });
}

void CaesarWidget::on_dataUploadButton_clicked()
{
    QString fn = QFileDialog::getOpenFileName(this,tr("Upload file"), QDir::homePath(), tr("Images (*.png *.jpg *.jpeg *.gif *.fits)"));

    if (fn.isEmpty())
        return;

    auto multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    auto file = new QFile(fn);
    if (!file->open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr(qPrintable(file->errorString())));
        return;
    }

    file->setParent(multipart);
    QHttpPart filePart;
    //filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/fits"));
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"" + QFileInfo(fn).fileName() + "\""));
    filePart.setBodyDevice(file);
    multipart->append(filePart);

    auto loadingWidget = new LoadingWidget;
    loadingWidget->setFileName(fn);
    loadingWidget->show();
    loadingWidget->activateWindow();

    ui->dataUploadButton->setEnabled(false);

    auto url = baseUrl() + "/upload";
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    auto reply = nam->post(req, multipart);
    multipart->setParent(reply);
    loadingWidget->setLoadingProcess(reply);

    connect(reply, &QNetworkReply::uploadProgress, loadingWidget, &LoadingWidget::updateProgressBar);
    connect(reply, &QNetworkReply::finished, [this, reply, loadingWidget](){
        if (reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(loadingWidget, tr(""), tr("File uploaded."));
            emit ui->dataRefreshButton->clicked();
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "CaesarWidget.on_dataUploadButton_clicked.error" << statusCode << reply->errorString();
            QMessageBox::critical(loadingWidget, tr("Error"), tr(qPrintable(reply->errorString())));
        }
        reply->deleteLater();
        loadingWidget->deleteLater();

        ui->dataUploadButton->setEnabled(true);
    });
}

void CaesarWidget::on_dataDownloadButton_clicked()
{
    QString fileId, srcName;
    if (!selectedItemId(ui->dataTable, fileId) || !selectedItemName(srcName)) {
        QMessageBox::information(this, tr(""), tr("No file selected."));
        return;
    }

    auto dstPath = QDir::homePath().append(QDir::separator()).append(srcName);
    auto filename = QFileDialog::getSaveFileName(this, tr("Save file as..."), dstPath);
    if (filename.isEmpty())
        return;

    auto loadingWidget = new LoadingWidget;
    loadingWidget->setFileName(filename);
    loadingWidget->show();
    loadingWidget->activateWindow();

    ui->dataDownloadButton->setEnabled(false);

    auto url = baseUrl() + "/download/" + fileId;
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    auto reply = nam->get(req);
    loadingWidget->setLoadingProcess(reply);

    connect(reply, &QNetworkReply::downloadProgress, loadingWidget, &LoadingWidget::updateProgressBar);
    connect(reply, &QNetworkReply::finished, [this, reply, fileId, filename, loadingWidget](){
        if (reply->error() == QNetworkReply::NoError) {
            QFile file(filename);
            if (!file.open(QIODevice::WriteOnly)) {
                QMessageBox::critical(loadingWidget, tr("Error"), tr(qPrintable(file.errorString())));
            } else {
                file.write(reply->readAll());
                file.close();

                QMessageBox::information(loadingWidget, tr(""), tr("File downloaded."));
            }
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "CaesarWidget.on_dataDownloadButton_clicked.error" << statusCode << reply->errorString();
            QMessageBox::critical(loadingWidget, tr("Error"), tr(qPrintable(reply->errorString())));
        }
        reply->deleteLater();
        loadingWidget->deleteLater();

        ui->dataDownloadButton->setEnabled(true);
    });
}

void CaesarWidget::on_dataDeleteButton_clicked()
{
    QString fileId;
    if (!selectedItemId(ui->dataTable, fileId)) {
        QMessageBox::information(this, tr(""), tr("No file selected."));
        return;
    }

    int ret = QMessageBox::question(this, tr("Confirm"),
                                   tr("Do you want to delete the selected file?"),
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    if (ret == QMessageBox::No)
        return;

    ui->dataDeleteButton->setEnabled(false);

    auto url = baseUrl() + "/delete/" + fileId;
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    auto reply = nam->post(req, QByteArray());
    connect(reply, &QNetworkReply::finished, [this, reply](){
        if (reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(this, tr(""), tr("File deleted."));
            emit ui->dataRefreshButton->clicked();
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "CaesarWidget.on_dataDeleteButton_clicked.error" << statusCode << reply->errorString();
            QMessageBox::critical(this, tr("Error"), tr(qPrintable(reply->errorString())));
        }
        reply->deleteLater();

        ui->dataDeleteButton->setEnabled(true);
    });
}

void CaesarWidget::on_jobRefreshButton_clicked()
{
    ui->jobRefreshButton->setEnabled(false);

    auto url = baseUrl() + "/jobs";
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    auto reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, [this, reply](){
        if (reply->error() == QNetworkReply::NoError) {
            auto res = QJsonDocument::fromJson(reply->readAll()).array();
            updateJobsTable(res);
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "CaesarWidget.on_jobRefreshButton_clicked.error" << statusCode << reply->errorString();
            QMessageBox::critical(this, tr("Error"), tr(qPrintable(reply->errorString())));
        }
        reply->deleteLater();

        ui->jobRefreshButton->setEnabled(true);
    });
}


void CaesarWidget::on_jobDownloadButton_clicked()
{
    QString jobId;
    if (!selectedItemId(ui->jobsTable, jobId)) {
        QMessageBox::information(this, tr(""), tr("No job selected."));
        return;
    }

    auto dstPath = QDir::homePath().append(QDir::separator()).append(jobId).append(".tar.gz");
    auto filename = QFileDialog::getSaveFileName(this, tr("Save file as..."), dstPath);
    if (filename.isEmpty())
        return;

    auto loadingWidget = new LoadingWidget;
    loadingWidget->setFileName(filename);
    loadingWidget->show();
    loadingWidget->activateWindow();

    ui->jobDownloadButton->setEnabled(false);

    auto url = baseUrl() + "/job/" + jobId + "/output";
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    auto reply = nam->get(req);
    loadingWidget->setLoadingProcess(reply);

    connect(reply, &QNetworkReply::downloadProgress, loadingWidget, &LoadingWidget::updateProgressBar);
    connect(reply, &QNetworkReply::finished, [this, reply, jobId, filename, loadingWidget](){
        if (reply->error() == QNetworkReply::NoError) {
            QFile file(filename);
            if (!file.open(QIODevice::WriteOnly)) {
                QMessageBox::critical(loadingWidget, tr("Error"), tr(qPrintable(file.errorString())));
            } else {
                file.write(reply->readAll());
                file.close();

                QMessageBox::information(loadingWidget, tr(""), tr("Job output downloaded."));
            }
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "CaesarWidget.on_jobDownloadButton_clicked.error" << statusCode << reply->errorString();
            QMessageBox::critical(loadingWidget, tr("Error"), tr(qPrintable(reply->errorString())));
        }
        reply->deleteLater();
        loadingWidget->deleteLater();

        ui->jobDownloadButton->setEnabled(true);
    });
}

void CaesarWidget::on_jobCancelButton_clicked()
{
    QString jobId;
    if (!selectedItemId(ui->jobsTable, jobId)) {
        QMessageBox::information(this, tr(""), tr("No job selected."));
        return;
    }

    int ret = QMessageBox::question(this, tr("Confirm"),
                                   tr("Do you want to cancel the selected job?"),
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    if (ret == QMessageBox::No)
        return;

    ui->jobCancelButton->setEnabled(false);

    auto url = baseUrl() + "/job/" + jobId + "/cancel";
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    auto reply = nam->post(req, QByteArray());
    connect(reply, &QNetworkReply::finished, [this, reply](){
        auto response = reply->readAll();
        auto status = QJsonDocument::fromJson(response).object()["status"].toString();
        if (reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(this, tr(""), tr(qPrintable(status)));
            emit ui->dataRefreshButton->clicked();
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "CaesarWidget.on_dataDeleteButton_clicked.error" << statusCode << reply->errorString();
            QMessageBox::critical(this, tr("Error"), tr(qPrintable(status)));
        }
        reply->deleteLater();

        ui->jobCancelButton->setEnabled(true);
    });
}
