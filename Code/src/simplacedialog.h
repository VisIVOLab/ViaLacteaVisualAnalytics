#ifndef SIMPLACEDIALOG_H
#define SIMPLACEDIALOG_H

#include <QDialog>

namespace Ui {
class SimPlaceDialog;
}

class SimPlaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SimPlaceDialog(QWidget *parent = nullptr);
    ~SimPlaceDialog();

signals:
    void dialogSubmitted(double lon, double lat, double distance);

private slots:
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();

private:
    Ui::SimPlaceDialog *ui;
};

#endif // SIMPLACEDIALOG_H
