#include "vlkbsimplequerycomposer.h"
#include "ui_vlkbsimplequerycomposer.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QtDebug>
#include <QDomDocument>
#include <QAuthenticator>
#include <QFile>
#include <QDir>
#include "vialactea_fileload.h"
#include "singleton.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>

VLKBSimpleQueryComposer::VLKBSimpleQueryComposer(vtkwindow_new *v, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VLKBSimpleQueryComposer)
{
    ui->setupUi(this);
    vtkwin=v;
    loading = new LoadingWidget();
    isConnected=false;
    isFilament=false;
    is3dSelections=false;
    isBubble=false;

    m_sSettingsFile = QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp").append("/setting.ini");
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);


    url=settings.value("vlkbtableurl", "").toString();
    ui->vlkbUrlLineEdit->setText(url);

    on_connectPushButton_clicked();

}

VLKBSimpleQueryComposer::~VLKBSimpleQueryComposer()
{
    delete ui;
}

void VLKBSimpleQueryComposer::setVtkWindow(vtkwindow_new *v)
{
    vtkwin=v;
}
void VLKBSimpleQueryComposer::setIsFilament()
{
    isFilament=true;
    ui->tableGroupBox->hide();
}

void VLKBSimpleQueryComposer::setIsBubble()
{
    isBubble=true;
    ui->tableGroupBox->hide();
}


void VLKBSimpleQueryComposer::setIs3dSelections()
{
    is3dSelections=true;
    ui->tableGroupBox->hide();
}

void VLKBSimpleQueryComposer::setLongitude(float long1, float long2)
{
    float min=long1;
    float max=long2;

    if(long1>=long2)
    {
        max=long1;
        min=long2;
    }

    ui->longMaxLineEdit->setText( QString::number( max) );
    ui->longMinLineEdit->setText( QString::number(min) );

}

void VLKBSimpleQueryComposer::setLatitude(float lat1, float lat2)
{
    float min=lat1;
    float max=lat2;

    if(lat1>=lat2)
    {
        max=lat1;
        min=lat2;
    }

    ui->latMaxLineEdit->setText( QString::number( max) );
    ui->latMinLineEdit->setText( QString::number(min) );

}


void VLKBSimpleQueryComposer::on_connectPushButton_clicked()
{

    if(!isConnected)
    {

        loading = new LoadingWidget();
        loading->init();
        loading->setFileName("Connecting to VLKB...");
        loading ->show();
        loading->activateWindow();
        loading->setFocus();

        //url=ui->vlkbUrlLineEdit->text();



        manager = new QNetworkAccessManager(this);

        connect(manager, SIGNAL(finished(QNetworkReply*)),  this, SLOT(availReplyFinished(QNetworkReply*)));
        manager->get(QNetworkRequest(QUrl(url+"/availability")));
       // manager->get(QNetworkRequest(QUrl("http://ia2-vialactea.oats.inaf.it/vlkb/availability")));

    }
    else
    {

        ui->connectPushButton->setText("Connect");
        ui->vlkbUrlLineEdit->setEnabled(true);


        ui->tableComboBox->setEnabled(false);
        ui->longMaxLineEdit->setEnabled(false);
        ui->longMaxLineEdit->setEnabled(false);
        ui->latMinLineEdit->setEnabled(false);
        ui->latMaxLineEdit->setEnabled(false);
        ui->queryPushButton->setEnabled(false);
        ui->outputNameLineEdit->setEnabled(false);

        isConnected=false;
    }

}

