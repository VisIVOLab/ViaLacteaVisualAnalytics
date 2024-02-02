#include "FilterFITSDialog.h"
#include "ui_FilterFITSDialog.h"

#include "fitswcs.h"
#include "imutils.h"

#include <QDir>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QMessageBox>

FilterFITSDialog::FilterFITSDialog(const QString &input, QWidget *parent)
    : QDialog(parent), ui(new Ui::FilterFITSDialog), inputPath(input)
{
    ui->setupUi(this);
    ui->lineFWHM->setValidator(new QDoubleValidator(ui->lineFWHM));
    ui->lineFactor->setValidator(new QIntValidator(ui->lineFactor));

    struct WorldCoor *wcs = GetWCSFITS(input.toUtf8().data(), 0);
    ui->labelDim->setText(QString("%1x%2").arg(wcs->nxpix).arg(wcs->nypix));
    ui->labelRes->setText(QString::number(std::max(wcs->cdelt[0], wcs->cdelt[1])));
    wcsfree(wcs);
}

FilterFITSDialog::~FilterFITSDialog()
{
    delete ui;
}

void FilterFITSDialog::accept()
{
    if (!ui->checkFilter->isChecked() && !ui->checkResize->isChecked()) {
        QMessageBox::warning(this, QString(), "Select at least one action.");
        return;
    }

    QString inputFilepath(inputPath);
    const QString outDir = QDir::home().absoluteFilePath("VisIVODesktopTemp/tmp_download");
    const QString outFile = QFileInfo(inputFilepath).baseName() + "_filtered.fits";
    const QString outputFilePath = QDir(outDir).absoluteFilePath(outFile);

    if (ui->checkFilter->isChecked()) {
        const double fwhm = ui->lineFWHM->text().toDouble();
        const double sigma = fwhm / 2.355;
        if (!fits_smooth(inputFilepath.toStdString(), sigma, outputFilePath.toStdString())) {
            QMessageBox::critical(this, QString(), "Could not filter input file!");
            return;
        }
        inputFilepath = outputFilePath;
    }

    if (ui->checkResize->isChecked()) {
        const int factor = ui->lineFactor->text().toInt();
        if (!fits_resize(inputFilepath.toStdString(), factor, outputFilePath.toStdString())) {
            QMessageBox::critical(this, QString(), "Could not resize input file!");
            return;
        }
    }

    QMessageBox::information(this, QString(), "File saved in " + outputFilePath);
    QDialog::accept();
}
