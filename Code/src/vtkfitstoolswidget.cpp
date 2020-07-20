#include "vtkfitstoolswidget.h"
#include "ui_vtkfitstoolswidget.h"
#include <vtkCallbackCommand.h>
#include "vtkInteractorStyleRubberBand2D.h"
#include <QSlider>
#include <QCheckBox>
#include <QSignalMapper>
#include "vialactea_fileload.h"
#include <QFileDialog>
#include <QColorDialog>
#include <QShortcut>
#include <QDebug>
#include "singleton.h"

#include "dbquery.h"

static void SelectionChangedCallbackFunction (
        vtkObject* caller,
        long unsigned int eventId,
        void* clientData,
        void* callData );


vtkfitstoolswidget::vtkfitstoolswidget(vtkwindow_new *v, QWidget *parent) : QWidget(parent), ui(new Ui::vtkfitstoolswidget)
{
    ui->setupUi(this);

    vtkwin=v;

    QHeaderView* header = ui->addedSourcesListWidget->horizontalHeader();
    header->setVisible(true);
    header->sectionResizeMode(QHeaderView::Stretch);

    updateList();

    QShortcut *shortcut_local = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_L) , this);
    QObject::connect(shortcut_local, SIGNAL(activated()), this, SLOT(on_addLocalSourcesPushButton_clicked()));

    QShortcut *shortcut_remote = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_R) , this);
    QObject::connect(shortcut_remote, SIGNAL(activated()), this, SLOT(on_addRemoteSourcesPushButton_clicked()));

    /*
    QShortcut *shortcut_dataCube_remote = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_D) , this);
    QObject::connect(shortcut_dataCube_remote, SIGNAL(activated()), this, SLOT(on_datacubeButton_clicked()));
*/

    ui->addedSourcesListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect( ui->addedSourcesListWidget, SIGNAL(customContextMenuRequested(QPoint)), SLOT(customMenuRequested(QPoint)));


}

vtkfitstoolswidget::~vtkfitstoolswidget()
{
    delete ui;
}

void vtkfitstoolswidget::customMenuRequested(QPoint pos)
{


    QItemSelectionModel *select = ui->addedSourcesListWidget->selectionModel();

    if(!select->selectedRows().isEmpty())
    {
        QModelIndex index=  ui->addedSourcesListWidget->indexAt(pos);

        QMenu *menu=new QMenu(this);
        QAction *viewAct = new QAction(tr("&Delete"), this);

        //viewAct->setStatusTip(tr("Visualize selected map file"));
        connect(viewAct, SIGNAL(triggered()), this, SLOT(deleteSelected()));

        menu->addAction(viewAct);

        menu->popup(ui->addedSourcesListWidget->viewport()->mapToGlobal(pos));

    }
}

void vtkfitstoolswidget::deleteSelected()
{
    QItemSelectionModel *select = ui->addedSourcesListWidget->selectionModel();

    if(!select->selectedRows().isEmpty())
    {

        int row= select->selectedRows().first().row();
        vtkwin->removeSingleEllipse( vtkwin->getEllipseActorList().value( ui->addedSourcesListWidget->item(row, 1)->text() ) );
        ui->addedSourcesListWidget->removeRow(row);

    }
}

int vtkfitstoolswidget::getNumOfElements()
{
    return ui->addedSourcesListWidget->model()->rowCount();

}

