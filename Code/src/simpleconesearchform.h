#ifndef SIMPLECONESEARCHFORM_H
#define SIMPLECONESEARCHFORM_H

#include <QWidget>

namespace Ui {
class SimpleConeSearchForm;
}

namespace pybind11 {
class dict;
}

class SimpleConeSearchForm : public QWidget
{
    Q_OBJECT

public:
    explicit SimpleConeSearchForm(QWidget *parent = nullptr);
    ~SimpleConeSearchForm();

private slots:
    void search();

private:
    Ui::SimpleConeSearchForm *ui;

    bool checkInputs();
    void updateTable(const pybind11::dict &dict);
};

#endif // SIMPLECONESEARCHFORM_H
