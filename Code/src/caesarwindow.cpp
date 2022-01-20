#include "caesarwindow.h"
#include "ui_caesarwindow.h"

#include "loadingwidget.h"

#include <QDir>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkReply>
#include <QSpinBox>

const QString CaesarWindow::baseUrl = "http://caesar-api.neanias.eu/caesar/api/v1.0";

CaesarWindow::CaesarWindow(QWidget *parent, const QString &imagePath)
    : QMainWindow(parent)
    , ui(new Ui::CaesarWindow)
    , auth(&NeaniasCaesarAuth::Instance())
    , autoRefresh(false)
    , jobRefreshPeriod(10)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    ui->jobsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    nam = new QNetworkAccessManager(this);

    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &CaesarWindow::on_jobRefreshButton_clicked);

    // Ask to upload the current file
    if (!imagePath.isEmpty()) {
        auto msg = new QMessageBox(this);
        msg->setIcon(QMessageBox::Question);
        msg->setWindowTitle("Upload image");
        msg->setText("Do you want to upload the current image?");
        msg->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msg->setDefaultButton(QMessageBox::No);
        msg->show();
        connect(msg,
                &QMessageBox::buttonClicked,
                this,
                [this, msg, imagePath](QAbstractButton *button) {
                    if (msg->standardButton(button) == QMessageBox::Yes) {
                        uploadData(imagePath);
                    }
                    msg->deleteLater();
                });
    }

    emit ui->dataRefreshButton->clicked();
    emit ui->jobRefreshButton->clicked();
    getSupportedApps();
}

CaesarWindow::~CaesarWindow()
{
    delete ui;
}

void CaesarWindow::on_jobRefreshButton_clicked()
{
    ui->jobRefreshButton->setEnabled(false);

    QString url = baseUrl + "/jobs";
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    QNetworkReply *reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            auto res = QJsonDocument::fromJson(reply->readAll()).array();
            updateJobsTable(res);
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "CAESAR GET /jobs error" << statusCode << reply->errorString();
            if (autoRefresh) {
                refreshTimer->stop();
                autoRefresh = false;
                ui->jobAutoRefreshCheckbox->setChecked(false);
            }
            QMessageBox::critical(this, tr("Error"), tr(qPrintable(reply->errorString())));
        }
        reply->deleteLater();

        ui->jobRefreshButton->setEnabled(true);
    });
}

QString CaesarWindow::formatDate(const QString &date)
{
    return QDateTime::fromString(date, Qt::ISODateWithMs).toString(Qt::DefaultLocaleShortDate);
}

QString CaesarWindow::formatElapsedTime(const int &elapsedTime)
{
    QString str;
    auto hours = elapsedTime / 3600;
    auto mins = (elapsedTime % 3600) / 60;
    auto secs = elapsedTime % 60;

    if (hours > 0) {
        str = str.append(QString("%1h ").arg(hours));
    }
    if (mins > 0) {
        str = str.append(QString("%1%2m ").arg((mins < 10 ? "0" : ""), QString::number(mins)));
    }
    str = str.append(QString("%1%2s").arg((secs < 10 ? "0" : ""), QString::number(secs)));

    return str;
}

bool CaesarWindow::selectedItemId(const QTableWidget *table, QString &id)
{
    auto items = table->selectedItems();
    if (items.count() < 1) {
        return false;
    }
    id = items[0]->text();
    return true;
}

bool CaesarWindow::selectedDataFilename(QString &filename)
{
    auto items = ui->dataTable->selectedItems();
    if (items.count() < 1) {
        return false;
    }
    filename = items[1]->text();
    return true;
}

void CaesarWindow::getSupportedApps()
{
    ui->appsComboBox->clear();

    QString url = baseUrl + "/apps";
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    QNetworkReply *reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            const auto apps = QJsonDocument::fromJson(reply->readAll()).object()["apps"].toArray();
            foreach (const auto &app, apps) {
                ui->appsComboBox->insertItem(ui->appsComboBox->count(), app.toString());
            }
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            QString status = QJsonDocument::fromJson(reply->readAll()).object()["status"].toString();
            qDebug() << "CAESAR GET /apps" << statusCode << status;
            QMessageBox::critical(this, tr("Error"), status);
        }
        reply->deleteLater();
    });
}

