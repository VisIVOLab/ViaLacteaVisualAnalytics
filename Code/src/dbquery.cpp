#include "dbquery.h"
#include "ui_dbquery.h"
#include "qdebug.h"
#include <QXmlStreamReader>
#include <QMessageBox>
#include <QDomElement>
#include "xmlparser.h"
#include <QStringList>
#include "QDir"
#include <QCoreApplication>
#include <QTimer>
#include "downloadmanager.h"
#include <QSignalMapper>
#include "vtkwindow_new.h"
#include "QObject"
#include <QSettings>

dbquery::dbquery(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dbquery)
{
    ui->setupUi(this);
    n_surveys=10; //+2
    n_species=11; //+1
    n_transitions=7; //+1
    loading = new LoadingWidget();

    QStringList list_surveys=(QStringList()<<"CHAMP"<<"HOPS"<<"FCRAO_GRS"<<"MALT90"<<"THRUMMS"<<"NANTEN"<<"OGS"<<"JCMT-HARP"<<"VGPS"<<"CGPS");
    QStringList list_species=(QStringList()<<"12CO"<<"13CO"<<"C18O"<<"CN"<<"H2O"<<"HCN"<<"HNC"<<"HCO+"<<"N2H+"<<"NH3"<<"HI");
    QStringList list_transitions=(QStringList()<<"1-0"<<"1(1)0a-1(1)0s"<<"1(23)-0(12)"<<"2(2)0a-2(2)0s"<<"3-2"<<"6(1,6)-5(2,3)"<<"21CM");

    ui->comboBox_surveys->addItems(list_surveys);
    ui->comboBox_species->addItems(list_species);
    ui->comboBox_transitions->addItems(list_transitions);

    //nam = new QNetworkAccessManager(this);



    QSettings settings(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp").append("/setting.ini"), QSettings::NativeFormat);
    vlkbUrl= settings.value("vlkburl", "").toString();

    parser=new xmlparser();



}

dbquery::~dbquery()
{
    delete ui;
    delete parser;
}

void dbquery::finishedSlot(QNetworkReply* reply)
{
    QString string;
    QString file;

    // Reading attributes of the reply
    // e.g. the HTTP status code
    QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    // Or the target URL if it was a redirect:
    QVariant redirectionTargetUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    if (reply->error() == QNetworkReply::NoError)
    {
        QXmlStreamReader xml(reply);

        QString url=reply->request().url().toString();


        QString subString1 = url.mid(0,62); //

        //first query (l,b,r,v_l, v_u) xml result -> looking for a PUBLISHER ID
        if( url.contains("vlkb_search") )
        {
            parser->parseXML(xml, string);

            if(string.compare("NULL")!=0)
            {
                loading->setFileName("Datacube found");

                QUrl url2 (vlkbUrl+"/vlkb_cutout?pubdid="+string+"&l="+ui->lineEdit_l->text()+
                           "&b="+ui->lineEdit_b->text()+"&r="+ui->lineEdit_b->text()+"&vl="+ui->lineEdit_vl->text()+"&vu="+ui->lineEdit_vu->text()+"&nullvals");

                nam->get(QNetworkRequest(url2));
            }
            else
            {
                loading->setFileName("Datacube inexistent");
                loading->loadingEnded();
                loading->hide();
                QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("Datacube inexistent - Try again"));
            }
        }
        //second query (PUBDID) xml result -> looking for a datacube (URL)
        if (url.contains("vlkb_cutout") )
        {
            parser->parseXML(xml, string);
            if(string.compare("NULL")!=0)
            {
                loading->setFileName("Datacube found");
                DownloadManager *manager= new DownloadManager();
                QString urlString=string.trimmed();
                QUrl url3(urlString);

                //segnale tra due oggetti:
                connect(manager, SIGNAL(downloadCompleted()),this, SLOT(on_download_completed()));

                file=manager->doDownload(url3);
                loading->loadingEnded();
                loading->hide();

                downloadedFile=file;


            }
            else
            {
                QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("Inconsistent data (PubDID vs Region only partially overlap)"));
                loading->loadingEnded();
                loading->hide();
            }

        }
    }


    else
    {
        //handle errors here
        qDebug()<<"Server is not replying";
    }
    //delete reply;
    reply->deleteLater();

}

