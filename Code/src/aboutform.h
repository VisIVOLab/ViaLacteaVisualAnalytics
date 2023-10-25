#ifndef ABOUTFORM_H
#define ABOUTFORM_H

#include <QUrl>
#include <QWidget>

namespace Ui {
class AboutForm;
}

class AboutForm : public QWidget
{
    Q_OBJECT

public:
    explicit AboutForm(QWidget *parent = 0);
    ~AboutForm();

private slots:
    void on_btnNeanias_clicked();
    void on_btnCirasa_clicked();
    void on_btnEcogal_clicked();

private:
    Ui::AboutForm *ui;
    QUrl neaniasUrl;
    QUrl cirasaUrl;
    QUrl ecogalUrl;
};

#endif // ABOUTFORM_H