void CaesarWindow::updateJobsTable(const QJsonArray &jobs)
{
    ui->jobsTable->setRowCount(0);

    foreach (const auto &it, jobs) {
        auto job = it.toObject();
        auto id = job["job_id"].toString();
        auto tag = job["tag"].toString();
        auto date = formatDate(job["submit_date"].toString());
        auto status = job["state"].toString();
        auto elapsed_time = formatElapsedTime(job["elapsed_time"].toInt());

        ui->jobsTable->insertRow(ui->jobsTable->rowCount());
        ui->jobsTable->setItem(ui->jobsTable->rowCount() - 1, 0, new QTableWidgetItem(id));
        ui->jobsTable->setItem(ui->jobsTable->rowCount() - 1, 1, new QTableWidgetItem(tag));
        ui->jobsTable->setItem(ui->jobsTable->rowCount() - 1, 2, new QTableWidgetItem(date));
        ui->jobsTable->setItem(ui->jobsTable->rowCount() - 1, 3, new QTableWidgetItem(status));
        ui->jobsTable->setItem(ui->jobsTable->rowCount() - 1, 4, new QTableWidgetItem(elapsed_time));
    }
}

void CaesarWindow::updateDataTable(const QJsonArray &files)
{
    ui->dataTable->setRowCount(0);

    foreach (const auto &it, files) {
        const auto file = it.toObject();
        auto id = file["fileid"].toString();
        auto name = file["filename_orig"].toString();
        auto ext = file["fileext"].toString();
        auto tag = file["tag"].toString();
        auto date = formatDate(file["filedate"].toString());

        ui->dataTable->insertRow(ui->dataTable->rowCount());
        ui->dataTable->setItem(ui->dataTable->rowCount() - 1, 0, new QTableWidgetItem(id));
        ui->dataTable->setItem(ui->dataTable->rowCount() - 1, 1, new QTableWidgetItem(name));
        ui->dataTable->setItem(ui->dataTable->rowCount() - 1, 2, new QTableWidgetItem(ext));
        ui->dataTable->setItem(ui->dataTable->rowCount() - 1, 3, new QTableWidgetItem(tag));
        ui->dataTable->setItem(ui->dataTable->rowCount() - 1, 4, new QTableWidgetItem(date));
    }
}

void CaesarWindow::uploadData(const QString &fn)
{
    QString tag = QInputDialog::getText(this, "File tag", "Tag (optional):");

    auto file = new QFile(fn);
    if (!file->open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr(qPrintable(file->errorString())));
        return;
    }

    auto multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    file->setParent(multipart);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/fits"));
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"file\"; filename=\"" + QFileInfo(fn).fileName()
                                + "\""));
    filePart.setBodyDevice(file);

    QHttpPart tagPart;
    tagPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                      QVariant("form-data; name=\"tag\""));
    tagPart.setBody(tag.toUtf8());

    multipart->append(filePart);
    multipart->append(tagPart);

    auto loadingWidget = new LoadingWidget;
    loadingWidget->setText(QFileInfo(fn).fileName());
    loadingWidget->show();
    loadingWidget->activateWindow();

    ui->dataUploadButton->setEnabled(false);

    QString url = baseUrl + "/upload";
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    QNetworkReply *reply = nam->post(req, multipart);
    multipart->setParent(reply);
    loadingWidget->setLoadingProcess(reply);

    connect(reply, &QNetworkReply::uploadProgress, loadingWidget, &LoadingWidget::updateProgressBar);
    connect(reply, &QNetworkReply::finished, this, [this, reply, loadingWidget]() {
        if (reply->error() == QNetworkReply::NoError) {
            auto response = QJsonDocument::fromJson(reply->readAll()).object();
            auto msg = response["status"].toString() + ".\nUUID " + response["uuid"].toString();
            QMessageBox::information(loadingWidget, tr("Success"), msg);
            emit ui->dataRefreshButton->clicked();
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            QString status = QJsonDocument::fromJson(reply->readAll()).object()["status"].toString();
            qDebug() << "CAESAR POST /upload error" << statusCode << status;
            ;
            QMessageBox::critical(loadingWidget, tr("Error"), status);
        }
        reply->deleteLater();
        loadingWidget->deleteLater();

        ui->dataUploadButton->setEnabled(true);
    });
}