void VLKBSimpleQueryComposer::availReplyFinished (QNetworkReply *reply)
{

    qDebug()<<"availReplyFinished";

    if(reply->error())
    {

        QMessageBox::critical(this,"Error", "Error: \n"+reply->errorString());

        ui->tableComboBox->setEnabled(false);
        ui->longMaxLineEdit->setEnabled(false);
        ui->longMaxLineEdit->setEnabled(false);
        ui->latMinLineEdit->setEnabled(false);
        ui->latMaxLineEdit->setEnabled(false);
        ui->queryPushButton->setEnabled(false);
        ui->outputNameLineEdit->setEnabled(false);

        available=false;

    }
    else
    {
        QDomDocument doc;
        doc.setContent(reply->readAll());
        qDebug()<<doc.toByteArray();
        QDomNodeList list=doc.elementsByTagName("vosi:available");
        qDebug()<<list.at(0).toElement().text();

        if(list.at(0).toElement().text()=="true")
        {
            qDebug()<<"entro";

            available=true;
            qDebug()<<"isFilament "<<isFilament;

            if(!isFilament)
            {
                qDebug()<<"cerco tabelle";
                QNetworkAccessManager * manager = new QNetworkAccessManager(this);

                connect(manager, SIGNAL(finished(QNetworkReply*)),  this, SLOT(tableReplyFinished(QNetworkReply*)));
                manager->get(QNetworkRequest(QUrl(url+"/tables")));
                //manager->get(QNetworkRequest(QUrl("http://ia2-vialactea.oats.inaf.it:8080/vlkb/tables")),postData);


            }
            else
            {
                ui->tableComboBox->setEnabled(true);
                ui->longMaxLineEdit->setEnabled(true);
                ui->longMinLineEdit->setEnabled(true);
                ui->latMinLineEdit->setEnabled(true);
                ui->latMaxLineEdit->setEnabled(true);
                ui->queryPushButton->setEnabled(true);

                ui->connectPushButton->setText("Disconnect");
                ui->vlkbUrlLineEdit->setEnabled(false);
                isConnected=true;

                loading->close();
                reply->deleteLater();
            }
        }
    }

    reply->deleteLater();

}


void VLKBSimpleQueryComposer::tableReplyFinished (QNetworkReply *reply)
{

    qDebug()<<"tableReplyFinished";
    if(reply->error())
    {
        QMessageBox::critical(this,"Error", "Error: \n"+reply->errorString());
    }
    else
    {
        QDomDocument doc;
        doc.setContent(reply->readAll());
        QDomNodeList list=doc.elementsByTagName("table");

        ui->tableComboBox->clear();
        for(int i=0;i<list.size();i++)
        {
            VlkbTable *table= new VlkbTable();
            QDomElement node = list.at(i).toElement();
            QString bandTableName;

            for (int j=0;j<node.childNodes().size();j++)
            {
                if(node.childNodes().at(j).toElement().tagName()=="name")
                {
                    table->setName(node.childNodes().at(j).toElement().text());
                    if(table->getName().contains("vlkb_compactsources.band") && ( table->getName().right(2).compare("um") ==0) )
                    {
                        bandTableName = table->getName().mid( table->getName().lastIndexOf(".")+1 );
                        table->setShortname( bandTableName );
                        ui->tableComboBox->addItem(table->getShortName());

                        qDebug()<<""<<table->getShortName();


                    }
                }
                else if(node.childNodes().at(j).toElement().tagName()=="column")
                {
                    table->addColumn(node.childNodes().at(j).toElement().childNodes().at(0).toElement().text(),node.childNodes().at(j).toElement().childNodes().at(1).toElement().text());
                }
            }

            table_list.append(table);
            //  ui->tableComboBox->addItem(table->getName());

        }

        ui->tableComboBox->addItem("bandmerged");

        int index =  ui->tableComboBox->findText("bandmerged");
        if ( index != -1 ) { // -1 for not found
            ui->tableComboBox->setCurrentIndex(index);
        }


        ui->tableComboBox->setEnabled(true);
        ui->longMaxLineEdit->setEnabled(true);
        ui->longMinLineEdit->setEnabled(true);
        ui->latMinLineEdit->setEnabled(true);
        ui->latMaxLineEdit->setEnabled(true);
        ui->queryPushButton->setEnabled(true);

        ui->connectPushButton->setText("Disconnect");
        ui->vlkbUrlLineEdit->setEnabled(false);
        isConnected=true;
        //non funziona, da verificare
        //ui->tableComboBox->addItem("AllBands");

    }
    loading->close();

    reply->deleteLater();

}