//this method is used for the datacube extraction
void dbquery::finishedSlot2(QNetworkReply* reply)
{

    QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);



    if (reply->error() == QNetworkReply::NoError)
    {

        QList< QMap<QString,QString> > datacubes;
        QXmlStreamReader xml(reply);
        parser->datacubeExtraction(xml, datacubes);

        this->addDatacubesToUI(datacubes);

    } else
    {
        //handle errors here
        qDebug()<<"Server not replying";
    }
    //delete reply;
    this->activateWindow();
    this->setFocus();
    loading->close();
    reply->deleteLater();

    
}



void dbquery::setCoordinate(QString l,QString b)
{
    ui->lineEdit_l->setText( l );
    ui->lineEdit_b->setText( b);
    double ray=0.1;
    if(l.toDouble()>b.toDouble())
        ray=b.toDouble()/2;
    else
        ray=l.toDouble()/2;
    QString myray=QString::number(qAbs(ray));
    ui->lineEdit_r->setText( myray);
}

void dbquery::setCoordinate(char* l,char* b)
{
    ui->lineEdit_l->setText( l );
    ui->lineEdit_b->setText( b);
}



void dbquery::enableAllItems(QComboBox *comboBox, int nItems){

    QVariant v(1 | 32);
    //qDebug()<<"enabling All: "<<nItems;
    QModelIndex index;
    for(int i=0; i<nItems; i++)
    {
        index = comboBox->model()->index(i, 0);
        comboBox->model()->setData(index, v, Qt::UserRole - 1);
    }
    // comboBox->setCurrentIndex(0);
}

void dbquery::disableItems(QComboBox *comboBox, int nItem, int* indexes, int size){
    QVariant v(0);
    //qDebug()<<"Disabling: "<<nItem<<" items";
    QModelIndex index;
    for(int i=0; i<nItem; i++)
    {
        index = comboBox->model()->index(indexes[i], 0);
        comboBox->model()->setData(index, v, Qt::UserRole - 1);
    }
    //comboBox->setCurrentIndex(5);
}

