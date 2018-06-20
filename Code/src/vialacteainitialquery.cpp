#include "vialacteainitialquery.h"
#include "ui_vialacteainitialquery.h"
#include <QUrl>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QXmlStreamReader>
#include "downloadmanager.h"
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include "mainwindow.h"
#include "singleton.h"
#include <QSettings>
#include <QUrlQuery>

VialacteaInitialQuery::VialacteaInitialQuery(QString fn, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VialacteaInitialQuery)
{
    ui->setupUi(this);

    ui->rectGroupBox->hide();

    QSettings settings(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp").append("/setting.ini"), QSettings::NativeFormat);
    vlkbUrl= settings.value("vlkburl", "").toString();




    if (settings.value("vlkbtype", "public").toString()=="public")
    {
        qDebug()<<"public access to vlkb";
        settings.setValue("vlkburl","http://ia2-vialactea.oats.inaf.it/libjnifitsdb-1.0.2p/");
        settings.setValue("vlkbtableurl","http://ia2-vialactea.oats.inaf.it/vlkb/catalogues/tap");
 url_prefix="http://";


    }
    else if (settings.value("vlkbtype", "public").toString()=="private")
    {
        qDebug()<<"private access to vlkb";


        QString user= settings.value("vlkbuser", "").toString();
        QString pass = settings.value("vlkbpass", "").toString();


       // settings.setValue("vlkburl","http://"+user+":"+pass+"@ia2-vialactea.oats.inaf.it:8080/libjnifitsdb-0.23.2/");
      //  settings.setValue("vlkburl","http://"+user+":"+pass+"@ia2-vialactea.oats.inaf.it:8080/libjnifitsdb-0.23.16/");
        settings.setValue("vlkburl","http://"+user+":"+pass+"@ia2-vialactea.oats.inaf.it:8080/libjnifitsdb-1.0.2/");
        settings.setValue("vlkbtableurl","http://ia2-vialactea.oats.inaf.it:8080/vlkb");
        url_prefix="http://"+user+":"+pass+"@";


    }





  //  QString user= settings.value("vlkbuser", "").toString();
  //  QString pass = settings.value("vlkbpass", "").toString();

    // settings.setValue("vlkburl","http://"+user+":"+pass+"@ia2-vialactea.oats.inaf.it:8080/libjnifitsdb-0.23.16/");
    //settings.setValue("vlkburl","http://"+user+":"+pass+"@ia2-vialactea.oats.inaf.it:8080/libjnifitsdb-1.0.2/");


    parser =new xmlparser();
    loading = new LoadingWidget();
    outputFile=fn;
    species="";
    p=parent;
    myCallingVtkWindow=0;

    transitions.insert("PLW","500um");
    transitions.insert("PMW","350um");
    transitions.insert("PSW","250um");
    transitions.insert("red","160um");
    transitions.insert("blue","70um");



}

VialacteaInitialQuery::~VialacteaInitialQuery()
{
    delete ui;
}

void VialacteaInitialQuery::on_pushButton_clicked()
{
    this->close();
}


void VialacteaInitialQuery::on_pointRadioButton_toggled(bool checked)
{
    if(checked)
    {
        ui->rectGroupBox->hide();
        ui->pointGroupBox->show();
    }
    else
    {
        ui->rectGroupBox->show();
        ui->pointGroupBox->hide();
    }
}

void VialacteaInitialQuery::setL(QString l)
{
    ui->l_lineEdit->setText(l);
}
void VialacteaInitialQuery::setB(QString b)
{
    ui->b_lineEdit->setText(b.replace(" ",""));
}
void VialacteaInitialQuery::setR(QString r)
{
    isRadius=true;
    ui->r_lineEdit->setText(r);
}

void VialacteaInitialQuery::setDeltaRect(QString dl,QString db)
{
    isRadius=false;
    ui->dlLineEdit->setText(dl);
    ui->dbLineEdit->setText(db);


}
void VialacteaInitialQuery::setSurveyname(QString s)
{
    //surveyname="HI-Gal%20mosaic%20"+s;//PSW";
    // surveyname="HI-Gal%20mosaic";
    surveyname=s;
    // transition=transitions.value(s);
}

void VialacteaInitialQuery::setTransition(QString s)
{
    transition=s;

}



void VialacteaInitialQuery::cutoutRequest(QString url, QList< QMap<QString,QString> > el, int pos)
{
    loading ->show();
    loading->setFileName("Querying cutout services");
    elementsOnDb=el;
    species=elementsOnDb.at(pos).value("Species");
    pubdid=elementsOnDb.at(pos).value("PublisherDID");
    surveyname=elementsOnDb.at(pos).value("Survey");
    descriptionFromDB=elementsOnDb.at(pos).value("Description");
    transition=elementsOnDb.at(pos).value("Transition");
    velfrom=elementsOnDb.at(pos).value("from");
    velto=elementsOnDb.at(pos).value("to");

    velocityUnit=elementsOnDb.at(pos).value("VelocityUnit");
    nam = new QNetworkAccessManager(this);
    QObject::connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishedSlot(QNetworkReply*)));


    url=url.replace("http://",url_prefix);

    QUrl url_enc=QUrl(url);
    QList<QPair<QString, QString> > urlQuery = QUrlQuery(url_enc).queryItems();
    QList<QPair<QString, QString> > ::iterator j;
    for (j = urlQuery.begin(); j != urlQuery.end(); ++j)
    {
        (*j).second=QUrl::toPercentEncoding((*j).second);
    }

    QUrlQuery q;
    q.setQueryItems(urlQuery);
    url_enc.setQuery(q);

    nam->get(QNetworkRequest(url_enc));

}