void CaesarWindow::buildJobForm(const QJsonObject &app)
{
    QGroupBox *box;
    QFormLayout *layout;

    foreach (const auto &input, app.keys()) {
        auto name = input;
        auto type = app[input].toObject()["type"].toString();
        auto category = app[input].toObject()["category"].toString();
        auto subcategory = app[input].toObject()["subcategory"].toString();
        auto desc = app[input].toObject()["description"].toString();
        QJsonValue default_value = app[input].toObject()["default"];

        if (boxes.contains(category)) {
            box = boxes[category];
            layout = qobject_cast<QFormLayout *>(box->layout());
        } else {
            box = new QGroupBox(ui->jobSubmissionWidget);
            box->setTitle(category);
            layout = new QFormLayout(box);
            layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
            box->setLayout(layout);
            boxes.insert(category, box);
        }

        if (subcategory != "") {
            auto subbox = box->findChild<QGroupBox *>(subcategory);
            if (subbox == nullptr) {
                subbox = new QGroupBox(box);
                subbox->setObjectName(subcategory);
                subbox->setTitle(subcategory);
                auto tmp = new QFormLayout(subbox);
                tmp->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
                subbox->setLayout(tmp);
            }
            box = subbox;
            layout = qobject_cast<QFormLayout *>(box->layout());
        }

        QWidget *widget;
        QPair<QVariant::Type, QWidget *> p;

        if (type == "str") {
            p.first = QVariant::String;
            auto tmp = new QComboBox(box);
            tmp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            auto values = app[input].toObject()["allowed_values"].toArray();
            foreach (const auto &v, values) {
                tmp->insertItem(tmp->count(), v.toString());
            }
            tmp->setCurrentText(default_value.toString());
            widget = tmp;
        } else if (type == "int") {
            p.first = QVariant::Int;
            auto min = app[input].toObject()["min"].toInt();
            auto max = app[input].toObject()["max"].toInt();
            auto tmp = new QSpinBox(box);
            tmp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            tmp->setRange(min, max);
            tmp->setValue(default_value.toInt());
            widget = tmp;
        } else if (type == "float") {
            p.first = QVariant::Double;
            auto min = app[input].toObject()["min"].toDouble();
            auto max = app[input].toObject()["max"].toDouble();
            auto tmp = new QDoubleSpinBox(box);
            tmp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            tmp->setDecimals(3);
            tmp->setRange(min, max);
            tmp->setValue(default_value.toDouble());
            widget = tmp;
        } else /*if (type == "none")*/ {
            p.first = QVariant::Bool;
            widget = new QCheckBox(box);
        }
        p.second = widget;
        inputs.insert(name, p);

        widget->setObjectName(name);
        widget->setToolTip(desc);
        layout->addRow(name, widget);
    }

    foreach (const auto &box, boxes) {
        auto sub = box->findChildren<QGroupBox *>();
        foreach (const auto &it, sub) {
            box->layout()->addWidget(it);
        }
        ui->jobSubmissionWidget->layout()->addWidget(box);
    }
}

void CaesarWindow::on_jobAutoRefreshCheckbox_clicked(bool checked)
{
    autoRefresh = checked;
    if (autoRefresh) {
        refreshTimer->start(jobRefreshPeriod * 1000);
    } else {
        refreshTimer->stop();
    }
}

void CaesarWindow::on_dataRefreshButton_clicked()
{
    ui->dataRefreshButton->setEnabled(false);

    QString url = baseUrl + "/fileids";
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    QNetworkReply *reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            auto res = QJsonDocument::fromJson(reply->readAll()).array();
            updateDataTable(res);
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "CAESAR GET /fileids error" << statusCode << reply->errorString();
            QMessageBox::critical(this, tr("Error"), tr(qPrintable(reply->errorString())));
        }
        reply->deleteLater();

        ui->dataRefreshButton->setEnabled(true);
    });
}

void CaesarWindow::on_dataDeleteButton_clicked()
{
    QString fileId;
    if (!selectedItemId(ui->dataTable, fileId)) {
        QMessageBox::information(this, tr(""), tr("No file selected."));
        return;
    }

    int ret = QMessageBox::question(this,
                                    tr("Confirm"),
                                    tr("Do you want to delete the selected file?"),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No);
    if (ret == QMessageBox::No)
        return;

    ui->dataDeleteButton->setEnabled(false);

    QString url = baseUrl + "/delete/" + fileId;
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    QNetworkReply *reply = nam->post(req, QByteArray());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(this, tr(""), tr("File deleted."));
            emit ui->dataRefreshButton->clicked();
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            QString status = QJsonDocument::fromJson(reply->readAll()).object()["status"].toString();
            qDebug() << "CAESAR POST /delete error" << statusCode << status;
            QMessageBox::critical(this, tr("Error"), status);
        }
        reply->deleteLater();

        ui->dataDeleteButton->setEnabled(true);
    });
}

