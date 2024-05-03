#include "fitsheaderviewer.h"
#include "ui_fitsheaderviewer.h"

#include <fitsio.h>

#include <QMessageBox>

FitsHeaderViewer::FitsHeaderViewer(QWidget *parent) : QWidget(parent), ui(new Ui::FitsHeaderViewer)
{
    ui->setupUi(this);
}

FitsHeaderViewer::~FitsHeaderViewer()
{
    delete ui;
}

void FitsHeaderViewer::showHeader(const QString &filepath)
{
    fitsfile *fitsFile;
    int status = 0;
    char errorBuffer[FLEN_ERRMSG];

    if (fits_open_file(&fitsFile, filepath.toStdString().c_str(), READONLY, &status)) {
        fits_get_errstatus(status, errorBuffer);
        QMessageBox::critical(this, "Errore",
                              QString("Impossibile aprire il file FITS: %1").arg(errorBuffer));
        return;
    }

    char headerBuffer[FLEN_CARD];
    QString headerText = "<pre>";

    int nKeys;
    fits_get_hdrspace(fitsFile, &nKeys, nullptr, &status);

    for (int i = 1; i <= nKeys; ++i) {
        fits_read_record(fitsFile, i, headerBuffer, &status);
        if (status) {
            fits_get_errstatus(status, errorBuffer);
            QMessageBox::critical(
                    this, "Errore",
                    QString("Errore durante la lettura dell'header: %1").arg(errorBuffer));
            break;
        }

        QString card(headerBuffer);
        if (!card.simplified().isEmpty()) {
            headerText += highlightKeyword(card);
            headerText += "<br/>";
        }
    }
    headerText += "</pre>";

    fits_close_file(fitsFile, &status);

    if (status) {
        fits_get_errstatus(status, errorBuffer);
        QMessageBox::critical(
                this, "Errore",
                QString("Errore durante la chiusura del file FITS: %1").arg(errorBuffer));
        return;
    }

    ui->headerText->setHtml(headerText);
}

QString FitsHeaderViewer::highlightKeyword(QString card) const
{
    QString colored = "<span style='color:blue;'>" + card.left(8) + "</span>";
    return card.replace(0, 8, colored);
}