void VLKBSimpleQueryComposer::on_queryPushButton_clicked()
{
    doQuery(ui->tableComboBox->currentText());
    this->close();

    /*
    for(int i=0; i<ui->tableComboBox->count(); i++)
    {
       qDebug()<<ui->tableComboBox->itemText(i)<<" -"<<ui->tableComboBox->itemData(i).value<bool>();
       if(ui->tableComboBox->itemData(i).value<bool>())
            doQuery(ui->tableComboBox->itemText(i));
    }
    */
}

void VLKBSimpleQueryComposer::doQuery(QString band)
{
    loading->init();
    loading->setFileName("Retrieving dataset from VLKB");

    loading ->show();
    // loading->activateWindow();

    manager = new QNetworkAccessManager(this);

    QByteArray postData;

    postData.append("REQUEST=doQuery&");
    postData.append("VERSION=1.0&");
    postData.append("LANG=ADQL&");
    postData.append("FORMAT=tsv&");

    if(isFilament)
    {
/*

        //query funzionante
        QString query="SELECT * FROM vlkb_filaments.filaments WHERE ( glon>="+ui->longMinLineEdit->text()+" and glon<="+ui->longMaxLineEdit->text()+") AND ";
        query+= "(glat>="+ui->latMinLineEdit->text()+" and glat<="+ui->latMaxLineEdit->text()+")";

*/

        QString query="SELECT f.idfil_mos as idfil_mos, f.contour as contour, f.glon as glon, f.glat as glat, b.contour as branches_contour, b.contour1d as branches_contour1d, b.contour_new as branches_contour_new, b.flagspine as flagspine_branches  FROM vlkb_filaments.filaments as f JOIN vlkb_filaments.branches as b on f.idfil_mos = b.idfil_mos";
        query += " WHERE( glon>="+ui->longMinLineEdit->text()+" and glon<="+ui->longMaxLineEdit->text()+") AND ";
        query += "(glat>="+ui->latMinLineEdit->text()+" and glat<="+ui->latMaxLineEdit->text()+")";

        postData.append("QUERY="+QUrl::toPercentEncoding(query));

        qDebug()<<query;
    }
    else if(isBubble)
    {
        QString query="SELECT * FROM vlkb_filaments.bubbles WHERE ( glon_cen>="+ui->longMinLineEdit->text()+" and glon_cen<="+ui->longMaxLineEdit->text()+") AND ";
        query+= "(glat_cen>="+ui->latMinLineEdit->text()+" and glat_cen<="+ui->latMaxLineEdit->text()+")";
        postData.append("QUERY="+QUrl::toPercentEncoding(query));

        qDebug()<<query;
    }
    else if(is3dSelections)
    {
        //old
        /*
        QString query="SELECT * FROM vlkb_compactsources.bandmerged_sed_view WHERE ";
        query += "(( glon250>="+ui->longMinLineEdit->text()+" and glon250<="+ui->longMaxLineEdit->text()+") AND ";
        query+= "(glat250>="+ui->latMinLineEdit->text()+" and glat250<="+ui->latMaxLineEdit->text()+")";
        query += " AND x !=0 )";

        qDebug()<<query;
        postData.append("QUERY="+QUrl::toPercentEncoding(query));
        */

        QString query="SELECT dist*cos(radians(glon)+3.14159265359/2) as x, dist*sin(radians(glon)+3.14159265359/2) as y,dist*tan(radians(glat)) as z, * FROM vlkb_compactsources.props_dist WHERE ";
        query += "(( glon>="+ui->longMinLineEdit->text()+" and glon<="+ui->longMaxLineEdit->text()+") AND ";
        query+= "(glat>="+ui->latMinLineEdit->text()+" and glat<="+ui->latMaxLineEdit->text()+"))";

        qDebug()<<query;
        postData.append("QUERY="+QUrl::toPercentEncoding(query));


    }
    else
    {
        if(band.compare("bandmerged") ==0)
        {
            isBandmerged=true;
           /*
           // QString query="SELECT DISTINCT * FROM vlkb_compactsources.bandmerged_sed_view WHERE";
            query+= "(( glon500>="+ui->longMinLineEdit->text()+" and glon500<="+ui->longMaxLineEdit->text()+") AND ";
            query+= "(glat500>="+ui->latMinLineEdit->text()+" and glat500<="+ui->latMaxLineEdit->text()+"))";

            query+= " OR (( glon350>="+ui->longMinLineEdit->text()+" and glon350<="+ui->longMaxLineEdit->text()+") AND ";
            query+= "(glat350>="+ui->latMinLineEdit->text()+" and glat350<="+ui->latMaxLineEdit->text()+"))";

            query+= " OR (( glon250>="+ui->longMinLineEdit->text()+" and glon250<="+ui->longMaxLineEdit->text()+") AND ";
            query+= "(glat250>="+ui->latMinLineEdit->text()+" and glat250<="+ui->latMaxLineEdit->text()+"))";

            query+= " OR (( glon160>="+ui->longMinLineEdit->text()+" and glon160<="+ui->longMaxLineEdit->text()+") AND ";
            query+= "(glat160>="+ui->latMinLineEdit->text()+" and glat160<="+ui->latMaxLineEdit->text()+"))";

            query+= " OR (( glon70>="+ui->longMinLineEdit->text()+" and glon70<="+ui->longMaxLineEdit->text()+") AND ";
            query+= "(glat70>="+ui->latMinLineEdit->text()+" and glat70<="+ui->latMaxLineEdit->text()+"))";
*/

            QString query="SELECT DISTINCT * FROM vlkb_compactsources.sed_view_final WHERE ";
            query+= "(( glonft>="+ui->longMinLineEdit->text()+" and glonft<="+ui->longMaxLineEdit->text()+") AND ";
            query+= "(glatft>="+ui->latMinLineEdit->text()+" and glatft<="+ui->latMaxLineEdit->text()+"))";

            qDebug()<<" query REMOTE: "<<query;

            postData.append("QUERY="+QUrl::toPercentEncoding(query));

        }
        else
        {
            isBandmerged=false;

            //aggiungo al table name 'vlkb_compactsources.'
            QString query="SELECT * FROM vlkb_compactsources."+band+" WHERE ( glon>="+ui->longMinLineEdit->text()+" and glon<="+ui->longMaxLineEdit->text()+") AND ";
            query+= "(glat>="+ui->latMinLineEdit->text()+" and glat<="+ui->latMaxLineEdit->text()+")";
            postData.append("QUERY="+QUrl::toPercentEncoding(query));
        }
    }
    connect ( manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
              this,
              SLOT(onAuthenticationRequestSlot(QNetworkReply*,QAuthenticator*)) );

    connect(manager, SIGNAL(finished(QNetworkReply*)),  this, SLOT(queryReplyFinished(QNetworkReply*)));

    manager->post(QNetworkRequest(url+"/sync"),postData);
    //manager->post(QNetworkRequest(QUrl("http://ia2-vialactea.oats.inaf.it:8080/vlkb/sync")),postData);
    qDebug()<<" query url: "<<url;

}