void CaesarWindow::on_dataDownloadButton_clicked()
{
    QString fileId, srcName;
    if (!selectedItemId(ui->dataTable, fileId) || !selectedDataFilename(srcName)) {
        QMessageBox::information(this, tr(""), tr("No file selected."));
        return;
    }

    auto dstPath = QDir::home().absoluteFilePath(srcName);
    auto filename = QFileDialog::getSaveFileName(this, tr("Save file"), dstPath);
    if (filename.isEmpty())
        return;

    auto loadingWidget = new LoadingWidget;
    loadingWidget->setText(QFileInfo(filename).fileName());
    loadingWidget->show();
    loadingWidget->activateWindow();

    ui->dataDownloadButton->setEnabled(false);

    QString url = baseUrl + "/download/" + fileId;
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    QNetworkReply *reply = nam->get(req);
    loadingWidget->setLoadingProcess(reply);

    connect(reply,
            &QNetworkReply::downloadProgress,
            loadingWidget,
            &LoadingWidget::updateProgressBar);
    connect(reply, &QNetworkReply::finished, this, [this, reply, fileId, filename, loadingWidget]() {
        if (reply->error() == QNetworkReply::NoError) {
            QFile file(filename);
            if (!file.open(QIODevice::WriteOnly)) {
                QMessageBox::critical(loadingWidget,
                                      tr("Error"),
                                      tr(qPrintable(file.errorString())));
            } else {
                file.write(reply->readAll());
                file.close();

                QMessageBox::information(loadingWidget, tr(""), tr("File downloaded."));
            }
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            QString status = QJsonDocument::fromJson(reply->readAll()).object()["status"].toString();
            qDebug() << "CAESAR GET /download error" << statusCode << status;
            QMessageBox::critical(loadingWidget, tr("Error"), status);
        }
        reply->deleteLater();
        loadingWidget->deleteLater();

        ui->dataDownloadButton->setEnabled(true);
    });
}

void CaesarWindow::on_dataUploadButton_clicked()
{
    QString fn = QFileDialog::getOpenFileName(this,
                                              tr("Upload file"),
                                              QDir::homePath(),
                                              tr("Images (*.fits)"));
    if (!fn.isEmpty()) {
        uploadData(fn);
    }
}

void CaesarWindow::on_jobDownloadButton_clicked()
{
    QString jobId;
    if (!selectedItemId(ui->jobsTable, jobId)) {
        QMessageBox::information(this, tr(""), tr("No job selected."));
        return;
    }

    auto dstPath = QDir::home().absoluteFilePath(jobId).append(".tar.gz");
    auto filename = QFileDialog::getSaveFileName(this, tr("Save file"), dstPath);
    if (filename.isEmpty())
        return;

    auto loadingWidget = new LoadingWidget;
    loadingWidget->setText(QFileInfo(filename).fileName());
    loadingWidget->show();
    loadingWidget->activateWindow();

    ui->jobDownloadButton->setEnabled(false);

    QString url = baseUrl + "/job/" + jobId + "/output";
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    QNetworkReply *reply = nam->get(req);
    loadingWidget->setLoadingProcess(reply);

    connect(reply,
            &QNetworkReply::downloadProgress,
            loadingWidget,
            &LoadingWidget::updateProgressBar);
    connect(reply, &QNetworkReply::finished, this, [this, reply, jobId, filename, loadingWidget]() {
        if (reply->error() == QNetworkReply::NoError) {
            QFile file(filename);
            if (!file.open(QIODevice::WriteOnly)) {
                QMessageBox::critical(loadingWidget,
                                      tr("Error"),
                                      tr(qPrintable(file.errorString())));
            } else {
                file.write(reply->readAll());
                file.close();

                QMessageBox::information(loadingWidget, tr(""), tr("Job output downloaded."));
            }
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            QString status = QJsonDocument::fromJson(reply->readAll()).object()["status"].toString();
            qDebug() << "CAESAR GET /job/output error" << statusCode << status;
            QMessageBox::critical(loadingWidget, tr("Error"), status);
        }
        reply->deleteLater();
        loadingWidget->deleteLater();

        ui->jobDownloadButton->setEnabled(true);
    });
}