void dbquery::on_comboBox_surveys_activated(const QString &arg1)
{

    if(arg1=="CHAMP")
    {
        //meno il 7
        int indexes_species[10]={0,1,2,3,4,5,6,8,9,10};
        int nItems_species=10;
        this->enableAllItems(ui->comboBox_species,n_species);
        this->disableItems(ui->comboBox_species, nItems_species, indexes_species, n_species);
        //         ui->comboBox_species->setCurrentIndex(7);


        //tranne 0
        int indexes_transition[6]={1,2,3,4,5,6};
        int nItems_transition=6;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition,n_transitions);
        //        ui->comboBox_transitions->setCurrentIndex(0);
    }

    else if(arg1=="HOPS")
    {

        //meno 4 e 9
        int indexes_species[9]={0,1,2,3,5,6,7,8,10};
        int nItems_species=9;
        this->enableAllItems(ui->comboBox_species, n_species);
        this->disableItems(ui->comboBox_species, nItems_species, indexes_species, n_species);
        //  ui->comboBox_species->setCurrentIndex(4);

        //tranne 1,3,5
        int indexes_transition[4]={0,2,4,6};
        int nItems_transition=4;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition, n_transitions);
        //      ui->comboBox_transitions->setCurrentIndex(1);

    }
    else if(arg1=="FCRAO_GRS")
    {


        //meno 1
        int indexes_species[10]={0,2,3,4,5,6,7,8,9,10};
        int nItems_species=10;
        this->enableAllItems(ui->comboBox_species, n_species);
        this->disableItems(ui->comboBox_species, nItems_species, indexes_species, n_species);
        // ui->comboBox_species->setCurrentIndex(1);
        //tranne 0
        int indexes_transition[6]={1,2,3,4,5,6};
        int nItems_transition=6;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition,n_transitions);
        // ui->comboBox_transitions->setCurrentIndex(0);
    }
    else if(arg1=="MALT90")
    {

        //meno 5 6 7 8
        int indexes_species[7]={0,1,2,3,4,9,10};
        int nItems_species=7;
        this->enableAllItems(ui->comboBox_species, n_species);
        // this->disableItems(ui->comboBox_species, nItems_species, indexes_species, n_species);
        ui->comboBox_species->setCurrentIndex(5);
        //tranne 0
        int indexes_transition[6]={1,2,3,4,5,6};
        int nItems_transition=6;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition, n_transitions);
        //ui->comboBox_transitions->setCurrentIndex(0);
    }
    else if(arg1=="THRUMMS")
    {
        //meno 0 1 2 3
        int indexes_species[7]={4,5,6,7,8,9,10};
        int nItems_species=7;
        this->enableAllItems(ui->comboBox_species, n_species);
        this->disableItems(ui->comboBox_species, nItems_species, indexes_species, n_species);
        //ui->comboBox_species->setCurrentIndex(0);
        //tranne 0 e 2
        int indexes_transition[5]={1,3,4,5,6};
        int nItems_transition=5;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition,n_transitions);
        ui->comboBox_transitions->setCurrentIndex(0);
    }
    else if(arg1=="NANTEN")
    {
        //meno 0
        int indexes_species[10]={1,2,3,4,5,6,7,8,9,10};
        int nItems_species=10;
        this->enableAllItems(ui->comboBox_species, n_species);
        this->disableItems(ui->comboBox_species, nItems_species, indexes_species, n_species);
        ui->comboBox_species->setCurrentIndex(0);
        //tranne 0
        int indexes_transition[6]={1,2,3,4,5,6};
        int nItems_transition=6;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition, n_transitions);
        // ui->comboBox_transitions->setCurrentIndex(0);
    }
    else if(arg1=="OGS")
    {
        //meno 0 1
        int indexes_species[9]={2,3,4,5,6,7,8,9,10};
        int nItems_species=9;
        this->enableAllItems(ui->comboBox_species, n_species);
        this->disableItems(ui->comboBox_species, nItems_species, indexes_species, n_species);
        //ui->comboBox_species->setCurrentIndex(0);
        //tranne 0
        int indexes_transition[6]={1,2,3,4,5,6};
        int nItems_transition=6;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition, n_transitions);
        //ui->comboBox_transitions->setCurrentIndex(0);

    }
    else if(arg1=="JCMT-HARP")
    {
        //meno 0
        int indexes_species[10]={1,2,3,4,5,6,7,8,9,10};
        int nItems_species=10;
        this->enableAllItems(ui->comboBox_species, n_species);
        this->disableItems(ui->comboBox_species, nItems_species, indexes_species, n_species);
        //ui->comboBox_species->setCurrentIndex(0);
        //tranne 4
        int indexes_transition[6]={0,1,2,3,5,6};
        int nItems_transition=6;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition, n_transitions);
        //ui->comboBox_transitions->setCurrentIndex(4);
    }

    else if(arg1=="VGPS")
    {
        //meno 10
        int indexes_species[10]={0,1,2,3,4,5,6,7,8,9};
        int nItems_species=10;
        this->enableAllItems(ui->comboBox_species, n_species);
        this->disableItems(ui->comboBox_species, nItems_species, indexes_species, n_species);
        //ui->comboBox_species->setCurrentIndex(10);
        //tranne 6
        int indexes_transition[6]={0,1,2,3,4,5};
        int nItems_transition=6;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition, n_transitions);
        //ui->comboBox_transitions->setCurrentIndex(6);
    }
    else if(arg1=="CGPS")
    {
        //meno 10
        int indexes_species[10]={0,1,2,3,4,5,6,7,8,9};
        int nItems_species=10;
        this->enableAllItems(ui->comboBox_species, n_species);
        this->disableItems(ui->comboBox_species, nItems_species, indexes_species, n_species);
        //ui->comboBox_species->setCurrentIndex(10);
        //tranne 6 di 7
        int indexes_transition[6]={0,1,2,3,4,5};
        int nItems_transition=6;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition, n_transitions);
        //ui->comboBox_transitions->setCurrentIndex(6);
    }

    this->enableAllItems(ui->comboBox_surveys, n_surveys);
}

