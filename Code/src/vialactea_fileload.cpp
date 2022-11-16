#include "vialactea_fileload.h"
#include "ui_vialactea_fileload.h"

#include "mainwindow.h"
#include "singleton.h"
//#include "vialactea.h"
#include "astroutils.h"
#include <QDebug>
#include <QSettings>

Vialactea_FileLoad::Vialactea_FileLoad(QString f, bool silent, QWidget *parent)
    : QWidget(parent), ui(new Ui::Vialactea_FileLoad)
{
    ui->setupUi(this);

    silentLoad = silent;

    connect(ui->bandmergedradioButton, SIGNAL(toggled(bool)), this,
            SLOT(on_groupBox_toggled(bool)));
    connect(ui->mapRadioButton, SIGNAL(toggled(bool)), this, SLOT(on_groupBox_toggled(bool)));
    connect(ui->catalogueRadioButton, SIGNAL(toggled(bool)), this, SLOT(on_groupBox_toggled(bool)));

    init(f);
}

Vialactea_FileLoad::Vialactea_FileLoad(QString f, vtkwindow_new *v, QWidget *parent)
    : QWidget(parent), ui(new Ui::Vialactea_FileLoad)
{
    ui->setupUi(this);
    vtkwin = v;
    init(f);
}

void Vialactea_FileLoad::init(QString f)
{
    filename = f;
    ui->filenameLabel->setText(filename);
    if (filename.endsWith(".fits", Qt::CaseInsensitive)) {
        ui->mapRadioButton->setChecked(true);
        ui->waveLabel->setVisible(false);
        ui->waveLineEdit->setVisible(false);
    }
}

Vialactea_FileLoad::~Vialactea_FileLoad()
{
    delete ui;
}

void Vialactea_FileLoad::setWavelength(QString w)
{
    ui->waveLineEdit->setText(w);
}

void Vialactea_FileLoad::setCatalogueActive()
{
    ui->catalogueRadioButton->setChecked(true);
}

void Vialactea_FileLoad::on_okPushButton_clicked()
{
    if (ui->mapRadioButton->isChecked()) {

    } else if (ui->catalogueRadioButton->isChecked()) {
        MainWindow *w = &Singleton<MainWindow>::Instance();
        w->importAscii(filename, ui->waveLineEdit->text(), true, false, vtkwin);
        if (!silentLoad)
            this->close();
    }

    else if (ui->bandmergedradioButton->isChecked()) {
        importBandMerged();
    }
}

void Vialactea_FileLoad::importBandMerged()
{
    MainWindow *w = &Singleton<MainWindow>::Instance();
    w->importAscii(filename, "", true, true, vtkwin);
    this->close();
}

void Vialactea_FileLoad::importFilaments()
{
    MainWindow *w = &Singleton<MainWindow>::Instance();
    w->importAsciiFilaments(filename, vtkwin);
    this->close();
}

void Vialactea_FileLoad::importBubbles()
{
    MainWindow *w = &Singleton<MainWindow>::Instance();
    w->importAsciiBubbles(filename, vtkwin);
    this->close();
}

void Vialactea_FileLoad::import3dSelection()
{
    MainWindow *w = &Singleton<MainWindow>::Instance();
    w->importAscii3dSelection(filename, vtkwin);
    this->close();
}

void Vialactea_FileLoad::on_groupBox_toggled(bool arg1)
{
    if (ui->catalogueRadioButton->isChecked()) {
        ui->waveLabel->setVisible(true);
        ui->waveLineEdit->setVisible(true);
    } else {
        ui->waveLabel->setVisible(false);
        ui->waveLineEdit->setVisible(false);
    }
}
