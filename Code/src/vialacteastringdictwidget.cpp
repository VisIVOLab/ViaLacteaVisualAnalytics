#include "vialacteastringdictwidget.h"
#include "ui_vialacteastringdictwidget.h"

#include <QSettings>
#include <QDir>
#include <QNetworkReply>
#include <QMessageBox>
#include <QDomDocument>
#include <QAuthenticator>
#include <QNetworkRequest>

VialacteaStringDictWidget::VialacteaStringDictWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VialacteaStringDictWidget)
{
    ui->setupUi(this);

}

void VialacteaStringDictWidget::buildDict()
{
    m_sSettingsFile = QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp").append("/setting.ini");


    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)),  this, SLOT(availReplyFinished(QNetworkReply*)));

    QNetworkRequest request(QUrl(settings.value("vlkbtableurl", "").toString()+"/availability"));
//    QNetworkRequest request(QUrl("http://ia2-vialactea.oats.inaf.it/vlkb/availability"));


    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("text/html; charset=utf-8"));
    manager->get(request);



}




void VialacteaStringDictWidget::availReplyFinished (QNetworkReply *reply)
{


    if(reply->error())
    {

        QMessageBox::critical(this,"Error", "Error: \n"+reply->errorString());



    }
    else
    {
        QDomDocument doc;
        doc.setContent(reply->readAll());
        QDomNodeList list=doc.elementsByTagName("vosi:available");

        if(list.at(0).toElement().text()=="true")
        {

            executeQueryTapSchemaTables();
            executeQueryTapSchemaColumns();

        }
    }

    reply->deleteLater();

}

void VialacteaStringDictWidget::executeQueryTapSchemaTables()
{

    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);

    QString query="SELECT * FROM TAP_SCHEMA.tables";

    manager = new QNetworkAccessManager(this);

    QByteArray postData;

    postData.append("REQUEST=doQuery&");
    postData.append("VERSION=1.0&");
    postData.append("LANG=ADQL&");
    postData.append("FORMAT=tsv&");


    postData.append("QUERY="+QUrl::toPercentEncoding(query));


    connect ( manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(onAuthenticationRequestSlot(QNetworkReply*,QAuthenticator*)) );


    connect(manager, SIGNAL(finished(QNetworkReply*)),  this, SLOT(queryReplyFinishedTapSchemaTables(QNetworkReply*)));


    QString url = settings.value("vlkbtableurl", "").toString()+"/sync";
   // QString url = "http://ia2-vialactea.oats.inaf.it:8080/vlkb/sync";


    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    manager->post(request,postData);

}

void VialacteaStringDictWidget::executeQueryTapSchemaColumns()
{

    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);

    QString query="SELECT * FROM TAP_SCHEMA.columns";

    manager = new QNetworkAccessManager(this);

    QByteArray postData;

    postData.append("REQUEST=doQuery&");
    postData.append("VERSION=1.0&");
    postData.append("LANG=ADQL&");
    postData.append("FORMAT=tsv&");


    postData.append("QUERY="+QUrl::toPercentEncoding(query));


    connect ( manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(onAuthenticationRequestSlot(QNetworkReply*,QAuthenticator*)) );


    connect(manager, SIGNAL(finished(QNetworkReply*)),  this, SLOT(queryReplyFinishedTapSchemaColumns(QNetworkReply*)));


    QString url = settings.value("vlkbtableurl", "").toString()+"/sync";
   // QString url = "http://ia2-vialactea.oats.inaf.it:8080/vlkb/sync";

    qDebug()<<url;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    manager->post(request,postData);

}


void VialacteaStringDictWidget::queryReplyFinishedTapSchemaTables (QNetworkReply *reply)
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

            QByteArray bytes = reply->readLine();
            QString s_data = QString::fromLatin1(bytes.data());
            //se inizia per '<' è un xml, se è xml c'e' stato un errore
            if(QString::compare(s_data.at(0), "<", Qt::CaseInsensitive) == 0)
            {
                QMessageBox::critical(this,"Error", "Error: \n"+bytes);
            }
            else
            {
                QString line_data;
                while(! reply->atEnd())
                {
                     line_data = QString::fromLatin1(reply->readLine().data());

                     QStringList list2=line_data.split('\t');

                     tableUtypeStringDict.insert(list2[3],list2[5].remove(QRegExp("[\n\t\r]")));
                     tableDescStringDict.insert(list2[3],list2[0].remove(QRegExp("[\n\t\r]")));



                }

             //   qDebug()<<tableUtypeStringDict.value("vlkb_compactsources.band500um");
             //   qDebug()<<tableDescStringDict.value("vlkb_compactsources.band500um");







            }

        }
        /* Clean up. */
        reply->deleteLater();

    }


}

void VialacteaStringDictWidget::queryReplyFinishedTapSchemaColumns (QNetworkReply *reply)
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

            QByteArray bytes = reply->readLine();
            QString s_data = QString::fromLatin1(bytes.data());
            //se inizia per '<' è un xml, se è xml c'e' stato un errore
            if(QString::compare(s_data.at(0), "<", Qt::CaseInsensitive) == 0)
            {
                QMessageBox::critical(this,"Error", "Error: \n"+bytes);
            }
            else
            {
                QString line_data;
                while(! reply->atEnd())
                {
                     line_data = QString::fromLatin1(reply->readLine().data());

                     QStringList list2=line_data.split('\t');


                     colUtypeStringDict.insert(list2[8] +"."+list2[1],list2[11].remove(QRegExp("[\n\t\r]")));
                     colDescStringDict.insert(list2[8] +"."+list2[1],list2[3].remove(QRegExp("[\n\t\r]")));

                }

            //     qDebug()<<"AUUUUUUUUUUUUUU"<<colUtypeStringDict.value("vlkb_filaments.branches.contour");
            //     qDebug()<<"AUUUUUUUUUUUUUU"<<colDescStringDict.value("vlkb_filaments.branches.contour");


            }

        }
        reply->deleteLater();

    }


}


QUrl VialacteaStringDictWidget::redirectUrl(const QUrl& possibleRedirectUrl,   const QUrl& oldRedirectUrl) const {
    QUrl redirectUrl;

    if(!possibleRedirectUrl.isEmpty() && possibleRedirectUrl != oldRedirectUrl) {
        redirectUrl = possibleRedirectUrl;
    }

    return redirectUrl;
}


void VialacteaStringDictWidget::onAuthenticationRequestSlot(QNetworkReply *aReply, QAuthenticator *aAuthenticator)
{
    aAuthenticator->setUser("vialactea");
    aAuthenticator->setPassword("secret");
}





VialacteaStringDictWidget::~VialacteaStringDictWidget()
{
    delete ui;
}
