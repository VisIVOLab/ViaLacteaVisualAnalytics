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
};

#endif // FILTERFITSDIALOG_H
