#include <pybind11/embed.h>

#include "vlkbsimplequerycomposer.h"
#include "ui_vlkbsimplequerycomposer.h"

#include "authkeys.h"
#include "vialactea_fileload.h"

#include <QAuthenticator>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QSettings>
#include <QtDebug>
#include <QUrl>
#include <QUuid>

namespace py = pybind11;

VLKBSimpleQueryComposer::VLKBSimpleQueryComposer(vtkwindow_new *v, QWidget *parent)
    : QWidget(parent), ui(new Ui::VLKBSimpleQueryComposer)
{
    ui->setupUi(this);
    vtkwin = v;
    loading = new LoadingWidget();
    isConnected = false;
    isFilament = false;
    is3dSelections = false;
    isBubble = false;

    m_sSettingsFile = QDir::homePath()
            .append(QDir::separator())
            .append("VisIVODesktopTemp")
            .append(QDir::separator())
            .append("setting.ini");
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    url = settings.value("vlkbtableurl", "").toString();
    ui->vlkbUrlLineEdit->setText(url);
    url.replace("http://", QString("http://%1:%2@").arg(IA2_TAP_USER, IA2_TAP_PASS));
    vlkbtype = settings.value("vlkbtype", "ia2").toString();
    table_prefix = "vlkb_";
    on_connectPushButton_clicked();
}

VLKBSimpleQueryComposer::~VLKBSimpleQueryComposer()
{
    delete ui;
}

void VLKBSimpleQueryComposer::setVtkWindow(vtkwindow_new *v)
{
    vtkwin = v;
}
void VLKBSimpleQueryComposer::setIsFilament()
{
    isFilament = true;
    ui->tableGroupBox->hide();
}

void VLKBSimpleQueryComposer::setIsBubble()
{
    isBubble = true;
    ui->tableGroupBox->hide();
}

void VLKBSimpleQueryComposer::setIs3dSelections()
{
    is3dSelections = true;
    ui->tableGroupBox->hide();
}

void VLKBSimpleQueryComposer::setLongitude(float long1, float long2)
{
    float min = long1;
    float max = long2;
    if (long1 >= long2) {
        max = long1;
        min = long2;
    }
    ui->longMaxLineEdit->setText(QString::number(max));
    ui->longMinLineEdit->setText(QString::number(min));
}

void VLKBSimpleQueryComposer::setLatitude(float lat1, float lat2)
{
    float min = lat1;
    float max = lat2;
    if (lat1 >= lat2) {
        max = lat1;
        min = lat2;
    }
    ui->latMaxLineEdit->setText(QString::number(max));
    ui->latMinLineEdit->setText(QString::number(min));
}

void VLKBSimpleQueryComposer::on_connectPushButton_clicked()
{
    try {
        if (!isConnected) {
            // Check TAP availability
            py::module_ tap = py::module_::import("pyvo_tap");
            py::dict res = tap.attr("availability")(url.toStdString());

            if (res["exit_code"].cast<int>() != 0) {
                throw std::runtime_error(res["error"].cast<std::string>());
            }

            if (!res["result"].cast<bool>()) {
                throw std::runtime_error("TAP is not available.");
            }

            isConnected = true;
            if (!isFilament) {
                fetchBandTables();
            }
        } else {
            isConnected = false;
        }
    } catch (const std::exception &e) {
        isConnected = false;
        QMessageBox::critical(this, QString(), e.what());
    }

    updateUI(isConnected);
}

void VLKBSimpleQueryComposer::on_queryPushButton_clicked()
{
    doQuery(ui->tableComboBox->currentText());
    this->close();
}

void VLKBSimpleQueryComposer::doQuery(QString band)
{
    QString output_file;
    if (ui->savedatasetCheckBox->isChecked()) {
        output_file = ui->outputNameLineEdit->text();
    } else {
        output_file = QString("%1/tmp_download/%2.dat")
                              .arg(QDir::home().absoluteFilePath("VisIVODesktopTemp"),
                                   QUuid::createUuid().toString(QUuid::WithoutBraces));
    }

    try {
        py::module_ tap = py::module_::import("pyvo_tap");
        std::string query = generateQuery().toStdString();
        std::string out_format = "ascii.tab";

        py::dict res =
                tap.attr("search")(url.toStdString(), query, output_file.toStdString(), out_format);
        if (res["exit_code"].cast<int>() != 0) {
            throw std::runtime_error(res["error"].cast<std::string>());
        }

        if (res["result"]["nels"].cast<int>() == 0) {
            throw std::runtime_error("Empty table. Table contained no rows.");
        }

        auto fileload = new Vialactea_FileLoad(output_file, true);
        fileload->setVtkWin(vtkwin);
        if (isFilament) {
            fileload->importFilaments();
        } else if (isBubble) {
            fileload->importBubbles();
        } else if (is3dSelections) {
            fileload->import3dSelection();
        } else {
            fileload->setCatalogueActive();
            if (!isBandmerged) {
                QString w = ui->tableComboBox->currentText().replace("band", "").replace("um", "");
                fileload->setWavelength(w);
            } else {
                fileload->setWavelength("all");
            }
            fileload->on_okPushButton_clicked();
        }

    } catch (const std::exception &e) {
        QMessageBox::critical(this, QString(), e.what());
    }
}

