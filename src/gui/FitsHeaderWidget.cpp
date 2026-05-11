#include "FitsHeaderWidget.h"
#include "ui_FitsHeaderWidget.h"

#include <fitsio.h>

using namespace Qt::StringLiterals;

FitsHeaderWidget::FitsHeaderWidget(QWidget *parent) : QWidget(parent), ui(new Ui::FitsHeaderWidget)
{
    ui->setupUi(this);
}

FitsHeaderWidget::~FitsHeaderWidget()
{
    delete ui;
}

void FitsHeaderWidget::showHeader(const QString &filepath)
{
    fitsfile *fptr;
    int status = 0;

    if (fits_open_image(&fptr, filepath.toStdString().c_str(), READONLY, &status)) {
        ui->textHeader->clear();
        return;
    }

    char headerBuffer[FLEN_CARD];
    QString header = u"<pre>"_s;

    int nKeys = 0;
    fits_get_hdrspace(fptr, &nKeys, nullptr, &status);

    for (int i = 1; i <= nKeys; ++i) {
        fits_read_record(fptr, i, headerBuffer, &status);

        const auto card = QString::fromUtf8(headerBuffer).leftJustified(FLEN_CARD);
        if (!card.isEmpty()) {
            header.append(highlightKeyword(card));
        }
        header.append("<br/>"_L1);
    }
    header.append("</pre>"_L1);

    fits_close_file(fptr, &status);
    ui->textHeader->setHtml(header);
}

QString FitsHeaderWidget::highlightKeyword(const QString &card, const QColor &color)
{
    return u"<span style='color:%2'>%1</span>%3"_s.arg(card.first(8).toHtmlEscaped(), color.name(),
                                                       card.sliced(8).toHtmlEscaped());
}
