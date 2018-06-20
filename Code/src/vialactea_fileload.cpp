#include "vialactea_fileload.h"
#include "ui_vialactea_fileload.h"
#include "mainwindow.h"
#include "singleton.h"
//#include "vialactea.h"
#include <QDebug>
#include "astroutils.h"
#include <QSettings>

Vialactea_FileLoad::Vialactea_FileLoad(QString f, bool silent, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Vialactea_FileLoad)
{
    ui->setupUi(this);




    silentLoad=silent;

    connect( ui->bandmergedradioButton, SIGNAL( toggled(bool) ), this, SLOT( on_groupBox_toggled(bool) ) );
    connect( ui->mapRadioButton, SIGNAL( toggled(bool) ), this, SLOT( on_groupBox_toggled(bool) ) );
    connect( ui->catalogueRadioButton, SIGNAL( toggled(bool) ), this, SLOT( on_groupBox_toggled(bool) ) );

    init(f);

}

Vialactea_FileLoad::Vialactea_FileLoad(QString f, vtkwindow_new *v,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Vialactea_FileLoad)
{
    ui->setupUi(this);

    vtkwin=v;
    init(f);
}




void Vialactea_FileLoad::init(QString f)
{
    filename=f;

    ui->filenameLabel->setText(filename);

    if(filename.endsWith(".fits",Qt::CaseInsensitive))
    {
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



    if(ui->mapRadioButton->isChecked())
    {
        /*

        vtkSmartPointer<vtkFitsReader> fitsReader = vtkSmartPointer<vtkFitsReader>::New();
        fitsReader->SetFileName(filename.toLocal8Bit().data());
        QFileInfo infoFile = QFileInfo(filename);

        ViaLactea *vialacteaWindow = &Singleton<ViaLactea>::Instance();

        // Modify the TreeModel
        int rows = vialacteaWindow->getTreeModel()->rowCount();
        vialacteaWindow->getTreeModel()->insertRow(rows);
        vialacteaWindow->getTreeModel()->setFITSIMG(vialacteaWindow->getTreeModel()->index(rows,0),fitsReader);
        vialacteaWindow->getTreeModel()->setData(vialacteaWindow->getTreeModel()->index(rows,0),QIcon( ":/icons/VME_IMAGE.bmp" ));
        vialacteaWindow->getTreeModel()->setData(vialacteaWindow->getTreeModel()->index(rows,1),infoFile.fileName());
        // end: Modify the TreeModel


        this->close();
*/
    }
    else if(ui->catalogueRadioButton->isChecked())
    {
        qDebug()<<"catalogue "<<silentLoad;
        MainWindow *w = &Singleton<MainWindow>::Instance();
        w->importAscii( filename,ui->waveLineEdit->text(),true,false,vtkwin );
        if(!silentLoad)
            this->close();

    }

    else if(ui->bandmergedradioButton->isChecked())
    {
        importBandMerged();
    }
}

void Vialactea_FileLoad::importBandMerged()
{
    MainWindow *w = &Singleton<MainWindow>::Instance();
    w->importAscii(filename,"",true,true,vtkwin);
    this->close();
}

void Vialactea_FileLoad::importFilaments()
{
    MainWindow *w = &Singleton<MainWindow>::Instance();
    w->importAsciiFilaments(filename,vtkwin);
    // w->importAscii(filename,"",true,true,vtkwin);
    this->close();
}

void Vialactea_FileLoad::importBubbles()
{
    MainWindow *w = &Singleton<MainWindow>::Instance();
    w->importAsciiBubbles(filename,vtkwin);
    this->close();
}

void Vialactea_FileLoad::import3dSelection()
{
    MainWindow *w = &Singleton<MainWindow>::Instance();
    w->importAscii3dSelection(filename,vtkwin);
    // w->importAscii(filename,"",true,true,vtkwin);
    this->close();
}

void Vialactea_FileLoad::on_groupBox_toggled(bool arg1)
{
    if(ui->catalogueRadioButton->isChecked())
    {

        ui->waveLabel->setVisible(true);
        ui->waveLineEdit->setVisible(true);
    }
    else
    {
        ui->waveLabel->setVisible(false);
        ui->waveLineEdit->setVisible(false);
    }

}