void VialacteaInitialQuery::selectedStartingLayersRequest(QUrl url)
{
    qDebug()<<"1) L "<<ui->l_lineEdit->text()<<" B "<<ui->b_lineEdit->text()<<" DL "<<ui->dlLineEdit->text()<<" DB "<<ui->dbLineEdit->text()<<" R "<<ui->r_lineEdit->text();
    qDebug()<<"\t"<<" species"<<species<<" trans "<<transition<<" survey "<<surveyname;
    loading ->show();
    loading->setFileName("Querying cutout services");

    nam = new QNetworkAccessManager(this);
    QObject::connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishedSlot(QNetworkReply*)));

    qDebug()<<url;
    nam->get(QNetworkRequest(url));

}


void VialacteaInitialQuery::on_queryPushButton_clicked()
{


    loading ->show();
    loading->activateWindow();
    loading->setFileName("Querying cutout services");


    nam = new QNetworkAccessManager(this);
    QObject::connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishedSlot(QNetworkReply*)));

    QString urlString=vlkbUrl+"/vlkb_search?l="+ui->l_lineEdit->text()+"&b="+ui->b_lineEdit->text();//+"&species="+species;
    if(isRadius)
    {
        urlString+="&r="+ui->r_lineEdit->text();
    }
    else
        urlString+="&dl="+ui->dlLineEdit->text()+"&db="+ui->dbLineEdit->text();

    urlString+="&vl=-500000&vu=500000";

    QUrl url2 (urlString);

    nam->get(QNetworkRequest(url2));

    qDebug()<<"INITIAL QUERY:\n"<<urlString;
}


