#include "simpleconesearchform.h"
#include "ui_simpleconesearchform.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QSettings>
#include <QUrl>
#include <QUrlQuery>

SimpleConeSearchForm::SimpleConeSearchForm(QWidget *parent)
    : QWidget(parent, Qt::Window), ui(new Ui::SimpleConeSearchForm), idxAccessURL(-1)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);

    ui->table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(ui->btnSearch, &QPushButton::clicked, this, &SimpleConeSearchForm::search);
    connect(ui->btnCutout, &QPushButton::clicked, this, &SimpleConeSearchForm::requestCutout);

    QSettings settings(QDir::homePath().append("/VisIVODesktopTemp/setting.ini"),
                       QSettings::IniFormat);
    this->pythonExe = settings.value("python.exe").toString();
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

    QProcess scs;
    scs.setProgram(this->pythonExe);
    scs.setProcessChannelMode(QProcess::MergedChannels);

    QDir dir(QApplication::applicationDirPath());

    QStringList args;
    args << dir.absoluteFilePath("pyvo_scs.py")
         << QDir::home().absoluteFilePath("VisIVODesktopTemp/tmp_download") << ui->textURL->text()
         << ui->textRA->text() << ui->textDec->text() << ui->textRadius->text()
         << QString::number(ui->comboVerbosity->currentIndex() + 1);
    scs.setArguments(args);
    scs.start();
    scs.waitForFinished();

    if (scs.exitCode() != QProcess::NormalExit) {
        QMessageBox::critical(this, QString(), scs.readAllStandardOutput());
        return;
    }

    QString path = scs.readAllStandardOutput();
    this->updateTable(path);
}

void SimpleConeSearchForm::requestCutout()
{
    // if (this->idxAccessURL < 0) {
    //     QMessageBox::critical(this, QString(), "Table has no access_url column.");
    //     return;
    // }

    // if (ui->table->selectedItems().empty()) {
    //     QMessageBox::information(this, QString(), "No item selected.");
    //     return;
    // }

    // int row = ui->table->selectedItems().first()->row();
    // std::string url = ui->table->item(row, this->idxAccessURL)->text().toStdString();

    // try {
    //     py::module_ datalink = py::module_::import("pyvo_datalink");
    //     py::dict res = datalink.attr("soda_cutout_sync")(url);
    //     if (res["exit_code"].cast<int>() != 0) {
    //         throw std::runtime_error(res["error"].cast<std::string>());
    //     }

    //     QString soda = QString::fromStdString(res["result"]["url"].cast<std::string>());
    //     QString id = QString::fromStdString(res["result"]["id"].cast<std::string>());

    //     QString fn = QFileDialog::getSaveFileName(
    //             this, QString(), QDir::home().absoluteFilePath(QFileInfo(id).fileName()),
    //             "FITS files (*.fits)");
    //     if (fn.isEmpty()) {
    //         return;
    //     }

    //     QUrl reqUrl(soda);
    //     QUrlQuery query;
    //     query.addQueryItem("ID", id);
    //     query.addQueryItem(
    //             "CIRCLE",
    //             QString("%1 %2 %3")
    //                     .arg(ui->textRA->text(), ui->textDec->text(), ui->textRadius->text()));
    //     reqUrl.setQuery(query);

    //     auto loading = new LoadingWidget;
    //     loading->setText("Downloading cutout...");
    //     loading->show();

    //     auto nam = new QNetworkAccessManager(this);
    //     QNetworkRequest req(reqUrl);
    //     auto reply = nam->get(req);
    //     loading->setLoadingProcess(reply);
    //     connect(reply, &QNetworkReply::finished, this, [=]() {
    //         reply->deleteLater();
    //         nam->deleteLater();
    //         loading->deleteLater();

    //         if (reply->error()) {
    //             QMessageBox::critical(this, QString(), reply->errorString());
    //             return;
    //         }

    //         QFile f(fn);
    //         f.open(QIODevice::WriteOnly);
    //         f.write(reply->readAll());
    //         f.close();
    //         QMessageBox::information(this, QString(), "Download completed.");
    //     });
    // } catch (const std::exception &e) {
    //     QMessageBox::critical(this, QString(), e.what());
    // }
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

void SimpleConeSearchForm::updateTable(const QString &filepath)
{
    this->idxAccessURL = -1;

    QFile f(filepath);
    f.open(QIODevice::ReadOnly);

    auto columns = f.readLine().simplified().split(',');
    ui->table->setColumnCount(columns.size());
    for (int i = 0; i < columns.size(); ++i) {
        auto item = new QTableWidgetItem;
        item->setText(columns[i]);
        ui->table->setHorizontalHeaderItem(i, item);

        if (columns[i] == "access_url") {
            this->idxAccessURL = i;
        }
    }

    while (f.canReadLine()) {
        auto row = f.readLine().simplified().split(',');
        int lastRow = ui->table->rowCount();
        ui->table->insertRow(lastRow);
        for (int i = 0; i < row.size(); ++i) {
            ui->table->setItem(lastRow, i, new QTableWidgetItem(QString(row[i])));
        }
    }
}
