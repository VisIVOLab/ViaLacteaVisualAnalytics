#ifndef FITSHEADERMODIFIERDIALOG_H
#define FITSHEADERMODIFIERDIALOG_H

#include <QDialog>

#include <fitsio.h>

namespace Ui {
class FitsHeaderModifierDialog;
}

class FitsHeaderModifierDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FitsHeaderModifierDialog(const QString &filepath, const QStringList &missingKeywords,
                                      QWidget *parent = nullptr);
    ~FitsHeaderModifierDialog();

    void accept() override;

private:
    Ui::FitsHeaderModifierDialog *ui;
    QString filepath;

    void writeStringKeys(fitsfile *fptr, const QString &rootKey, const QStringList &values);
    void writeFloatKeys(fitsfile *fptr, const QString &rootKey, const QList<float> &values);
};

#endif // FITSHEADERMODIFIERDIALOG_H