void VialacteaInitialQuery::finishedSlot(QNetworkReply* reply)
{
    QString string;
    QString file;


    qDebug()<<reply->errorString();

    qDebug()<<"2) L "<<ui->l_lineEdit->text()<<" B "<<ui->b_lineEdit->text()<<" DL "<<ui->dlLineEdit->text()<<" DB "<<ui->dbLineEdit->text()<<" R "<<ui->r_lineEdit->text();

    qDebug ()<<reply;


    if (reply->error() == QNetworkReply::NoError)
    {

        QXmlStreamReader xml(reply);
        QString url=reply->request().url().toString();

        /*       if( url.contains("vlkb_search")  )
        {

            parser->parseXML_fitsDownload(xml, string);


        }
*/
        //seconda query per cercare il fits corretto
        /*   if( url.contains("vlkb_search") && url.contains("surveyname") )
        {

            parser->parseXML_fitsDownload(xml, string);
            if(string.compare("NULL")!=0)
            {
                loading->setFileName("Fits image found");

                QString urlStringCutout=vlkbUrl+"/vlkb_cutout?pubdid="+QUrl::toPercentEncoding(string)+"&l="+ui->l_lineEdit->text()+"&b="+ui->b_lineEdit->text();
                if(isRadius)
                {
                    urlStringCutout+="&r="+ui->r_lineEdit->text();
                }
                else
                    urlStringCutout+="&dl="+ui->dlLineEdit->text()+"&db="+ui->dbLineEdit->text();

                urlStringCutout+="&vl=0&vu=0&nullvals";

                qDebug()<<urlStringCutout;
                QUrl url2 (urlStringCutout);


                //nam->get(QNetworkRequest(url2));
            }
            else
            {
                loading->setFileName("No fits image on this region");
                loading->loadingEnded();
                loading->hide();
                QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("No fits image on this region - Try again"));
            }
        }

        */

        if (url.contains("vlkb_cutout") || url.contains("vlkb_merge")  )
        {
            parser->parseXML(xml, string);

            if(!string.contains("NULL"))
            {
                loading->setFileName("Datacube found");
                DownloadManager *manager= new DownloadManager();
                QString urlString=string.trimmed();
                QUrl url3(urlString);

                //segnale tra due oggetti:
                connect(manager, SIGNAL(downloadCompleted()),this, SLOT(on_download_completed()));

                file=manager->doDownload(url3,outputFile);
                loading->loadingEnded();
                loading->hide();
                downloadedFile=file;

            }
            else
            {
                QStringList result_splitted=string.split(" ");
                QString msg="Inconsistent data (PubDID vs Region only partially overlap)";
                if(result_splitted.length()>1)
                    msg="Null values percentage is "+ result_splitted[1]+" (greater than 95%)";
                QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr(msg.toStdString().c_str()));

                loading->loadingEnded();
                loading->hide();

                //  p->show();
            }

        }

        //La query iniziale
        if( url.contains("vlkb_search") && !url.contains("surveyname") )
        {
            elementsOnDb= parser->parseXmlAndGetList(xml);
            QString best_url;
            int best_code=4;
            QList< QMap<QString,QString> >::iterator j;
             qDebug()<<"CERCO: "<<species<<" "<<" "<<surveyname<<" "<< transition;
            for (j = elementsOnDb.begin(); j != elementsOnDb.end(); ++j)
            {

                 qDebug()<<"ANALIZZO: "<<(*j).value("Species")<<" "<<(*j).value("Survey")<<" "<< (*j).value("Transition")<<" "<<(*j).value("code")<<(*j).value("VelocityUnit")<<" ";
                if( (*j).value("Species").compare(species)==0 && (*j).value("Survey").contains(surveyname)  && (*j).value("Transition").compare(transition) ==0  && (*j).value("code").toInt()<best_code  )
                {
                    // qDebug()<<(*j).value("Species")<<" "<<(*j).value("Survey")<<" "<<surveyname<<" "<<(*j).value("Transition") << " code "<< (*j).value("code");
                    best_url=(*j).value("URL");
                    best_code=(*j).value("code").toInt();
                }
            }





            QUrl url (best_url);

            QList<QPair<QString, QString> > urlQuery = QUrlQuery(url).queryItems();
            QList<QPair<QString, QString> > ::iterator i;
            for (i = urlQuery.begin(); i != urlQuery.end(); ++i)
            {
                (*i).second=QUrl::toPercentEncoding((*i).second);
            }

            QUrlQuery q;
            q.setQueryItems(urlQuery);
            url.setQuery(q);


            nam->get(QNetworkRequest(url));

            /*
        QString urlString=vlkbUrl+"/vlkb_search?surveyname="+QUrl::toPercentEncoding(surveyname)+"&l="+ui->l_lineEdit->text()+"&b="+ui->b_lineEdit->text()+"&species="+QUrl::toPercentEncoding(species)+"&transition="+QUrl::toPercentEncoding(transition);
        if(isRadius)
        {
            urlString+="&r="+ui->r_lineEdit->text();
        }
        else
            urlString+="&dl="+ui->dlLineEdit->text()+"&db="+ui->dbLineEdit->text();

        urlString+="&vl=0&vu=0";


        qDebug()<<"******°°°°°°° "<<urlString;
        QUrl url (urlString);
 nam->get(QNetworkRequest(url));

        */
        }

        if( url.contains("vlkb_search") && url.contains("surveyname") )
        {
            QList< QMap<QString,QString> > elementsOnDb_tmp= parser->parseXmlAndGetList(xml);

            QString best_url;
            int best_code=4;

            qDebug()<<"CERCO: "<<species<<" "<<" "<<surveyname<<" "<< transition;
            qDebug()<<"aaa "<<elementsOnDb_tmp.size();
            QList< QMap<QString,QString> >::iterator j;
            for (j = elementsOnDb_tmp.begin(); j != elementsOnDb_tmp.end(); ++j)
            {
                //qDebug()<<(*j)<;
                qDebug()<<"ANALIZZO: "<<(*j).value("Species")<<" "<<(*j).value("Survey")<<" "<< (*j).value("Transition")<<" "<<(*j).value("code");

                if( (*j).value("Species").compare(species)==0 && (*j).value("Survey").contains(surveyname) && (*j).value("Transition").compare(transition) ==0  && (*j).value("code").toInt()<best_code  )
                {

                    // qDebug()<<(*j).value("Species")<<" "<<(*j).value("Survey")<<" "<<surveyname<<" "<<(*j).value("Transition") << " code "<< (*j).value("code");
                    best_url=(*j).value("URL");
                    best_code=(*j).value("code").toInt();
                }
            }

            qDebug()<<"parrrrarararar B:    "<<best_url;




            QUrl url (best_url);

            QList<QPair<QString, QString> > urlQuery = QUrlQuery(url).queryItems();
            QList<QPair<QString, QString> > ::iterator i;
            for (i = urlQuery.begin(); i != urlQuery.end(); ++i)
            {
                (*i).second=QUrl::toPercentEncoding((*i).second);
            }

            QUrlQuery q;
            q.setQueryItems(urlQuery);
            url.setQuery(q);


            nam->get(QNetworkRequest(url));
        }


        /*
    if (reply->error() == QNetworkReply::NoError)
    {

        QXmlStreamReader xml(reply);
        QString url=reply->request().url().toString();
        qDebug()<<"finishedSlot ";
        if( url.contains("vlkb_search")  )
        {

            parser->parseXML_fitsDownload(xml, string);
            qDebug()<<"STRING:\n"<<string;


        }
*/
        /*
        //seconda query per cercare il fits corretto
        if( url.contains("vlkb_search") && url.contains("surveyname") )
        {

            parser->parseXML_fitsDownload(xml, string);
            qDebug()<<string;
            if(string.compare("NULL")!=0)
            {
                loading->setFileName("Fits image found");

                QString urlStringCutout=vlkbUrl+"/vlkb_cutout?pubdid="+QUrl::toPercentEncoding(string)+"&l="+ui->l_lineEdit->text()+"&b="+ui->b_lineEdit->text();
                if(isRadius)
                {
                    urlStringCutout+="&r="+ui->r_lineEdit->text();
                }
                else
                    urlStringCutout+="&dl="+ui->dlLineEdit->text()+"&db="+ui->dbLineEdit->text();

                urlStringCutout+="&vl=0&vu=0&nullvals";

                qDebug()<<urlStringCutout;
                QUrl url2 (urlStringCutout);


                nam->get(QNetworkRequest(url2));
            }
            else
            {
                loading->setFileName("No fits image on this region");
                loading->loadingEnded();
                loading->hide();
                QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("No fits image on this region - Try again"));
            }
        }

        if (url.contains("vlkb_cutout") )
        {
            parser->parseXML(xml, string);

            if(!string.contains("NULL"))
            {
                loading->setFileName("Datacube found");
                DownloadManager *manager= new DownloadManager();
                QString urlString=string.trimmed();
                QUrl url3(urlString);

                //segnale tra due oggetti:
                connect(manager, SIGNAL(downloadCompleted()),this, SLOT(on_download_completed()));

                file=manager->doDownload(url3,outputFile);
                loading->loadingEnded();
                loading->hide();
                downloadedFile=file;

            }
            else
            {
                QStringList result_splitted=string.split(" ");
                QString msg="Inconsistent data (PubDID vs Region only partially overlap)";
                if(result_splitted.length()>1)
                    msg="Null values percentage is "+ result_splitted[1]+" (greater than 95%)";
                QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr(msg.toStdString().c_str()));

                loading->loadingEnded();
                loading->hide();

                //  p->show();
            }

        }

        //La query iniziale
        if( url.contains("vlkb_search") && !url.contains("surveyname") )
        {
            elementsOnDb= parser->parseXmlAndGetList(xml);

            QString urlString=vlkbUrl+"/vlkb_search?surveyname="+QUrl::toPercentEncoding(surveyname)+"&l="+ui->l_lineEdit->text()+"&b="+ui->b_lineEdit->text()+"&species="+QUrl::toPercentEncoding(species)+"&transition="+QUrl::toPercentEncoding(transition);
            if(isRadius)
            {
                urlString+="&r="+ui->r_lineEdit->text();
            }
            else
                urlString+="&dl="+ui->dlLineEdit->text()+"&db="+ui->dbLineEdit->text();

            urlString+="&vl=0&vu=0";


            qDebug()<<"******°°°°°°° "<<urlString;
            QUrl url (urlString);
            nam->get(QNetworkRequest(url));
        }

        */
    }

    else
    {
        //handle errors here
        loading->loadingEnded();
        loading->hide();
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("Server is not replying"));

    }
    //delete reply;
    reply->deleteLater();

}