void dbquery::on_comboBox_species_activated(const QString &arg1)
{
    if(arg1=="12CO")
    {


        //tranne 4,5,6,7 di 8
        int indexes_surveys[6]={0,1,2,3,8,9};
        int nItems_surveys=6;
        this->enableAllItems(ui->comboBox_surveys, n_surveys);
        this->disableItems(ui->comboBox_surveys, nItems_surveys, indexes_surveys, n_surveys);
        //ui->comboBox_surveys->setCurrentIndex(4);

        //meno il 0,4 di 6
        int indexes_transition[5]={1,2,3,5,6};
        int nItems_transition=5;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition, n_transitions);
        //ui->comboBox_transitions->setCurrentIndex(0);
    }
    else if(arg1=="13CO")
    {
        //tranne 2,4,6 di 8
        int indexes_surveys[7]={0,1,3,5,7,8,9};
        int nItems_surveys=7;
        this->enableAllItems(ui->comboBox_surveys, n_surveys);
        this->disableItems(ui->comboBox_surveys, nItems_surveys, indexes_surveys,n_surveys);
        //ui->comboBox_surveys->setCurrentIndex(2);

        //meno il 0 di 6
        int indexes_transition[6]={1,2,3,4,5,6};
        int nItems_transition=6;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition ,n_transitions);
        //ui->comboBox_transitions->setCurrentIndex(0);

    }
    else if(arg1=="C18O")
    {

        //tranne 4 di 8
        int indexes_surveys[9]={0,1,2,3,5,6,7,8,9};
        int nItems_surveys=9;
        this->enableAllItems(ui->comboBox_surveys, n_surveys);
        this->disableItems(ui->comboBox_surveys, nItems_surveys, indexes_surveys, n_surveys);
        //ui->comboBox_surveys->setCurrentIndex(4);

        //meno il 0 di 6
        int indexes_transition[6]={1,2,3,4,5,6};
        int nItems_transition=6;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition, n_transitions);
        //ui->comboBox_transitions->setCurrentIndex(0);

    }
    else if(arg1=="CN")
    {
        //tranne 4 di 8
        int indexes_surveys[9]={0,1,2,3,5,6,7,8,9};
        int nItems_surveys=9;
        this->enableAllItems(ui->comboBox_surveys, n_surveys);
        this->disableItems(ui->comboBox_surveys, nItems_surveys, indexes_surveys, n_surveys);
        //ui->comboBox_surveys->setCurrentIndex(4);

        //meno il 2 di 6
        int indexes_transition[6]={0,1,3,4,5,6};
        int nItems_transition=6;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition, n_transitions);
        //ui->comboBox_transitions->setCurrentIndex(2);

    }
    else if(arg1=="H2O")
    {
        //tranne 1 di 8
        int indexes_surveys[9]={0,2,3,4,5,6,7,8,9};
        int nItems_surveys=9;
        this->enableAllItems(ui->comboBox_surveys, n_surveys);
        this->disableItems(ui->comboBox_surveys, nItems_surveys, indexes_surveys, n_surveys);
        //ui->comboBox_surveys->setCurrentIndex(1);

        //meno il 5 di 6
        int indexes_transition[6]={0,1,2,3,4,6};
        int nItems_transition=6;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition, n_transitions);
        //ui->comboBox_transitions->setCurrentIndex(5);

    }
    else if(arg1=="HCN")
    {

        //tranne 3 di 8
        int indexes_surveys[9]={0,1,2,4,5,6,7,8,9};
        int nItems_surveys=9;
        this->enableAllItems(ui->comboBox_surveys, n_surveys);
        this->disableItems(ui->comboBox_surveys, nItems_surveys, indexes_surveys, n_surveys);
        //ui->comboBox_surveys->setCurrentIndex(3);

        //meno il 0 di 6
        int indexes_transition[6]={1,2,3,4,5,6};
        int nItems_transition=6;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition, n_transitions);
        //ui->comboBox_transitions->setCurrentIndex(0);

    }
    else if(arg1=="HNC")
    {
        //tranne 3 di 8
        int indexes_surveys[9]={0,1,2,4,5,6,7,8,9};
        int nItems_surveys=9;
        this->enableAllItems(ui->comboBox_surveys, n_surveys);
        this->disableItems(ui->comboBox_surveys, nItems_surveys, indexes_surveys, n_surveys);
        //ui->comboBox_surveys->setCurrentIndex(3);

        //meno il 0 di 6
        int indexes_transition[6]={1,2,3,4,5,6};
        int nItems_transition=6;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition, n_transitions);
        //ui->comboBox_transitions->setCurrentIndex(0);

    }
    else if(arg1=="HCO+")
    {

        //tranne 0,3 di 8
        int indexes_surveys[8]={1,2,4,5,6,7,8,9};
        int nItems_surveys=8;
        this->enableAllItems(ui->comboBox_surveys, n_surveys);
        this->disableItems(ui->comboBox_surveys, nItems_surveys, indexes_surveys, n_surveys);
        //ui->comboBox_surveys->setCurrentIndex(0);

        //meno il 0 di 6
        int indexes_transition[6]={1,2,3,4,5,6};
        int nItems_transition=6;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition, n_transitions);
        //ui->comboBox_transitions->setCurrentIndex(0);

    }
    else if(arg1=="N2H+")
    {
        //tranne 3 di 8
        int indexes_surveys[9]={0,1,2,4,5,6,7,8,9};
        int nItems_surveys=9;
        this->enableAllItems(ui->comboBox_surveys, n_surveys);
        this->disableItems(ui->comboBox_surveys, nItems_surveys, indexes_surveys, n_surveys);
        //ui->comboBox_surveys->setCurrentIndex(3);

        //meno il 0 di 6
        int indexes_transition[6]={1,2,3,4,5,6};
        int nItems_transition=6;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition, n_transitions);
        //ui->comboBox_transitions->setCurrentIndex(0);

    }
    else if(arg1=="NH3")
    {
        //tranne 1 di 8
        int indexes_surveys[9]={0,2,3,4,5,6,7,8,9};
        int nItems_surveys=9;
        this->enableAllItems(ui->comboBox_surveys, n_surveys);
        this->disableItems(ui->comboBox_surveys, nItems_surveys, indexes_surveys, n_surveys);
        //ui->comboBox_surveys->setCurrentIndex(1);

        //meno il 1,3 di 6
        int indexes_transition[5]={0,2,4,5,6};
        int nItems_transition=5;
        this->enableAllItems(ui->comboBox_transitions, n_transitions);
        this->disableItems(ui->comboBox_transitions, nItems_transition, indexes_transition, n_transitions);
        //ui->comboBox_transitions->setCurrentIndex(1);

    }
    this->enableAllItems(ui->comboBox_species, n_species);
}

