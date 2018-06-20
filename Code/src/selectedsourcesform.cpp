#include "selectedsourcesform.h"
#include "ui_selectedsourcesform.h"

SelectedSourcesForm::SelectedSourcesForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectedSourcesForm)
{
    ui->setupUi(this);
}

SelectedSourcesForm::~SelectedSourcesForm()
{
    delete ui;
}
