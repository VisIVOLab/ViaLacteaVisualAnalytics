#include "HiPS2FITSForm.h"
#include "ui_HiPS2FITSForm.h"

#include "loadingwidget.h"
#include "vtkfitsreader.h"
#include "vtkwindow_new.h"

#include <QCompleter>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

HiPS2FITSForm::HiPS2FITSForm(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::HiPS2FITSForm),
      endpoint("http://alasky.cds.unistra.fr/hips-image-services/hips2fits")
{
    ui->setupUi(this);
    this->setWindowFlag(Qt::Window);
    this->setAttribute(Qt::WA_DeleteOnClose);

    auto intValidator = new QIntValidator(this);
    intValidator->setBottom(1);
    ui->lineWidth->setValidator(intValidator);
    ui->lineHeight->setValidator(intValidator);

    auto doubleValidator = new QDoubleValidator(this);
    ui->lineRA->setValidator(doubleValidator);
    ui->lineDec->setValidator(doubleValidator);
    ui->lineFoV->setValidator(doubleValidator);
    ui->lineRotationAngle->setValidator(doubleValidator);
    ui->lineMinCut->setValidator(doubleValidator);
    ui->lineMaxCut->setValidator(doubleValidator);

    this->nam = new QNetworkAccessManager(this);

    // Setup predefined list of HiPS
    QStringList els;
    els << "CDS/C/HI4PI/HI"
        << "CDS/P/2MASS/color"
        << "CDS/P/ACT_Planck/DR5/f220"
        << "CDS/P/CO	"
        << "CDS/P/DES-DR2/ColorIRG"
        << "CDS/P/DSS2/color"
        << "CDS/P/Fermi/3"
        << "CDS/P/Fermi/4"
        << "CDS/P/Fermi/5"
        << "CDS/P/Finkbeiner"
        << "CDS/P/GALEXGR6_7/FUV"
        << "CDS/P/GALEXGR6_7/NUV"
        << "CDS/P/HGPS/Flux"
        << "CDS/P/IRIS/color"
        << "CDS/P/MeerKAT/Galactic-Centre-1284MHz-StokesI"
        << "CDS/P/NVSS"
        << "CDS/P/PLANCK/R3/HFI/color"
        << "CDS/P/PLANCK/R3/HFI100"
        << "CDS/P/PLANCK/R3/LFI30"
        << "CDS/P/PanSTARRS/DR1/color-i-r-g"
        << "CDS/P/PanSTARRS/DR1/g"
        << "CDS/P/PanSTARRS/DR1/i"
        << "CDS/P/PanSTARRS/DR1/r"
        << "CDS/P/PanSTARRS/DR1/y"
        << "CDS/P/PanSTARRS/DR1/z"
        << "CDS/P/unWISE/color-W2-W1W2-W1"
        << "CSIRO/P/RACS/low/I"
        << "CSIRO/P/RACS/mid/I"
        << "ESAVO/P/AKARI/N60"
        << "ESAVO/P/AKARI/WideL"
        << "ESAVO/P/HERSCHEL/SPIRE-250"
        << "ESAVO/P/HERSCHEL/SPIRE-350"
        << "ESAVO/P/HERSCHEL/SPIRE-500"
        << "astron.nl/P/lotss_dr2_high"
        << "astron.nl/P/tgssadr"
        << "erosita/dr1/rate/023"
        << "erosita/dr1/rate/024"
        << "ov-gso/P/BAT/14-20keV"
        << "ov-gso/P/BAT/150-195keV"
        << "ov-gso/P/BAT/35-50keV"
        << "ov-gso/P/CGPS/VGPS"
        << "ov-gso/P/Fermi/Band2"
        << "ov-gso/P/GLEAM/072-103"
        << "ov-gso/P/GLEAM/103-134"
        << "ov-gso/P/GLEAM/139-170"
        << "ov-gso/P/GLEAM/170-231"
        << "ov-gso/P/GLIMPSE/irac1"
        << "ov-gso/P/GLIMPSE/irac2"
        << "ov-gso/P/GLIMPSE/irac3"
        << "ov-gso/P/GLIMPSE/irac4";

    auto completer = new QCompleter(els, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->comboHiPS->addItems(els);
    ui->comboHiPS->setCompleter(completer);

    QObject::connect(ui->pushButton, &QPushButton::clicked, this, &HiPS2FITSForm::sendQuery);
}

HiPS2FITSForm::~HiPS2FITSForm()
{
    delete ui;
}

void HiPS2FITSForm::sendQuery()
{
    QString hips = ui->comboHiPS->currentText();

    QUrl url(this->endpoint);
    QUrlQuery params;
    params.addQueryItem("hips", hips);
    params.addQueryItem("width", ui->lineWidth->text());
    params.addQueryItem("height", ui->lineHeight->text());
    params.addQueryItem("ra", ui->lineRA->text());
    params.addQueryItem("dec", ui->lineDec->text());
    params.addQueryItem("projection", ui->comboProjection->currentText().left(3));
    params.addQueryItem("fov", ui->lineFoV->text());
    params.addQueryItem("rotation_angle", ui->lineRotationAngle->text());
    params.addQueryItem("coordsys", ui->comboSystem->currentText());
    params.addQueryItem("format", ui->comboFormat->currentText());

    if (ui->comboFormat->currentIndex() > 0) {
        params.addQueryItem("min_cut", ui->lineMinCut->text() + "%");
        params.addQueryItem("max_cut", ui->lineMaxCut->text() + "%");
        params.addQueryItem("stretch", ui->comboStretch->currentText());
    }

    url.setQuery(params);

    auto loading = new LoadingWidget;
    loading->setText("Sending request...");

    QNetworkRequest req(url);
    auto reply = this->nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, loading, hips]() {
        reply->deleteLater();
        loading->deleteLater();

        if (reply->error()) {
            QString msg =
                    QJsonDocument::fromJson(reply->readAll()).object()["description"].toString();
            if (msg.isEmpty()) {
                msg = reply->errorString();
            }

            QMessageBox::critical(this, "Error", msg);
            return;
        }

        QDir dir = QDir::home().absoluteFilePath("VisIVODesktopTemp/tmp_download");
        QString filename =
                QString("cutout-%1.%2")
                        .arg(QString(hips).replace("/", "_"), ui->comboFormat->currentText());
        QString filepath = dir.absoluteFilePath(filename);

        QFile f(filepath);
        if (!f.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this, "Error", f.errorString());
            return;
        }

        f.write(reply->readAll());
        f.close();
        auto res = QMessageBox::information(
                this, "Success", "File saved in " + filepath + ".\nWould you like to open it?",
                QMessageBox::Yes | QMessageBox::No);
        if (res == QMessageBox::Yes) {
            this->openFile(filepath);
        }
    });

    loading->setLoadingProcess(reply);
    loading->show();
}

void HiPS2FITSForm::openFile(const QString &filepath)
{
    if (filepath.endsWith(".png") || filepath.endsWith(".jpg")) {
        // Open it with system's image viewer
        QDesktopServices::openUrl(QUrl::fromLocalFile(filepath));
        return;
    }

    // File is in FITS format
    auto fits = vtkSmartPointer<vtkFitsReader>::New();
    fits->SetFileName(filepath.toStdString());
    auto win = new vtkwindow_new(nullptr, fits);
    win->show();
    win->activateWindow();
    win->raise();
}

void HiPS2FITSForm::on_comboFormat_currentIndexChanged(int index)
{
    bool isImage = index > 0;
    ui->lineMinCut->setEnabled(isImage);
    ui->lineMaxCut->setEnabled(isImage);
    ui->comboStretch->setEnabled(isImage);
}
