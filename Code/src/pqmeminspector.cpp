#include "pqmeminspector.h"
#include "ui_pqmeminspector.h"

pqMemInspector::pqMemInspector(QWidget *parent) :
      QDialog(parent),
      ui(new Ui::pqMemInspector)
{
    ui->setupUi(this);
    this->setMinimumSize(200, 500);
    this->memWidget = new pqMemoryInspectorPanel(this);
    if (memWidget->Initialized()){
        memWidget->setAttribute(Qt::WA_Hover);
        memWidget->adjustSize();
        memWidget->activateWindow();
        this->ui->mainLayout->addWidget(memWidget);
    }
}

pqMemInspector::~pqMemInspector()
{
    delete ui;
}