void vtkfitstoolswidget::addToList(QString name,  vtkSmartPointer<vtkLODActor> actor)
{

    QSignalMapper* mapper = new QSignalMapper(this);
    QSignalMapper* mapper_slider = new QSignalMapper(this);



    int row= ui->addedSourcesListWidget->model()->rowCount();


    ui->addedSourcesListWidget->insertRow(row);

    QCheckBox  *cb1= new QCheckBox();

    cb1->setChecked(true);

    double r=actor->GetProperty()->GetColor()[0]*255;
    double g=actor->GetProperty()->GetColor()[1]*255;
    double b=actor->GetProperty()->GetColor()[2]*255;

    cb1->setStyleSheet("background-color: rgb("+QString::number(r)+","+QString::number(g)+" ,"+QString::number(b)+")");

    ui->addedSourcesListWidget->setCellWidget(row,0,cb1);

    connect(cb1, SIGNAL(stateChanged(int)),mapper, SLOT(map()));
    mapper->setMapping(cb1, row);


    QSlider *slider=new QSlider(Qt::Horizontal);
    sliders.append(slider);
    slider->setRange(0,255);
    slider->setValue(255);
    ui->addedSourcesListWidget->setCellWidget(row,2,slider);
    connect(slider, SIGNAL(sliderReleased()), mapper_slider, SLOT(map()));
    mapper_slider->setMapping(slider, row);

    QTableWidgetItem *item_1 = new QTableWidgetItem();
    item_1->setText(name);

    //  item_1->setTextColor(QColor(r*255, g*255, b*255));
    ui->addedSourcesListWidget->setItem(row,1,item_1);

    connect(mapper, SIGNAL(mapped(int)), this, SLOT(checkboxClicked(int)));
    connect(mapper_slider, SIGNAL(mapped(int)), this, SLOT(opacityValueChanged(int)));


}


void vtkfitstoolswidget::updateList()
{


    QSignalMapper* mapper = new QSignalMapper(this);
    QSignalMapper* mapper_slider = new QSignalMapper(this);



    QHash<QString,  vtkSmartPointer<vtkLODActor> >::iterator i;
    QHash<QString,  vtkSmartPointer<vtkLODActor> >tmp=vtkwin->getEllipseActorList();



    int row=0;

    for (i = tmp.begin(); i != tmp.end(); ++i)
    {

        ui->addedSourcesListWidget->insertRow(row);

        QCheckBox  *cb1= new QCheckBox();

        cb1->setChecked(true);


        double r=i.value()->GetProperty()->GetColor()[0]*255;
        double g=i.value()->GetProperty()->GetColor()[1]*255;
        double b=i.value()->GetProperty()->GetColor()[2]*255;


        cb1->setStyleSheet("background-color: rgb("+QString::number(r)+","+QString::number(g)+" ,"+QString::number(b)+")");

        ui->addedSourcesListWidget->setCellWidget(row,0,cb1);

        connect(cb1, SIGNAL(stateChanged(int)),mapper, SLOT(map()));
        mapper->setMapping(cb1, row);


        QSlider *slider=new QSlider(Qt::Horizontal);
        sliders.append(slider);
        slider->setRange(0,255);
        slider->setValue(255);
        ui->addedSourcesListWidget->setCellWidget(row,2,slider);
        connect(slider, SIGNAL(sliderReleased()), mapper_slider, SLOT(map()));
        mapper_slider->setMapping(slider, row);

        QTableWidgetItem *item_1 = new QTableWidgetItem();
        item_1->setText(i.key());

        ui->addedSourcesListWidget->setItem(row,1,item_1);
        row++;
    }

    connect(mapper, SIGNAL(mapped(int)), this, SLOT(checkboxClicked(int)));
    connect(mapper_slider, SIGNAL(mapped(int)), this, SLOT(opacityValueChanged(int)));

    vtkwin->update();

}


void vtkfitstoolswidget::opacityValueChanged(int o)
{
    double opacity=(double)sliders.at(o)->value()/255;
    vtkwin->getEllipseActorList().value(ui->addedSourcesListWidget->item(o, 1)->text())->GetProperty()->SetOpacity(opacity);
    vtkwin->update();
}

void vtkfitstoolswidget::checkboxClicked(int cb)
{

    if(vtkwin->getEllipseActorList().value(ui->addedSourcesListWidget->item(cb, 1)->text())->GetVisibility())
        vtkwin->getEllipseActorList().value(ui->addedSourcesListWidget->item(cb, 1)->text())->VisibilityOff();
    else
        vtkwin->getEllipseActorList().value(ui->addedSourcesListWidget->item(cb, 1)->text())->VisibilityOn();

    vtkwin->update();
}