void CaesarWindow::on_jobCancelButton_clicked()
{
    QString jobId;
    if (!selectedItemId(ui->jobsTable, jobId)) {
        QMessageBox::information(this, tr(""), tr("No job selected."));
        return;
    }

    int ret = QMessageBox::question(this,
                                    tr("Confirm"),
                                    tr("Do you want to cancel the selected job?"),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No);
    if (ret == QMessageBox::No)
        return;

    ui->jobCancelButton->setEnabled(false);

    QString url = baseUrl + "/job/" + jobId + "/cancel";
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    QNetworkReply *reply = nam->post(req, QByteArray());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        auto response = reply->readAll();
        auto status = QJsonDocument::fromJson(response).object()["status"].toString();
        if (reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(this, tr(""), tr(qPrintable(status)));
            emit ui->jobRefreshButton->clicked();
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "CAESAR POST /job/cancel error" << statusCode << status;
            QMessageBox::critical(this, tr("Error"), status);
        }
        reply->deleteLater();

        ui->jobCancelButton->setEnabled(true);
    });
}

void CaesarWindow::on_dataTable_itemSelectionChanged()
{
    QString id;
    if (selectedItemId(ui->dataTable, id)) {
        ui->fileLineEdit->setText(id);
    } else {
        ui->fileLineEdit->clear();
    }
}

void CaesarWindow::on_appsComboBox_currentTextChanged(const QString &app)
{
    // Clear previous form if already present
    inputs.clear();
    foreach (const auto &box, boxes.keys()) {
        boxes[box]->deleteLater();
        boxes.remove(box);
    }

    QString url = baseUrl + "/app/" + app + "/describe";
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    QNetworkReply *reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            const auto formInputs = QJsonDocument::fromJson(reply->readAll()).object();
            buildJobForm(formInputs);
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            QString status = QJsonDocument::fromJson(reply->readAll()).object()["status"].toString();
            qDebug() << "CAESAR GET /app/describe" << statusCode << status;
            QMessageBox::critical(this, tr("Error"), status);
        }
        reply->deleteLater();
    });
}

void CaesarWindow::on_jobSubmitButton_clicked()
{
    if (ui->fileLineEdit->text().isEmpty()) {
        QMessageBox::information(this, tr(""), tr("No file selected."));
        return;
    }

    if (ui->appsComboBox->currentText().isEmpty()) {
        QMessageBox::information(this, tr(""), tr("No app selected."));
        return;
    }

    QVariantMap map;
    foreach (const auto &key, inputs.keys()) {
        switch (inputs[key].first) {
        case QVariant::String: {
            auto l = qobject_cast<QComboBox *>(inputs[key].second);
            map[key] = l->currentText();
            break;
        }
        case QVariant::Int: {
            auto l = qobject_cast<QSpinBox *>(inputs[key].second);
            map[key] = l->value();
            break;
        }
        case QVariant::Double: {
            auto l = qobject_cast<QDoubleSpinBox *>(inputs[key].second);
            map[key] = l->value();
            break;
        }
        case QVariant::Bool: {
            auto l = qobject_cast<QCheckBox *>(inputs[key].second);
            map[key] = l->isChecked();
            break;
        }
        default:
            break;
        }
    }

    QJsonObject job;
    job["app"] = ui->appsComboBox->currentText();
    job["data_inputs"] = ui->fileLineEdit->text();
    job["tag"] = ui->tagLineEdit->text();
    job["job_inputs"] = QJsonObject::fromVariantMap(map);
    QJsonDocument doc(job);

    ui->jobSubmitButton->setEnabled(false);

    QString url = baseUrl + "/job";
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = nam->post(req, doc.toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        auto response = QJsonDocument::fromJson(reply->readAll()).object();
        if (reply->error() == QNetworkReply::NoError) {
            auto msg = response["status"].toString() + ".\nJob id " + response["job_id"].toString();
            QMessageBox::information(this, tr("Success"), msg);
            emit ui->jobRefreshButton->clicked();
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            QString status = response["status"].toString();
            qDebug() << "CAESAR POST /job ERROR" << statusCode << status;
            QMessageBox::critical(this, tr("Error"), status);
        }
        reply->deleteLater();
        ui->jobSubmitButton->setEnabled(true);
    });
}
