#include <pybind11/embed.h>

#include "simpleconesearchform.h"
#include "ui_simpleconesearchform.h"

#include "loadingwidget.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

namespace py = pybind11;

SimpleConeSearchForm::SimpleConeSearchForm(QWidget *parent)
    : QWidget(parent, Qt::Window), ui(new Ui::SimpleConeSearchForm), idxAccessURL(-1)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(ui->btnSearch, &QPushButton::clicked, this, &SimpleConeSearchForm::search);
    connect(ui->btnCutout, &QPushButton::clicked, this, &SimpleConeSearchForm::requestCutout);
}

SimpleConeSearchForm::~SimpleConeSearchForm()
{
    delete ui;
}

void SimpleConeSearchForm::search()
{
    if (!checkInputs()) {
        return;
    }

    std::string url = ui->textURL->text().toStdString();
    float ra = ui->textRA->text().toFloat();
    float dec = ui->textDec->text().toFloat();
    float radius = ui->textRadius->text().toFloat();
    int verbosity = ui->comboVerbosity->currentIndex() + 1;

    try {
        py::module_ scs = py::module_::import("pyvo_scs");
        py::dict res = scs.attr("search")(url, ra, dec, radius, verbosity);
        if (res["exit_code"].cast<int>() != 0) {
            throw std::runtime_error(res["error"].cast<std::string>());
        }

        updateTable(res["result"]);
    } catch (const std::exception &e) {
        QMessageBox::critical(this, QString(), e.what());
    }
}

void SimpleConeSearchForm::requestCutout()
{
    if (this->idxAccessURL < 0) {
        QMessageBox::critical(this, QString(), "Table has no access_url column.");
        return;
    }

    if (ui->table->selectedItems().empty()) {
        QMessageBox::information(this, QString(), "No item selected.");
        return;
    }

    int row = ui->table->selectedItems().first()->row();
    std::string url = ui->table->item(row, this->idxAccessURL)->text().toStdString();

    try {
        py::module_ datalink = py::module_::import("pyvo_datalink");
        py::dict res = datalink.attr("soda_cutout_sync")(url);
        if (res["exit_code"].cast<int>() != 0) {
            throw std::runtime_error(res["error"].cast<std::string>());
        }

        QString soda = QString::fromStdString(res["result"]["url"].cast<std::string>());
        QString id = QString::fromStdString(res["result"]["id"].cast<std::string>());

        QString fn = QFileDialog::getSaveFileName(
                this, QString(), QDir::home().absoluteFilePath(QFileInfo(id).fileName()),
                "FITS files (*.fits)");
        if (fn.isEmpty()) {
            return;
        }

        QUrl reqUrl(soda);
        QUrlQuery query;
        query.addQueryItem("ID", id);
        query.addQueryItem(
                "CIRCLE",
                QString("%1 %2 %3")
                        .arg(ui->textRA->text(), ui->textDec->text(), ui->textRadius->text()));
        reqUrl.setQuery(query);

        auto loading = new LoadingWidget;
        loading->setText("Downloading cutout...");
        loading->show();

        auto nam = new QNetworkAccessManager(this);
        QNetworkRequest req(reqUrl);
        auto reply = nam->get(req);
        loading->setLoadingProcess(reply);
        connect(reply, &QNetworkReply::finished, this, [=]() {
            reply->deleteLater();
            nam->deleteLater();
            loading->deleteLater();

            if (reply->error()) {
                QMessageBox::critical(this, QString(), reply->errorString());
                return;
            }

            QFile f(fn);
            f.open(QIODevice::WriteOnly);
            f.write(reply->readAll());
            f.close();
            QMessageBox::information(this, QString(), "Download completed.");
        });
    } catch (const std::exception &e) {
        QMessageBox::critical(this, QString(), e.what());
    }
}

bool SimpleConeSearchForm::checkInputs()
{
    if (!QUrl(ui->textURL->text()).isValid()) {
        QMessageBox::critical(this, QString(), "Cone URL is not a valid URL!");
        return false;
    }

    bool ok;
    ui->textRA->text().toFloat(&ok);
    if (!ok) {
        QMessageBox::critical(this, QString(), "RA value is not valid!");
        return false;
    }

    ui->textDec->text().toFloat(&ok);
    if (!ok) {
        QMessageBox::critical(this, QString(), "Dec value is not valid!");
        return false;
    }

    ui->textRadius->text().toFloat(&ok);
    if (!ok) {
        QMessageBox::critical(this, QString(), "Radius value is not valid!");
        return false;
    }

    return true;
}

void SimpleConeSearchForm::updateTable(const pybind11::dict &dict)
{
    this->idxAccessURL = -1;
    const py::list &columns = dict["columns"];
    ui->table->setColumnCount(columns.size());
    for (int i = 0; i < columns.size(); ++i) {
        const py::tuple &col = columns[i];
        auto item = new QTableWidgetItem;
        auto text = QString::fromStdString(col[0].cast<std::string>());
        item->setText(text);
        item->setToolTip(QString::fromStdString(col[1].cast<std::string>()));
        ui->table->setHorizontalHeaderItem(i, item);

        if (text == "access_url") {
            this->idxAccessURL = i;
        }
    }

    const py::list &table = dict["table"];
    ui->table->setRowCount(table.size());
    for (int i = 0; i < table.size(); ++i) {
        const py::list &row = table[i];
        for (int j = 0; j < columns.size(); ++j) {
            ui->table->setItem(
                    i, j, new QTableWidgetItem(QString::fromStdString(row[j].cast<std::string>())));
        }
    }
}