void vtkfitstoolswidget::on_selectButton_clicked()
{

    vtkwin->setSelectionFitsViewerInteractorStyle();
    vtkwin->activateWindow();

}

void SelectionChangedCallbackFunction (vtkObject* vtkNotUsed(caller), long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData),void* callData )
{
    std::cout << "Selection changed callback" << std::endl;

    unsigned int* rect = reinterpret_cast<unsigned int*> ( callData );
    unsigned int pos1X = rect[0];
    unsigned int pos1Y = rect[1];
    unsigned int pos2X = rect[2];
    unsigned int pos2Y = rect[3];

    std::cout << "Start x: " << pos1X
              << " Start y: " << pos1Y
              << " End x: " << pos2X
              << " End y: " << pos2Y << std::endl;
}


void vtkfitstoolswidget::on_noSelectButton_clicked()
{
    vtkwin->setVtkInteractorStyleImage();
}

void vtkfitstoolswidget::on_lutComboBox_currentIndexChanged(int index)
{
    
    //changeLut(ui->lutComboBox->currentText().toStdString().c_str());

    changeLutScale(ui->lutComboBox->currentText().toStdString().c_str(), ui->scaleComboBox->currentText().toStdString().c_str());
    
}

void vtkfitstoolswidget::on_scaleComboBox_currentIndexChanged(int index)
{
    changeLutScale(ui->lutComboBox->currentText().toStdString().c_str(), ui->scaleComboBox->currentText().toStdString().c_str());
}

void vtkfitstoolswidget::changeLutScale(std::string palette, std::string scale)
{
    vtkwin->changeFitsScale(palette, scale);
}

void vtkfitstoolswidget::changeLut(std::string palette)
{
    vtkwin->changeFitsPalette(palette);
}

void vtkfitstoolswidget::on_addLocalSourcesPushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(nullptr, "Open data file", QString(), "(*.*)");
    if(fileName.isEmpty())
        return;
    Vialactea_FileLoad *fileload = new Vialactea_FileLoad(fileName,vtkwin);
    fileload->show();
}

void vtkfitstoolswidget::on_addRemoteSourcesPushButton_clicked()
{


    vtkwin->setSkyRegionSelectorInteractorStyle();
    vtkwin->activateWindow();
}

void vtkfitstoolswidget::on_galacticRadioButton_toggled(bool checked)
{
    vtkwin->changeWCS(checked);
}

void vtkfitstoolswidget::on_eclipticRadioButton_toggled(bool checked)
{
}

void vtkfitstoolswidget::on_addedSourcesListWidget_doubleClicked(const QModelIndex &index)
{

    if(index.column()==0)
    {

        //Initial color
        double r=vtkwin->getEllipseActorList().value(ui->addedSourcesListWidget->item( index.row(), 1)->text())->GetProperty()->GetColor()[0]*255;
        double g=vtkwin->getEllipseActorList().value(ui->addedSourcesListWidget->item(index.row() , 1)->text())->GetProperty()->GetColor()[1]*255;
        double b=vtkwin->getEllipseActorList().value(ui->addedSourcesListWidget->item( index.row() , 1)->text())->GetProperty()->GetColor()[2]*255;

        QColor color = QColorDialog::getColor(QColor(r,g,b), this);

        vtkwin->getEllipseActorList().value(ui->addedSourcesListWidget->item( index.row(), 1)->text())->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
        vtkwin->update();

        //update color on table
        ui->addedSourcesListWidget->cellWidget(index.row(),0)->setStyleSheet("background-color: rgb("+QString::number(color.redF()*255)+","+QString::number(color.greenF()*255)+" ,"+QString::number(color.blueF()*255)+")");


    }
}


void vtkfitstoolswidget::on_datacubeButton_clicked()
{
    qDebug()<<"datacube";
    dbquery *queryWindow = &Singleton<dbquery>::Instance();
    queryWindow->show();

}


