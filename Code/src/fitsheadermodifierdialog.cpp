#include "fitsheadermodifierdialog.h"
#include "ui_fitsheadermodifierdialog.h"

#include <fitsio.h>

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

#include <stdexcept>

FitsHeaderModifierDialog::FitsHeaderModifierDialog(const QString &filepath,
                                                   const QStringList &missingKeywords,
                                                   QWidget *parent)
    : QDialog(parent), ui(new Ui::FitsHeaderModifierDialog), filepath(filepath)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    if (!missingKeywords.contains("CUNIT"))
        ui->boxCUNIT->hide();

    if (!missingKeywords.contains("CTYPE"))
        ui->boxCTYPE->hide();

    if (!missingKeywords.contains("CDELT"))
        ui->boxCDELT->hide();

    this->adjustSize();
}

FitsHeaderModifierDialog::~FitsHeaderModifierDialog()
{
    delete ui;
}

void FitsHeaderModifierDialog::accept()
{
    QString newFile =
            QFileDialog::getSaveFileName(this, QString(), QString(), "FITS files (*.fits)");
    if (newFile.isEmpty()) {
        QDialog::accept();
        return;
    }

    // QFile::copy does not overwrite the file, so we manually delete it before the copy
    if (!QFile::exists(newFile)) {
        QFile::remove(newFile);
    }
    QFile::copy(filepath, newFile);

    fitsfile *fptr;
    int status = 0;
    char errmsg[FLEN_ERRMSG];

    if (fits_open_data(&fptr, newFile.toUtf8().data(), READWRITE, &status)) {
        fits_get_errstatus(status, errmsg);
        fits_report_error(stderr, status);
        QMessageBox::critical(this, "Error", QString::fromUtf8(errmsg));
        QDialog::accept();
        return;
    }

    try {
        if (ui->boxCTYPE->isVisible()) {
            writeStringKeys(
                    fptr, "CTYPE",
                    { ui->textCTYPE1->text(), ui->textCTYPE2->text(), ui->textCTYPE3->text() });
        }

        if (ui->boxCUNIT->isVisible()) {
            writeStringKeys(
                    fptr, "CUNIT",
                    { ui->textCUNIT1->text(), ui->textCUNIT2->text(), ui->textCUNIT3->text() });
        }

        if (ui->boxCDELT->isVisible()) {
            writeFloatKeys(fptr, "CDELT",
                           { ui->textCDELT1->text().toFloat(), ui->textCDELT2->text().toFloat(),
                             ui->textCDELT3->text().toFloat() });
        }

        QMessageBox::information(this, "Success", "File has been saved!");
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error", QString::fromUtf8(e.what()));
    }

    fits_close_file(fptr, &status);
    QDialog::accept();
}

void FitsHeaderModifierDialog::writeStringKeys(fitsfile *fptr, const QString &rootKey,
                                               const QStringList &values)
{
    // fptr should be already open
    int status = 0;
    char errmsg[FLEN_ERRMSG];

    for (int i = 1; i <= values.size(); ++i) {
        QString key = rootKey + QString::number(i);

        if (fits_update_key_str(fptr, key.toUtf8().data(), values.at(i - 1).toUtf8().data(), NULL,
                                &status)) {
            fits_report_error(stderr, status);
            fits_get_errstatus(status, errmsg);
            throw std::runtime_error(errmsg);
        }
    }
}

void FitsHeaderModifierDialog::writeFloatKeys(fitsfile *fptr, const QString &rootKey,
                                              const QList<float> &values)
{
    // fptr should be already open
    int status = 0;
    char errmsg[FLEN_ERRMSG];

    for (int i = 1; i <= values.size(); ++i) {
        QString key = rootKey + QString::number(i);

        if (fits_update_key_flt(fptr, key.toUtf8().data(), values.at(i - 1), -9, NULL, &status)) {
            fits_report_error(stderr, status);
            fits_get_errstatus(status, errmsg);
            throw std::runtime_error(errmsg);
        }
    }
}
