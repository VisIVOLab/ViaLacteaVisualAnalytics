#include "caesarwidget.h"
#include "ui_caesarwidget.h"

#include <QJsonDocument>
#include <QMessageBox>
#include <QFileDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTimer>
#include <QDateTime>

#include "loadingwidget.h"

CaesarWidget::CaesarWidget(QWidget *parent, vtkSmartPointer<vtkFitsReader> parentImage) :
    QWidget(parent, Qt::Window),
    ui(new Ui::CaesarWidget),
    parentImage(parentImage),
    auth(&NeaniasCaesarAuth::Instance()),
    jobRefreshPeriod(10)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    ui->dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->jobsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    nam = new QNetworkAccessManager(this);

    // Ask to upload the current file
    if (parentImage != nullptr) {
        auto msg = new QMessageBox(this);
        msg->setIcon(QMessageBox::Question);
        msg->setWindowTitle("Upload image");
        msg->setText("Do you want to upload the current image?");
        msg->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msg->setDefaultButton(QMessageBox::No);
        msg->show();
        connect(msg, &QMessageBox::buttonClicked, this, [this, msg, parentImage](QAbstractButton *button){
            if (msg->standardButton(button) == QMessageBox::Yes) {
                QString fn = QString::fromStdString(parentImage->GetFileName());
                uploadData(fn);
            }
            msg->deleteLater();
        });
    }

    // Schedule jobs status refresh
    if (jobRefreshPeriod > 0) {
        auto timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &CaesarWidget::on_jobRefreshButton_clicked);
        timer->start(jobRefreshPeriod * 1000);
    }

    on_dataRefreshButton_clicked();
    // on_jobRefreshButton_clicked();
    getSupportedApps();
}

CaesarWidget::~CaesarWidget()
{
    delete ui;
}

QString CaesarWidget::baseUrl()
{
    return QString("http://caesar-api.dev.neanias.eu/caesar/api/v1.0");
}