void dbquery::on_comboBox_transitions_activated(const QString &arg1)
{
    if(arg1=="1-0")
        /*   n_surveys=10; //+2
    n_species=11; //+1
    n_transitions=7; //+1*/
    {
        //tranne 0,2,3,4,5,6,7 di 8
        int indexes_surveys[3]={1,8,9};
        int nItems_surveys=3;
        this->enableAllItems(ui->comboBox_surveys, n_surveys);
        this->disableItems(ui->comboBox_surveys, nItems_surveys, indexes_surveys, n_surveys);
        //ui->comboBox_surveys->setCurrentIndex(0);

        //meno il 0,1,2,5,6,7,8 di 10
        int indexes_species[4]={3,4,9,10};
        int nItems_species=4;
        this->enableAllItems(ui->comboBox_species, n_species);
        this->disableItems(ui->comboBox_species, nItems_species, indexes_species, n_species);

    }
    else if(arg1=="1(1)0a-1(1)0s")
    {
        //tranne 1 di 8
        int indexes_surveys[9]={0,2,3,4,5,6,7,8,9};
        int nItems_surveys=9;
        this->enableAllItems(ui->comboBox_surveys, n_surveys);
        this->disableItems(ui->comboBox_surveys, nItems_surveys, indexes_surveys, n_surveys);
        //ui->comboBox_surveys->setCurrentIndex(1);

        //meno il 9 di 10
        int indexes_species[10]={0,1,2,3,4,5,6,7,8,10};
        int nItems_species=10;
        this->enableAllItems(ui->comboBox_species, n_species);
        this->disableItems(ui->comboBox_species, nItems_species, indexes_species, n_species);
        //ui->comboBox_species->setCurrentIndex(9);
    }
    else if(arg1=="1(23)-0(12)")
    {
        //tranne 4 di 8
        int indexes_surveys[9]={0,1,2,3,5,6,7,8,9};
        int nItems_surveys=9;
        this->enableAllItems(ui->comboBox_surveys, n_surveys);
        this->disableItems(ui->comboBox_surveys, nItems_surveys, indexes_surveys, n_surveys);
        //ui->comboBox_surveys->setCurrentIndex(4);

        //meno il 3 di 10
        int indexes_species[10]={0,1,2,4,5,6,7,8,9,10};
        int nItems_species=10;
        this->enableAllItems(ui->comboBox_species, n_species);
        this->disableItems(ui->comboBox_species, nItems_species, indexes_species, n_species);
        //ui->comboBox_species->setCurrentIndex(3);

    }
    else if(arg1=="2(2)0a-2(2)0s")
    {
        //tranne 1 di 8
        int indexes_surveys[9]={0,2,3,4,5,6,7,8,9};
        int nItems_surveys=9;
        this->enableAllItems(ui->comboBox_surveys, n_surveys);
        this->disableItems(ui->comboBox_surveys, nItems_surveys, indexes_surveys, n_surveys);
        ui->comboBox_surveys->setCurrentIndex(1);
        //meno il 9 di 10
        int indexes_species[10]={0,1,2,3,4,5,6,7,8,10};
        int nItems_species=10;
        this->enableAllItems(ui->comboBox_species, n_species);
        this->disableItems(ui->comboBox_species, nItems_species, indexes_species, n_species);
        //ui->comboBox_species->setCurrentIndex(9);

    }
    else if(arg1=="3-2")
    {
        //tranne 7 di 8
        int indexes_surveys[9]={0,1,2,3,4,5,6,8,9};
        int nItems_surveys=9;
        this->enableAllItems(ui->comboBox_surveys, n_surveys);
        this->disableItems(ui->comboBox_surveys, nItems_surveys, indexes_surveys, n_surveys);
        //ui->comboBox_surveys->setCurrentIndex(7);
        //meno il 0 di 10
        int indexes_species[10]={1,2,3,4,5,6,7,8,9,10};
        int nItems_species=10;
        this->enableAllItems(ui->comboBox_species, n_species);
        this->disableItems(ui->comboBox_species, nItems_species, indexes_species, n_species);
        //ui->comboBox_species->setCurrentIndex(0);

    }
    else if(arg1=="6(1,6)-5(2,3)")
    {
        //tranne 1 di 8
        int indexes_surveys[9]={0,2,3,4,5,6,7,8,9};
        int nItems_surveys=9;
        this->enableAllItems(ui->comboBox_surveys, n_surveys);
        this->disableItems(ui->comboBox_surveys, nItems_surveys, indexes_surveys, n_surveys);
        //ui->comboBox_surveys->setCurrentIndex(1);
        //meno il 4 di 10
        int indexes_species[10]={0,1,2,3,5,6,7,8,9,10};
        int nItems_species=10;
        this->enableAllItems(ui->comboBox_species,n_species);
        this->disableItems(ui->comboBox_species, nItems_species, indexes_species, n_species);
        //ui->comboBox_species->setCurrentIndex(4);

    }

    this->enableAllItems(ui->comboBox_transitions, n_transitions);
}