void VLKBSimpleQueryComposer::onAuthenticationRequestSlot(QNetworkReply *aReply, QAuthenticator *aAuthenticator)
{
    aAuthenticator->setUser("vialactea");
    aAuthenticator->setPassword("secret");
}


void VLKBSimpleQueryComposer::queryReplyFinished (QNetworkReply *reply)
{

    if(reply->error())
    {
        QMessageBox::critical(this,"Error", "Error: \n"+reply->errorString());
    }
    else
    {
        QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

        /* We'll deduct if the redirection is valid in the redirectUrl function */
        urlRedirectedTo = redirectUrl(possibleRedirectUrl.toUrl(), urlRedirectedTo);

        /* If the URL is not empty, we're being redirected. */
        if(!urlRedirectedTo.isEmpty())
        {
            /* We'll do another request to the redirection url. */
            manager->get(QNetworkRequest(urlRedirectedTo));
        }
        else
        {

            QByteArray bytes = reply->readAll();
            QString s_data = QString::fromLatin1(bytes.data());

            //se inizia per '<' è un xml, se è xml c'e' stato un errore
            if(QString::compare(s_data.at(0), "<", Qt::CaseInsensitive) == 0)
            {
                QMessageBox::critical(this,"Error", "Error: \n"+bytes);
            }
            else
            {

                QString output_file;
                if ( ui->savedatasetCheckBox->isChecked())
                {
                    output_file=ui->outputNameLineEdit->text();
                }
                else
                {
                    output_file=QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp");
                    output_file.append("/tmp_download/").append("temp_dataset.dat");
                }

                QFile file(output_file);
                file.open(QIODevice::WriteOnly);
                file.write(bytes);
                file.close();

                QFile fileRead(output_file);
                fileRead.open(QIODevice::ReadOnly);
                fileRead.readLine();
                QString check=fileRead.readLine();
                fileRead.close();

                if (check!="")
                {
                    Vialactea_FileLoad *fileload = new Vialactea_FileLoad(output_file,true);
                    fileload->setVtkWin(vtkwin);

                    if(isFilament)
                    {
                        fileload->importFilaments();
                    }
                    else if(isBubble)
                    {
                        fileload->importBubbles();
                    }
                    else if (is3dSelections)
                    {
                        fileload->import3dSelection();
                    }
                    else
                    {
                        if( !isBandmerged )
                        {
                            qDebug()<<"******1";
                            fileload->setCatalogueActive();

                            QString w=  ui->tableComboBox->currentText().replace("band","").replace("um","");

                            fileload->setWavelength(w);
                            fileload->on_okPushButton_clicked();
                            //fileload->show();
                        }
                        else
                        {
                            qDebug()<<"******2";

                            qDebug()<<"else";
                            //fileload->importBandMerged();
                            fileload->setCatalogueActive();
                            fileload->setWavelength("all");

                            fileload->on_okPushButton_clicked();

                        }
                    }
                    //delete temp file
                    if (! ui->savedatasetCheckBox->isChecked())
                        QFile::remove(output_file);
                }
                else
                {
                    QMessageBox::critical(this,"Error", "Empty table. Table contained no rows");
                }

            }
            loading->loadingEnded();
            loading->hide();

        }
        /* Clean up. */
        reply->deleteLater();

    }

}

