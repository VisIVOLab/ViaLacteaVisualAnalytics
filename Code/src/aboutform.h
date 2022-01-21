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
    void on_neaniasLogo_clicked();

private:
    Ui::AboutForm *ui;
    QUrl neaniasUrl;
};

#endif // ABOUTFORM_H