void dbquery::on_queryPushButton_clicked()
{


    
    ui->lineEdit_l->setReadOnly(true);
    ui->lineEdit_b->setReadOnly(true);
    ui->lineEdit_r->setReadOnly(true);
    ui->lineEdit_vl->setReadOnly(true);
    ui->lineEdit_vu->setReadOnly(true);
    
    loading->init();
    loading->setFileName("Connecting to the cutout service");
    loading ->show();
    loading->activateWindow();
    
    survey=ui->comboBox_surveys->currentText();
    species=ui->comboBox_species->currentText();
    transition=ui->comboBox_transitions->currentText();
    
    parser->datacube_element.insert("Survey",survey);
    parser->datacube_element.insert("Species",species);
    parser->datacube_element.insert("Transition",transition);
    nam = new QNetworkAccessManager(this);
    QObject::connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishedSlot(QNetworkReply*)));

    //QUrl url ("http://vialactea:secret@palantir19.oats.inaf.it:8080/libjnifitsdb-0.15.0/vlkb_search?l="+ui->lineEdit_l->text()+"&b="+ui->lineEdit_b->text()+"&r="+ui->lineEdit_r->text()+"&vl="+ui->lineEdit_vl->text()+"&vu="+ui->lineEdit_vu->text());
    QUrl url(vlkbUrl+"/vlkb_search?l="+ui->lineEdit_l->text()+"&b="+ui->lineEdit_b->text()+"&r="+ui->lineEdit_r->text()+"&vl="+ui->lineEdit_vl->text()+"&vu="+ui->lineEdit_vu->text());


    

    //QNetworkReply* reply = nam->get(QNetworkRequest(url));
    nam->get(QNetworkRequest(url));
    
    ui->lineEdit_l->setReadOnly(false);
    ui->lineEdit_b->setReadOnly(false);
    ui->lineEdit_r->setReadOnly(false);
    ui->lineEdit_vl->setReadOnly(false);
    ui->lineEdit_vu->setReadOnly(false);
    
    


    
}