QUrl VLKBSimpleQueryComposer::redirectUrl(const QUrl& possibleRedirectUrl,   const QUrl& oldRedirectUrl) const {
    QUrl redirectUrl;
    /*
* Check if the URL is empty and
* that we aren't being fooled into a infinite redirect loop.
* We could also keep track of how many redirects we have been to
* and set a limit to it, but we'll leave that to you.
*/
    if(!possibleRedirectUrl.isEmpty() && possibleRedirectUrl != oldRedirectUrl) {
        redirectUrl = possibleRedirectUrl;
    }

    return redirectUrl;
}


void VLKBSimpleQueryComposer::on_tableComboBox_currentIndexChanged(int index)
{
    ui->outputNameLineEdit->setText( ui->tableComboBox->itemText(index)+".dat" );
    ui->outputNameLineEdit->setFocus();
}

void VLKBSimpleQueryComposer::on_savedatasetCheckBox_clicked(bool checked)
{
    if (checked)
    {
        ui->navigateFSButtono->setEnabled(true);
        ui->outputNameLabel->setEnabled(true);
        ui->outputNameLineEdit->setEnabled(true);
    }
    else
    {
        ui->navigateFSButtono->setEnabled(false);
        ui->outputNameLabel->setEnabled(false);
        ui->outputNameLineEdit->setEnabled(false);

    }

}

void VLKBSimpleQueryComposer::on_navigateFSButtono_clicked()
{

    QString fn = QFileDialog::getSaveFileName(this, "Save as...", QString(), "dataset files (*.dat)");

    if (!fn.isEmpty() && ! fn.endsWith(".dat", Qt::CaseInsensitive)  )
        fn += ".dat"; // default
    ui->outputNameLineEdit->setText(fn);

}

void VLKBSimpleQueryComposer::closeSlot()
{
    qDebug()<<"close slot";
}

void VLKBSimpleQueryComposer::closeEvent ( QCloseEvent * event )
{
    //if(!is3dSelections)
    //{
    if(vtkwin!=0){
        vtkwin->activateWindow();
    }
    //    vtkwin->show();
    //}
}
