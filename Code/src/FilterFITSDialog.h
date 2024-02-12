#ifndef FILTERFITSDIALOG_H
#define FILTERFITSDIALOG_H

#include <QDialog>

namespace Ui {
class FilterFITSDialog;
}

class FilterFITSDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilterFITSDialog(const QString &input, QWidget *parent = nullptr);
    ~FilterFITSDialog();

public slots:
    void accept() override;

private:
    Ui::FilterFITSDialog *ui;
    QString inputPath;

    QString outputFileName(const QString &inputFilepath) const;
    void openOutputFile(const QString &filepath);
};

#endif // FILTERFITSDIALOG_H