//datacube list button
void dbquery::on_pushButton_map_clicked()
{
    loading = new LoadingWidget();
    loading->init();
    loading->setFileName("Getting datacube list...");
    loading ->show();
    loading->activateWindow();
    loading->setFocus();

    nam = new QNetworkAccessManager(this);
    QObject::connect(nam, SIGNAL(finished(QNetworkReply*)),
                     this, SLOT(finishedSlot2(QNetworkReply*)));
    /*
    QUrl url ("http://vialactea:secret@palantir19.oats.inaf.it:8080/libjnifitsdb-0.15.0/vlkb_search?l="+ui->lineEdit_l->text()+
              "&b="+ui->lineEdit_b->text()+"&r="+ui->lineEdit_r->text()+"&vl="+ui->lineEdit_vl->text()+"&vu="+ui->lineEdit_vu->text());
  */
    QUrl url (vlkbUrl+"/vlkb_search?l="+ui->lineEdit_l->text()+
              "&b="+ui->lineEdit_b->text()+"&r="+ui->lineEdit_r->text()+"&vl="+ui->lineEdit_vl->text()+"&vu="+ui->lineEdit_vu->text());


    nam->get(QNetworkRequest(url));
}

void dbquery::handleButton(int i)
{
    loading = new LoadingWidget();
    loading->init();
    loading->setFileName("Downloading selected datacube...");
    loading ->show();
    loading->activateWindow();
    loading->setFocus();


    QMap<QString,QString> datacube=datacubes_list[i];

    nam = new QNetworkAccessManager(this);
    QObject::connect(nam, SIGNAL(finished(QNetworkReply*)),
                     this, SLOT(finishedSlot(QNetworkReply*)));
    
    QUrl url (vlkbUrl+"/vlkb_cutout?pubdid="+datacube["PublisherDID"]+"&l="
            +ui->lineEdit_l->text()+"&b="+ui->lineEdit_b->text()+"&r="+ui->lineEdit_r->text()+
            "&vl="+ui->lineEdit_vl->text()+"&vu="+ui->lineEdit_vu->text()+"&nullvals");
    /*
 QUrl url ("http://vialactea:secret@palantir19.oats.inaf.it:8080/libjnifitsdb-0.14.2/vlkb_cutout?pubdid="+datacube["PublisherDID"]+"&l="
 +ui->lineEdit_l->text()+"&b="+ui->lineEdit_b->text()+"&r="+ui->lineEdit_r->text()+
 "&vl="+ui->lineEdit_vl->text()+"&vu="+ui->lineEdit_vu->text());
*/
    nam->get(QNetworkRequest(url));
}

