#include "vlkbquery.h"
#include "ui_vlkbquery.h"
#include <QDebug>
#include <QDomDocument>
#include <QDir>
#include "vialactea_fileload.h"
#include <QMessageBox>
#include "sedvisualizerplot.h"

VLKBQuery::VLKBQuery(QString q, vtkwindow_new *v, QString w, QWidget *parent, Qt::GlobalColor color) :
    QWidget(parent),
    ui(new Ui::VLKBQuery)
{
    ui->setupUi(this);

    if (parent != 0)
    {
        sd = dynamic_cast<SEDVisualizerPlot*>(parent);
    }


    query=q;//QUrl::toPercentEncoding(q);

    loading = new LoadingWidget();
    loading->init();
    loading->setFileName("Querying VLKB..");
    loading ->show();
    loading->activateWindow();

    vtkwin=v;
    what=w;
    modelColor=color;
    connectToVlkb();

}

void VLKBQuery::connectToVlkb()
{
    qDebug()<<"connectToVlkb";
    url= "http://ia2-vialactea.oats.inaf.it:8080/vlkb";
    manager = new QNetworkAccessManager(this);
    qDebug()<<"connect";
    connect(manager, SIGNAL(finished(QNetworkReply*)),  this, SLOT(availReplyFinished(QNetworkReply*)));
    manager->get(QNetworkRequest(QUrl(url+"/availability")));
//    manager->get(QNetworkRequest(QUrl("http://ia2-vialactea.oats.inaf.it/vlkb/availability")));

    qDebug()<<"connected";

}

void VLKBQuery::availReplyFinished (QNetworkReply *reply)
{
    qDebug()<<"availReplyFinished";
    if(reply->error())
    {
        qDebug() << "ERROR!";
        qDebug() << reply->errorString();
        available=false;
    }
    else
    {
        QDomDocument doc;
        doc.setContent(reply->readAll());
        QDomNodeList list=doc.elementsByTagName("vosi:available");
        if(list.at(0).toElement().text()=="true")
        {

            available=true;
            executeQuery();
        }
    }
    reply->deleteLater();
}

void VLKBQuery::executeQuery()
{

    qDebug()<<"executeQuery";
    qDebug()<<query;
    manager = new QNetworkAccessManager(this);

    QByteArray postData;

    postData.append("REQUEST=doQuery&");
    postData.append("VERSION=1.0&");
    postData.append("LANG=ADQL&");
    postData.append("FORMAT=tsv&");
    postData.append("QUERY="+QUrl::toPercentEncoding(query));

    connect ( manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
              this,
              SLOT(onAuthenticationRequestSlot(QNetworkReply*,QAuthenticator*)) );
    // connect(manager, SIGNAL(finished(QNetworkReply*)),  this, SLOT(queryReplyFinished(QNetworkReply*)));
    if(what.compare("bm")==0)
        connect(manager, SIGNAL(finished(QNetworkReply*)),  this, SLOT(queryReplyFinishedBM(QNetworkReply*)));
    else if(what.compare("model")==0)
        connect(manager, SIGNAL(finished(QNetworkReply*)),  this, SLOT(queryReplyFinishedModel(QNetworkReply*)));

    manager->post(QNetworkRequest(url+"/sync"),postData);
    //manager->post(QNetworkRequest(QUrl("http://ia2-vialactea.oats.inaf.it:8080/vlkb/sync")),postData);


}


void VLKBQuery::executoSyncQuery()
{

    QByteArray postData;

    postData.append("REQUEST=doQuery&");
    postData.append("VERSION=1.0&");
    postData.append("LANG=ADQL&");
    postData.append("FORMAT=tsv&");


    QNetworkAccessManager *networkMgr = new QNetworkAccessManager(this);
    connect ( networkMgr, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
              this,
              SLOT(onAuthenticationRequestSlot(QNetworkReply*,QAuthenticator*)) );

    QNetworkReply *reply = networkMgr->get(QNetworkRequest(url+"/sync?REQUEST=doQuery&VERSION=1.0&LANG=ADQL&FORMAT=tsv&QUERY="+QUrl::toPercentEncoding(query)));

    qDebug()<<"pre loop >>> ";
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));

    // Execute the event loop here, now we will wait here until readyRead() signal is emitted
    // which in turn will trigger event loop quit.
    loop.exec();
    qDebug()<<"post loop";

    // Lets print the HTTP response.
    qDebug( reply->readAll() );
}


void VLKBQuery::onAuthenticationRequestSlot(QNetworkReply *aReply, QAuthenticator *aAuthenticator)
{
    qDebug() <<"auth";
    aAuthenticator->setUser("vialactea");
    aAuthenticator->setPassword("secret");
}