void CaesarWidget::uploadData(const QString &fn)
{
    auto file = new QFile(fn);
    if (!file->open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr(qPrintable(file->errorString())));
        return;
    }

    auto multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    file->setParent(multipart);
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/fits"));
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
    connect(reply, &QNetworkReply::finished, this, [this, reply, loadingWidget](){
        if (reply->error() == QNetworkReply::NoError) {
            auto response = QJsonDocument::fromJson(reply->readAll()).object();
            auto msg = response["status"].toString() + ".\nUUID " + response["uuid"].toString();
            QMessageBox::information(loadingWidget, tr("Success"), msg);
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

void CaesarWidget::updateDataTable(const QJsonArray &files)
{
    ui->dataTable->clearContents();
    ui->dataTable->setRowCount(0);

    foreach (const auto &it, files) {
        const auto file = it.toObject();
        auto id = file["fileid"].toString();
        auto name = file["filename_orig"].toString();
        auto tag = file["tag"].toString();
        auto date = formatDate(file["filedate"].toString());

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
        auto date = formatDate(job["submit_date"].toString());
        auto status = job["state"].toString();
        auto elapsed_time = formatElapsedTime(job["elapsed_time"].toInt());

        ui->jobsTable->insertRow(ui->jobsTable->rowCount());
        ui->jobsTable->setItem(ui->jobsTable->rowCount()-1, 0, new QTableWidgetItem(id));
        ui->jobsTable->setItem(ui->jobsTable->rowCount()-1, 1, new QTableWidgetItem(tag));
        ui->jobsTable->setItem(ui->jobsTable->rowCount()-1, 2, new QTableWidgetItem(date));
        ui->jobsTable->setItem(ui->jobsTable->rowCount()-1, 3, new QTableWidgetItem(status));
        ui->jobsTable->setItem(ui->jobsTable->rowCount()-1, 4, new QTableWidgetItem(elapsed_time));
    }
}

void CaesarWidget::getSupportedApps()
{
    ui->appsComboBox->clear();

    auto url = baseUrl() + "/apps";
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    auto reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        if (reply->error() == QNetworkReply::NoError) {
            const auto apps = QJsonDocument::fromJson(reply->readAll()).object()["apps"].toArray();
            foreach (const auto &app, apps) {
                ui->appsComboBox->insertItem(ui->appsComboBox->count(), app.toString());
            }
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "CaesarWidget.getSupportedApps.error" << statusCode << reply->errorString();
            QMessageBox::critical(this, tr("Error"), tr(qPrintable(reply->errorString())));
        }
        reply->deleteLater();
    });
}

void CaesarWidget::buildJobForm(const QJsonObject &app)
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
            layout = qobject_cast<QFormLayout*>(box->layout());
        } else {
            box = new QGroupBox(ui->jobSubmissionWidget);
            box->setTitle(category);
            layout = new QFormLayout(box);
            box->setLayout(layout);
            boxes.insert(category, box);
        }

        if (subcategory != "") {
            auto subbox = box->findChild<QGroupBox*>(subcategory);
            if (subbox == nullptr) {
                subbox = new QGroupBox(box);
                subbox->setObjectName(subcategory);
                subbox->setTitle(subcategory);
                subbox->setLayout(new QFormLayout(subbox));
            }
            box = subbox;
            layout = qobject_cast<QFormLayout*>(box->layout());
        }

        QWidget *widget;
        QPair<QVariant::Type, QWidget*> p;

        if (type == "str") {
            p.first = QVariant::String;
            auto tmp = new QComboBox(box);
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
            tmp->setRange(min, max);
            tmp->setValue(default_value.toInt());
            widget = tmp;
        } else if (type == "float") {
            p.first = QVariant::Double;
            auto min = app[input].toObject()["min"].toDouble();
            auto max = app[input].toObject()["max"].toDouble();
            auto tmp = new QDoubleSpinBox(box);
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
        auto sub = box->findChildren<QGroupBox*>();
        foreach(const auto &it, sub) {
            box->layout()->addWidget(it);
        }
        ui->jobSubmissionForm->layout()->addWidget(box);
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

bool CaesarWidget::selectedDataFilename(QString &filename)
{
    auto items = ui->dataTable->selectedItems();
    if (items.count() < 1) {
        return false;
    }
    filename = items[1]->text();
    return true;
}

QString CaesarWidget::formatDate(const QString &date)
{
    return QDateTime::fromString(date, Qt::ISODateWithMs).toString(Qt::DefaultLocaleShortDate);
}

QString CaesarWidget::formatElapsedTime(const int &elapsedTime)
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

void CaesarWidget::on_dataRefreshButton_clicked()
{
    ui->dataRefreshButton->setEnabled(false);

    auto url = baseUrl() + "/fileids";
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    auto reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
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
    QString fn = QFileDialog::getOpenFileName(this,tr("Upload file"), QDir::homePath(), tr("Images (*.fits)"));
    if (!fn.isEmpty()) {
        uploadData(fn);
    }
}

void CaesarWidget::on_dataDownloadButton_clicked()
{
    QString fileId, srcName;
    if (!selectedItemId(ui->dataTable, fileId) || !selectedDataFilename(srcName)) {
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
    connect(reply, &QNetworkReply::finished, this, [this, reply, fileId, filename, loadingWidget](){
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
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
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

void CaesarWidget::on_dataTable_itemSelectionChanged()
{
    QString id;
    if (selectedItemId(ui->dataTable, id)) {
        ui->fileLineEdit->setText(id);
    } else {
        ui->fileLineEdit->clear();
    }
}

void CaesarWidget::on_jobRefreshButton_clicked()
{
    ui->jobRefreshButton->setEnabled(false);

    auto url = baseUrl() + "/jobs";
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    auto reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
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
    connect(reply, &QNetworkReply::finished, this, [this, reply, jobId, filename, loadingWidget](){
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
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        auto response = reply->readAll();
        auto status = QJsonDocument::fromJson(response).object()["status"].toString();
        if (reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(this, tr(""), tr(qPrintable(status)));
            emit ui->jobRefreshButton->clicked();
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "CaesarWidget.on_dataDeleteButton_clicked.error" << statusCode << reply->errorString();
            QMessageBox::critical(this, tr("Error"), tr(qPrintable(status)));
        }
        reply->deleteLater();

        ui->jobCancelButton->setEnabled(true);
    });
}

void CaesarWidget::on_jobSubmitButton_clicked()
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
    foreach (const auto &key, inputs.keys())
    {
        switch (inputs[key].first) {
        case QVariant::String: {
            auto l = qobject_cast<QComboBox*>(inputs[key].second);
            map[key] = l->currentText();
            break;
        }
        case QVariant::Int: {
            auto l = qobject_cast<QSpinBox*>(inputs[key].second);
            map[key] = l->value();
            break;
        }
        case QVariant::Double: {
            auto l = qobject_cast<QDoubleSpinBox*>(inputs[key].second);
            map[key] = l->value();
            break;
        }
        case QVariant::Bool: {
            auto l = qobject_cast<QCheckBox*>(inputs[key].second);
            map[key] = l->isChecked();
            break;
        }
        default: break;
        }
    }

    QJsonObject job;
    job["app"] = ui->appsComboBox->currentText();
    job["data_inputs"] = ui->fileLineEdit->text();
    job["tag"] = "";
    job["job_inputs"] = QJsonObject::fromVariantMap(map);
    QJsonDocument doc(job);

    ui->jobSubmitButton->setEnabled(false);

    auto url = baseUrl() + "/job";
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    auto reply = nam->post(req, doc.toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        if (reply->error() == QNetworkReply::NoError) {
            auto response = QJsonDocument::fromJson(reply->readAll()).object();
            auto msg = response["status"].toString() + ".\nJob id " + response["job_id"].toString();
            QMessageBox::information(this, tr("Success"), msg);
            emit ui->jobRefreshButton->clicked();
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "CaesarWidget.on_jobSubmitButton_clicked.error" << statusCode << reply->errorString();
            QMessageBox::critical(this, tr("Error"), tr(qPrintable(reply->errorString())));
        }
        reply->deleteLater();
        ui->jobSubmitButton->setEnabled(true);
    });
}

void CaesarWidget::on_appsComboBox_currentTextChanged(const QString &app)
{
    // Clear previous form if already present
    inputs.clear();
    foreach (const auto &box, boxes.keys()) {
        boxes[box]->deleteLater();
        boxes.remove(box);
    }

    auto url = baseUrl() + "/app/" + app + "/describe";
    QNetworkRequest req(url);
    auth->putAccessToken(req);
    auto reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        if (reply->error() == QNetworkReply::NoError) {
            const auto formInputs = QJsonDocument::fromJson(reply->readAll()).object();
            buildJobForm(formInputs);
        } else {
            auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "CaesarWidget.on_appsComboBox_currentTextChanged.error" << statusCode << reply->errorString();
            QMessageBox::critical(this, tr("Error"), tr(qPrintable(reply->errorString())));
        }
        reply->deleteLater();
    });
}