void dbquery::addDatacubesToUI(QList< QMap<QString,QString> >& datacubes) {
    
    
    /* Create the data model */
    // 1. give it some headers
    
    datacubes_list=datacubes;
    
    int i=0;
    int z=0;
    ui->datacube_tableWidget->clear();
    int rows=ui->datacube_tableWidget->rowCount();
    int columns=ui->datacube_tableWidget->columnCount();
    for(int j=0; j<rows; j++)
        ui->datacube_tableWidget->removeRow(j);
    for(int j=0; j<columns; j++)
        ui->datacube_tableWidget->removeColumn(j);
    
    
    ui->datacube_tableWidget->insertColumn(0);
    ui->datacube_tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Survey"));
    ui->datacube_tableWidget->insertColumn(1);
    ui->datacube_tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Species"));
    ui->datacube_tableWidget->insertColumn(2);
    ui->datacube_tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem("Transition"));
    ui->datacube_tableWidget->insertColumn(3);
    ui->datacube_tableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem("Visualize"));
    
    while(!datacubes.isEmpty()) {
        QMap<QString,QString> datacube = datacubes.takeFirst();
        //datacubes_list->append(datacube);

        if(! datacube["Survey"].contains("HI-Gal"))
        {
            ui->datacube_tableWidget->insertRow(i);
            QTableWidgetItem *item_0 = new QTableWidgetItem();
            item_0->setText(datacube["Survey"]);
            ui->datacube_tableWidget->setItem(i,0,item_0);

            QTableWidgetItem *item_1 = new QTableWidgetItem();
            item_1->setText(datacube["Species"]);
            ui->datacube_tableWidget->setItem(i,1,item_1);

            QTableWidgetItem *item_2 = new QTableWidgetItem();
            item_2->setText(datacube["Transition"]);
            ui->datacube_tableWidget->setItem(i,2,item_2);

            QPushButton *p_button=new QPushButton("Visualize", this);
            p_button->setGeometry(QRect(QPoint(100, 100),
                                        QSize(200, 50)));

            QSignalMapper* mapper = new QSignalMapper(this);
            int row=i;
            connect(p_button, SIGNAL(released()),mapper, SLOT(map()));
            mapper->setMapping(p_button, row);
            connect(mapper, SIGNAL(mapped(int)), this, SLOT(handleButton(int)));
            ui->datacube_tableWidget->setCellWidget(i,3,p_button);
        }
        i++;

        z++;
    }
}



void dbquery::on_horizontalSlider_sliderMoved(int position)
{

    double mov=vtkwin->min+(double)position/20.*vtkwin->max-vtkwin->min;
    //int mov= static_cast<int> (vtkwin->min) + static_cast<int>(position/50) % static_cast<int>(vtkwin->max-vtkwin->min);
    
    vtkwin->shellE->SetValue(0, mov);
    vtkwin->update();
    vtkwin->showNormal();
    //shellE->SetValue(1, position);
    //renWin->Render();
    
}



void dbquery::setVtkWindow(vtkwindow_new *v)
{
    vtkwin=v;
}


void dbquery::on_vtkwindow_button_clicked()
{
    //vtkwin=new vtkwindow(this);
}

void dbquery::on_spinBox_valueChanged(int arg1)
{
    vtkwin->shellE->SetValue(0, arg1);
    vtkwin->update();
    vtkwin->showNormal();
}

void dbquery::on_download_completed()
{


    vtkFitsReader *fitsReader = vtkFitsReader::New();
    fitsReader->is3D=true;
    QString currentPath=QDir::currentPath()+"/"+downloadedFile;
    
    fitsReader->SetFileName(currentPath.toStdString());
    fitsReader->CalculateRMS();
    fitsReader->Update();
    
    vtkwin=new vtkwindow_new(this, fitsReader, 1);
    
    /*
    vtkwin->setSurvey(survey);
    vtkwin->setSpecies(species);
    vtkwin->setTransition(transition);
    vtkwin->vtkcontourwindow->survey=survey;
    vtkwin->vtkcontourwindow->species=species;
    vtkwin->vtkcontourwindow->transition=transition;
  */
    
    //vtkcontourwindow=new vtkwindow(this, fitsReader, 2);
    
    //ui->horizontalSlider->setMaximum(fitsReader->GetSigma()*10);
    //ui->horizontalSlider->setMinimum(3*fitsReader->GetSigma());
    
}
