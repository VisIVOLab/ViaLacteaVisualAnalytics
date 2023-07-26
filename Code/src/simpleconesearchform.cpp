#include <pybind11/embed.h>

#include "simpleconesearchform.h"
#include "ui_simpleconesearchform.h"

#include <QDebug>
#include <QMessageBox>
#include <QUrl>

#include <sstream>

namespace py = pybind11;

SimpleConeSearchForm::SimpleConeSearchForm(QWidget *parent)
    : QWidget(parent, Qt::Window), ui(new Ui::SimpleConeSearchForm)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(ui->btnSearch, &QPushButton::clicked, this, &SimpleConeSearchForm::search);
}

SimpleConeSearchForm::~SimpleConeSearchForm()
{
    delete ui;
}

void SimpleConeSearchForm::search()
{
    if (!checkInputs()) {
        return;
    }

    std::string url = ui->textURL->text().toStdString();
    float ra = ui->textRA->text().toFloat();
    float dec = ui->textDec->text().toFloat();
    float radius = ui->textRadius->text().toFloat();
    int verbosity = ui->comboVerbosity->currentIndex() + 1;

    try {
        py::module_ scs = py::module_::import("pyvo_scs");
        py::dict res = scs.attr("search")(url, ra, dec, radius, verbosity);
        if (res["exit_code"].cast<int>() != 0) {
            throw std::runtime_error(res["error"].cast<std::string>().c_str());
        }

        updateTable(res["result"]);
    } catch (const std::exception &e) {
        QMessageBox::critical(this, QString(), e.what());
    }
}

bool SimpleConeSearchForm::checkInputs()
{
    if (!QUrl(ui->textURL->text()).isValid()) {
        QMessageBox::critical(this, QString(), "Cone URL is not a valid URL!");
        return false;
    }

    bool ok;
    ui->textRA->text().toFloat(&ok);
    if (!ok) {
        QMessageBox::critical(this, QString(), "RA value is not valid!");
        return false;
    }

    ui->textDec->text().toFloat(&ok);
    if (!ok) {
        QMessageBox::critical(this, QString(), "Dec value is not valid!");
        return false;
    }

    ui->textRadius->text().toFloat(&ok);
    if (!ok) {
        QMessageBox::critical(this, QString(), "Radius value is not valid!");
        return false;
    }

    return true;
}

void SimpleConeSearchForm::updateTable(const pybind11::dict &dict)
{
    const py::list &columns = dict["columns"];
    ui->table->setColumnCount(columns.size());
    for (int i = 0; i < columns.size(); ++i) {
        const py::tuple &col = columns[i];
        auto item = new QTableWidgetItem;
        item->setText(QString::fromStdString(col[0].cast<std::string>()));
        item->setToolTip(QString::fromStdString(col[1].cast<std::string>()));
        ui->table->setHorizontalHeaderItem(i, item);
    }

    const py::list &table = dict["table"];
    ui->table->setRowCount(table.size());
    for (int i = 0; i < table.size(); ++i) {
        const py::list &row = table[i];
        for (int j = 0; j < columns.size(); ++j) {
            ui->table->setItem(
                    i, j, new QTableWidgetItem(QString::fromStdString(row[j].cast<std::string>())));
        }
    }
}