void VLKBSimpleQueryComposer::updateUI(bool enabled)
{
    QString btnText = enabled ? "Disconnect" : "Connect";
    ui->connectPushButton->setText(btnText);
    ui->vlkbUrlLineEdit->setEnabled(!enabled);
    ui->tableComboBox->setEnabled(enabled);
    ui->longMinLineEdit->setEnabled(enabled);
    ui->longMaxLineEdit->setEnabled(enabled);
    ui->latMinLineEdit->setEnabled(enabled);
    ui->latMaxLineEdit->setEnabled(enabled);
    ui->queryPushButton->setEnabled(enabled);
    ui->outputNameLineEdit->setEnabled(enabled);
}

void VLKBSimpleQueryComposer::fetchBandTables()
{
    ui->tableComboBox->clear();

    py::module_ tap = py::module_::import("pyvo_tap");
    py::dict res = tap.attr("band_tables")(url.toStdString());
    if (res["exit_code"].cast<int>() != 0) {
        throw std::runtime_error(res["error"].cast<std::string>());
    }

    py::list tables = res["result"];
    for (const auto &table : tables) {
        ui->tableComboBox->addItem(QString::fromStdString(table.cast<std::string>()));
    }
    ui->tableComboBox->addItem("bandmerged");
    ui->tableComboBox->setCurrentIndex(ui->tableComboBox->count() - 1);
}

QString VLKBSimpleQueryComposer::generateQuery()
{
    if (isFilament) {
        return QString("SELECT f.idfil_mos AS idfil_mos, f.contour AS contour, f.glon AS glon, "
                       "f.glat "
                       "AS glat, b.contour AS branches_contour, b.contour1d AS branches_contour1d, "
                       "b.contour_new AS branches_contour_new, b.flagspine AS flagspine_branches "
                       "FROM "
                       "vlkb_filaments.filaments AS f JOIN vlkb_filaments.branches AS b on "
                       "f.idfil_mos = b.idfil_mos WHERE (glon >= %1 AND glon <= %2) AND "
                       "(glat >= %3 AND glat <= %4)")
                .arg(ui->longMinLineEdit->text(), ui->longMaxLineEdit->text(),
                     ui->latMinLineEdit->text(), ui->latMaxLineEdit->text());
    }

    if (is3dSelections) {
        return QString("SELECT dist * cos(radians(glon)+ 3.14159265359 / 2) AS x, dist * "
                       "sin(radians(glon)+ 3.14159265359 / 2) AS y, dist * tan(radians(glat)) AS "
                       "z, * FROM vlkb_compactsources.props_dist WHERE ((glon >= %1 AND "
                       "glon <= %2) AND (glat >= %3 AND glat <= %4))")
                .arg(ui->longMinLineEdit->text(), ui->longMaxLineEdit->text(),
                     ui->latMinLineEdit->text(), ui->latMaxLineEdit->text());
    }

    QString band = ui->tableComboBox->currentText();
    if (band == "bandmerged") {
        isBandmerged = true;
        return QString("SELECT DISTINCT * FROM vlkb_compactsources.sed_view_final WHERE ((glonft "
                       ">= "
                       "%1 AND glonft <= %2) AND (glatft >= %3 AND glatft <= %4))")
                .arg(ui->longMinLineEdit->text(), ui->longMaxLineEdit->text(),
                     ui->latMinLineEdit->text(), ui->latMaxLineEdit->text());
    }

    isBandmerged = false;
    return QString("SELECT * FROM vlkb_compactsources.%5 WHERE (glon >= %1 AND "
                   "glon <= %2) AND (glat >= %3 AND glat <= %4)")
            .arg(ui->longMinLineEdit->text(), ui->longMaxLineEdit->text(),
                 ui->latMinLineEdit->text(), ui->latMaxLineEdit->text(), band);
}

void VLKBSimpleQueryComposer::on_tableComboBox_currentIndexChanged(int index)
{
    ui->outputNameLineEdit->setText(ui->tableComboBox->itemText(index) + ".dat");
    ui->outputNameLineEdit->setFocus();
}

void VLKBSimpleQueryComposer::on_savedatasetCheckBox_clicked(bool checked)
{
    if (checked) {
        ui->navigateFSButtono->setEnabled(true);
        ui->outputNameLabel->setEnabled(true);
        ui->outputNameLineEdit->setEnabled(true);
    } else {
        ui->navigateFSButtono->setEnabled(false);
        ui->outputNameLabel->setEnabled(false);
        ui->outputNameLineEdit->setEnabled(false);
    }
}

void VLKBSimpleQueryComposer::on_navigateFSButtono_clicked()
{
    QString fn =
            QFileDialog::getSaveFileName(this, "Save as...", QString(), "dataset files (*.dat)");
    if (!fn.isEmpty() && !fn.endsWith(".dat", Qt::CaseInsensitive))
        fn += ".dat"; // default
    ui->outputNameLineEdit->setText(fn);
}

void VLKBSimpleQueryComposer::closeEvent(QCloseEvent *event)
{
    if (vtkwin != 0) {
        vtkwin->activateWindow();
    }

    QWidget::closeEvent(event);
}