void VLKBQuery::queryReplyFinishedModel (QNetworkReply *reply)
{

    qDebug()<<"MODEL QUERY";

    if(reply->error())
    {
        qDebug() << "ERROR!";
        qDebug() << reply->errorString();
    }
    else
    {


        QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

        /* We'll deduct if the redirection is valid in the redirectUrl function */
        urlRedirectedTo = redirectUrl(possibleRedirectUrl.toUrl(), urlRedirectedTo);

        /* If the URL is not empty, we're being redirected. */
        if(!urlRedirectedTo.isEmpty())
        {

            qDebug()<<"URL REDIREZIONE: "<< urlRedirectedTo.toString();

            /* We'll do another request to the redirection url. */
            manager->get(QNetworkRequest(urlRedirectedTo));
        }
        else
        {

            QByteArray bytes = reply->readAll();

            loading->loadingEnded();
            loading->hide();



            QVector<QStringList> headerAndValueList;
            QList<QByteArray> lines = bytes.split('\r\n');

            foreach ( const QByteArray &line, lines)
            {
                QString myString (line);
                headerAndValueList.append( myString.split("\t") );
            }

            qDebug()<<"headerAndValueList: "<< headerAndValueList;

            sd->setModelFitValue(headerAndValueList, modelColor);
            /*
            QString s_data = QString::fromAscii(bytes.data());

            //se inizia per < è un xml, se è xml c'e' stato un errore
            if(QString::compare(s_data.at(0), "<", Qt::CaseInsensitive) == 0)
            {
                QMessageBox::critical(this,"Error", "Error: \n"+bytes);
            }
            //Fare un controllo più serio, su quando non ci sono sed restituite
            else if (bytes.size()!=1187)
            {
                QString output_file=QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp");
                output_file.append("/").append("test_model").append(".dat");

                QFile file(output_file);
                file.open(QIODevice::WriteOnly);
                file.write(bytes);
                file.close();

//                Vialactea_FileLoad *fileload = new Vialactea_FileLoad(output_file,vtkwin);
  //              fileload->importBandMerged();
            }
            else
            {
                QMessageBox::information(this,"Alert", "No SED");

            }
            loading->loadingEnded();
            loading->hide();

            /*
               Vialactea_FileLoad *fileload = new Vialactea_FileLoad(output_file);
               fileload->setVtkWin(vtkwin);
               fileload->show();
*/

        }
        /* Clean up. */
        reply->deleteLater();
    }


}

void VLKBQuery::queryReplyFinishedBM (QNetworkReply *reply)
{

    qDebug()<<"BM QUERY";


    if(reply->error())
    {
        qDebug() << "ERROR!";
        qDebug() << reply->errorString();
    }
    else
    {


        QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

        /* We'll deduct if the redirection is valid in the redirectUrl function */
        urlRedirectedTo = redirectUrl(possibleRedirectUrl.toUrl(), urlRedirectedTo);

        /* If the URL is not empty, we're being redirected. */
        if(!urlRedirectedTo.isEmpty())
        {

            qDebug()<<"URL REDIREZIONE: "<< urlRedirectedTo.toString();

            /* We'll do another request to the redirection url. */
            manager->get(QNetworkRequest(urlRedirectedTo));
        }
        else
        {

            QByteArray bytes = reply->readAll();
            qDebug()<<"Size: "<< bytes.size();

            QString s_data = QString::fromLatin1(bytes.data());

            //se inizia per < è un xml, se è xml c'e' stato un errore
            if(QString::compare(s_data.at(0), "<", Qt::CaseInsensitive) == 0)
            {
                QMessageBox::critical(this,"Error", "Error: \n"+bytes);
            }
            //Fare un controllo più serio, su quando non ci sono sed restituite
            else if (bytes.size()!=1187)
            {
                QString output_file=QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp");
                output_file.append("/tmp_download/").append("test_bm").append(".dat");

                QFile file(output_file);
                file.open(QIODevice::WriteOnly);
                file.write(bytes);
                file.close();

                Vialactea_FileLoad *fileload = new Vialactea_FileLoad(output_file,vtkwin);
                fileload->importBandMerged();
            }
            else
            {
                QMessageBox::information(this,"Alert", "No SED");

            }
            loading->loadingEnded();
            loading->hide();

            /*
               Vialactea_FileLoad *fileload = new Vialactea_FileLoad(output_file);
               fileload->setVtkWin(vtkwin);
               fileload->show();
*/

        }
        /* Clean up. */
        reply->deleteLater();
    }

}

QUrl VLKBQuery::redirectUrl(const QUrl& possibleRedirectUrl,   const QUrl& oldRedirectUrl) const {
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




VLKBQuery::~VLKBQuery()
{
    delete ui;
}
