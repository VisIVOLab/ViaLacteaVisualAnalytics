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
    this->pythonExe = settings.value("python.exe").toString();
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
    QProcess avail;
    avail.setProgram(this->pythonExe);
    avail.setProcessChannelMode(QProcess::MergedChannels);

    QStringList args;
    args << QDir(QApplication::applicationDirPath()).absoluteFilePath("pyvo_tap.py") << "avail"
         << this->url;
    avail.setArguments(args);
    avail.start();
    avail.waitForFinished();

    if (avail.exitCode() != QProcess::NormalExit) {
        this->isConnected = false;
        QMessageBox::critical(this, QString(), avail.readAllStandardOutput());
        return;
    } else {
        this->isConnected = true;
        if (!this->isFilament) {
            fetchBandTables();
        }
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

    QProcess proc;
    proc.setProgram(this->pythonExe);

    QStringList args;
    args << QDir(QApplication::applicationDirPath()).absoluteFilePath("pyvo_tap.py") << "query"
         << this->url << generateQuery() << output_file << "ascii.tab";
    proc.setArguments(args);
    proc.start();
    proc.waitForFinished();

    if (proc.exitCode() != QProcess::NormalExit) {
        QMessageBox::critical(this, QString(), proc.readAllStandardError());
        return;
    }

    int nels = proc.readLine().simplified().toInt();
    if (nels == 0) {
        QMessageBox::critical(this, QString(), "Empty table. Table contained no rows.");
        return;
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

    QProcess proc;
    proc.setProgram(this->pythonExe);

    QStringList args;
    args << QDir(QApplication::applicationDirPath()).absoluteFilePath("pyvo_tap.py") << "tables"
         << this->url;
    proc.setArguments(args);
    proc.start();
    proc.waitForFinished();

    if (proc.exitCode() != QProcess::NormalExit) {
        QMessageBox::critical(this, QString(), proc.readAllStandardOutput());
        return;
    }

    auto tables = proc.readAllStandardOutput().simplified().split(',');
    qDebug() << Q_FUNC_INFO << tables;
    for (const auto &table : tables) {
        ui->tableComboBox->addItem(table);
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