void VialacteaInitialQuery::on_download_completed()
{
    int test_flag_nanten=-1;
    if(pubdid.compare("")!=0)
        test_flag_nanten = pubdid.split("_", QString::SkipEmptyParts).last().toInt();

    QString currentPath;
    if (outputFile=="")
        currentPath=downloadedFile;
    else
        currentPath=downloadedFile;

    this->close();

    MainWindow *w = &Singleton<MainWindow>::Instance();

    //if( (velfrom.compare("0.0") ==0 && velto.compare("0.0")==0 )  || ( surveyname.compare("NANTEN")==0 && test_flag_nanten==2) || QString::compare("cornish",surveyname, Qt::CaseInsensitive)==0|| QString::compare("cornish2d",surveyname, Qt::CaseInsensitive)==0 || QString::compare("magpis",surveyname, Qt::CaseInsensitive)==0  )

    if ( (velfrom.compare("0.0") ==0 && velto.compare("0.0")==0 ) || species.compare("dust") ==0 || species.compare("Continuum")==0 || ( surveyname.compare("NANTEN")==0 && test_flag_nanten==2) || QString::compare("cornish",surveyname, Qt::CaseInsensitive)==0|| QString::compare("cornish2d",surveyname, Qt::CaseInsensitive)==0 || QString::compare("magpis",surveyname, Qt::CaseInsensitive)==0  )
    {
        bool l =false;

        //Per layer fits decommetante FV

        if(myCallingVtkWindow!=0)
        {
            l=true;
        }


        w->setSurvey(surveyname);
        w->setSpecies(species);
        w->setTransition(transition);
        w->setCallingVtkWindow(myCallingVtkWindow);
        w->setSelectedSurveyMap(selectedSurvey);
        w->importFitsImage(currentPath, elementsOnDb,ui->l_lineEdit->text(),ui->b_lineEdit->text(),ui->r_lineEdit->text(), ui->dbLineEdit->text(), ui->dlLineEdit->text(), l);
    }
    else
    {
        myCallingVtkWindow->setSelectedCubeVelocityUnit(velocityUnit);
        w->setCallingVtkWindow(myCallingVtkWindow);
        w->importFitsDC(currentPath);
    }
}
