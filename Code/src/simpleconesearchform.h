#ifndef SIMPLECONESEARCHFORM_H
#define SIMPLECONESEARCHFORM_H

#include <QWidget>

namespace Ui {
class SimpleConeSearchForm;
}

class SimpleConeSearchForm : public QWidget
{
    Q_OBJECT

public:
    explicit SimpleConeSearchForm(QWidget *parent = nullptr);
    ~SimpleConeSearchForm();

private slots:
    void search();
    void requestCutout();

private:
    Ui::SimpleConeSearchForm *ui;
    QString pythonExe;
    int idxAccessURL;

    bool checkInputs();
    void updateTable(const QString &filepath);
};

#endif // SIMPLECONESEARCHFORM_H
