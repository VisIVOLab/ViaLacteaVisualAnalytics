#ifndef SIMCOLLAPSEDIALOG_H
#define SIMCOLLAPSEDIALOG_H

#include <QDialog>

namespace Ui {
class SimCollapseDialog;
}

class SimCollapseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SimCollapseDialog(double *angles, QWidget *parent = nullptr);
    ~SimCollapseDialog();

signals:
    void dialogSubmitted(double scale);

private slots:
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();

private:
    Ui::SimCollapseDialog *ui;
};

#endif // SIMCOLLAPSEDIALOG_H
