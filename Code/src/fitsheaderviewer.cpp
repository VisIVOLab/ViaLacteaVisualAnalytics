#include "fitsheaderviewer.h"
#include "ui_fitsheaderviewer.h"
#include <QMessageBox>

FitsHeaderViewer::FitsHeaderViewer(const QString &fitsFilePath, QWidget *parent)
: QWidget(parent),
    ui(new Ui::FitsHeaderViewer)
{
    ui->setupUi(this);
    fitsfile *fitsFile;
       int status = 0;
       char errorBuffer[FLEN_ERRMSG];

       if (fits_open_file(&fitsFile, fitsFilePath.toStdString().c_str(), READONLY, &status)) {
           fits_get_errstatus(status, errorBuffer);
           QMessageBox::critical(this, "Errore", QString("Impossibile aprire il file FITS: %1").arg(errorBuffer));
           return;
       }

       char headerBuffer[80];
       QString headerText;

       int nKeys;
       fits_get_hdrspace(fitsFile, &nKeys, nullptr, &status);

       for (int i = 1; i <= nKeys; ++i) {
           fits_read_record(fitsFile, i, headerBuffer, &status);
           if (status) {
               fits_get_errstatus(status, errorBuffer);
               QMessageBox::critical(this, "Errore", QString("Errore durante la lettura dell'header: %1").arg(errorBuffer));
               break;
           }
           headerText += headerBuffer;
           headerText += '\n';
       }

       fits_close_file(fitsFile, &status);

       if (status) {
           fits_get_errstatus(status, errorBuffer);
           QMessageBox::critical(this, "Errore", QString("Errore durante la chiusura del file FITS: %1").arg(errorBuffer));
           return;
       }

       ui->headerText->setPlainText(headerText);
}

FitsHeaderViewer::~FitsHeaderViewer()
{
    delete ui;
}
