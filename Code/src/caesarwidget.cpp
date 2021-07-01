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

    nam = new QNetworkAccessManager(this);
    auth = &NeaniasCaesarAuth::Instance();

    emit ui->refreshButton->clicked();
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

bool CaesarWidget::selectedItemId(QString &id)
{
    auto items = ui->dataTable->selectedItems();
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

void CaesarWidget::on_refreshButton_clicked()
{
    ui->refreshButton->setEnabled(false);

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
            qDebug() << "CaesarWidget.on_refreshButton_clicked.error" << statusCode << reply->errorString();
            QMessageBox::critical(this, tr("Error"), tr(qPrintable(reply->errorString())));
        }
        reply->deleteLater();

        ui->refreshButton->setEnabled(true);
    });
}

void CaesarWidget::on_uploadButton_clicked()
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

    ui->uploadButton->setEnabled(false);

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
            emit ui->refreshButton->clicked();
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "CaesarWidget.on_uploadButton_clicked.error" << statusCode << reply->errorString();
            QMessageBox::critical(loadingWidget, tr("Error"), tr(qPrintable(reply->errorString())));
        }
        reply->deleteLater();
        loadingWidget->deleteLater();

        ui->uploadButton->setEnabled(true);
    });
}

void CaesarWidget::on_downloadButton_clicked()
{
    QString fileId, srcName;
    if (!selectedItemId(fileId) || !selectedItemName(srcName)) {
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

    ui->downloadButton->setEnabled(false);

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
            qDebug() << "CaesarWidget.on_downloadButton_clicked.error" << statusCode << reply->errorString();
            QMessageBox::critical(loadingWidget, tr("Error"), tr(qPrintable(reply->errorString())));
        }
        reply->deleteLater();
        loadingWidget->deleteLater();

        ui->downloadButton->setEnabled(true);
    });
}

void CaesarWidget::on_deleteButton_clicked()
{
    QString fileId;
    if (!selectedItemId(fileId)) {
        QMessageBox::information(this, tr(""), tr("No file selected."));
        return;
    }

    int ret = QMessageBox::question(this, tr("Confirm"),
                                   tr("Do you want to delete the selected file?"),
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    if (ret == QMessageBox::No)
        return;

    ui->deleteButton->setEnabled(false);

    auto url = baseUrl() + "/delete/" + fileId;
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    auto reply = nam->post(req, QByteArray());
    connect(reply, &QNetworkReply::finished, [this, reply](){
        if (reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(this, tr(""), tr("File deleted."));
            emit ui->refreshButton->clicked();
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "CaesarWidget.on_deleteButton_clicked.error" << statusCode << reply->errorString();
            QMessageBox::critical(this, tr("Error"), tr(qPrintable(reply->errorString())));
        }
        reply->deleteLater();

        ui->deleteButton->setEnabled(true);
    });
}
