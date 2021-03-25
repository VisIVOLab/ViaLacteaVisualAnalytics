#include "vialactea.h"
#include "ui_vialactea.h"
//#include <QWebFrame>
//#include <QWebElement>
#include "vialacteainitialquery.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QWebChannel>
#include "mainwindow.h"
#include "singleton.h"
#include <QSettings>
#include "authwrapper.h"
#include "settingform.h"
#include "aboutform.h"
#include "vlkbsimplequerycomposer.h"
#include "sed.h"
#include "sedvisualizerplot.h"
#include "vialacteastringdictwidget.h"

void WebProcess::jsCall(const QString &point,const QString &radius)
{
    emit processJavascript(point,radius);
}

const QString ViaLactea::ONLINE_TILE_PATH = "http://vlkb.dev.neanias.eu:8080/PanoramicView-v1.1.0/openlayers.html";

const QString ViaLactea::VLKB_URL_PUBLIC = "http://ia2-vialactea.oats.inaf.it/libjnifitsdb-1.0.2p/";
const QString ViaLactea::TAP_URL_PUBLIC = "http://ia2-vialactea.oats.inaf.it/vlkb/catalogues/tap";

const QString ViaLactea::VLKB_URL_PRIVATE = "http://ia2-vialactea.oats.inaf.it:8080/libjnifitsdb-1.0.2/";
const QString ViaLactea::TAP_URL_PRIVATE = "http://ia2-vialactea.oats.inaf.it:8080/vlkb";

const QString ViaLactea::VLKB_URL_NEANIAS = "http://vlkb.dev.neanias.eu:8080/vlkb-datasets-1.1/";
const QString ViaLactea::TAP_URL_NEANIAS = "http://vlkb.dev.neanias.eu:8080/vlkb/tap";

ViaLactea::ViaLactea(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ViaLactea)
{


    ui->setupUi(this);

    ui->saveToDiskCheckBox->setVisible(false);
    ui->fileNameLineEdit->setVisible(false);
    ui->selectFsPushButton->setVisible(false);



    //Cleanup previous run tmp
    QDir dir_tmp(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download"));
    foreach(QString dirFile, dir_tmp.entryList())
    {
        dir_tmp.remove(dirFile);
    }


    m_sSettingsFile = QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp").append("/setting.ini");

    updateVLKBSetting();

    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);

    if (settings.value("vlkbtype", "public").toString() == "neanias") {
        // The user has to login through SSO to continue or change access method
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setIcon(QMessageBox::Question);
        msgBox->setWindowTitle("Login required");
        msgBox->setInformativeText("You have selected private access to VLKB using NEANIAS SSO.\n\nSign in to continue using this private access or change access method in the Settings.");
        msgBox->addButton("Login", QMessageBox::AcceptRole);
        msgBox->addButton("Open settings", QMessageBox::DestructiveRole);
        msgBox->show();
        connect(msgBox, &QMessageBox::buttonClicked, [=](QAbstractButton *button){
            QMessageBox::ButtonRole btn = msgBox->buttonRole(button);
            if(btn == QMessageBox::AcceptRole)
            {
                // Open NEANIAS login page
                AuthWrapper *auth = &Singleton<AuthWrapper>::Instance();
                connect(auth, &AuthWrapper::authenticated,
                        &Singleton<VialacteaStringDictWidget>::Instance(), &VialacteaStringDictWidget::buildDict);
                auth->grant();
            }
            else
            {
                // Open settings window
                on_actionSettings_triggered();
            }
            msgBox->deleteLater();
        });
    }

    if (settings.value("online",true) == true)
    {
        tilePath = settings.value("onlinetilepath", ONLINE_TILE_PATH).toString();
        ui->webView->load(QUrl(tilePath));

    }
    else
    {
       tilePath = settings.value("tilepath", "").toString();
       ui->webView->load(QUrl::fromLocalFile(tilePath));

    }
    ui->webView->setContextMenuPolicy(Qt::CustomContextMenu);

    //TODO: receive a message clicked()
    //suggestion
    connect(ui->webView, SIGNAL(clicked()), this, SLOT(on_queryPushButton_clicked()));
    connect(ui->webView, SIGNAL(selectionChanged()), this, SLOT(textSelected()));
    //connect(ui->webView, SIGNAL(statusBarMessage(QString)),
    //           this, SIGNAL(on_webViewStatusBarMessage(QString)));
    //connect(ui->webView->page(), SIGNAL(statusBarMessage(QString)), this, SIGNAL(on_webViewStatusBarMessage(QString)));

    //create an object for javascript communication

    webobj = new WebProcess();
    QWebChannel *channel = new QWebChannel(this);
    channel->registerObject("webobj", webobj);
    ui->webView->page()->setWebChannel(channel);
    connect(webobj, SIGNAL(processJavascript(QString,QString)),
                   this, SLOT(on_webViewStatusBarMessage(QString,QString)));



    QObject::connect( this, SIGNAL(destroyed()), qApp, SLOT(quit()) );




    // VialacteaStringDictWidget *stringDictWidget = &Singleton<VialacteaStringDictWidget>::Instance();
    // stringDictWidget->buildDict();

    qDebug()<<"----------tilePath: "<<tilePath;

    mapSurvey.insert(0,QPair <QString, QString>("MIPSGAL","24 um"));
    mapSurvey.insert(1,QPair <QString, QString>("GLIMPSE I","8.0 um"));
    mapSurvey.insert(2,QPair <QString, QString>("GLIMPSE I","5.8 um"));
    mapSurvey.insert(3,QPair <QString, QString>("GLIMPSE I","4.5 um"));
    mapSurvey.insert(4,QPair <QString, QString>("GLIMPSE I","3.6 um"));
    mapSurvey.insert(5,QPair <QString, QString>("Hi-GAL","500 um"));
    mapSurvey.insert(6,QPair <QString, QString>("Hi-GAL","350 um"));
    mapSurvey.insert(7,QPair <QString, QString>("Hi-GAL","250 um"));
    mapSurvey.insert(8,QPair <QString, QString>("Hi-GAL","70 um"));
    mapSurvey.insert(9,QPair <QString, QString>("Hi-GAL","160 um"));
    mapSurvey.insert(10,QPair <QString, QString>("WISE","22 um"));
    mapSurvey.insert(11,QPair <QString, QString>("WISE","12 um"));
    mapSurvey.insert(12,QPair <QString, QString>("WISE","4.6 um"));
    mapSurvey.insert(13,QPair <QString, QString>("WISE","3.4 um"));
    mapSurvey.insert(14,QPair <QString, QString>("ATLASGAL","870 um"));
    mapSurvey.insert(15,QPair <QString, QString>("CSO BGPS","1.1 mm"));
    mapSurvey.insert(16,QPair <QString, QString>("CORNISH","5 GHz"));



}

ViaLactea::~ViaLactea()
{

    delete ui;
    delete  webobj;
}

void ViaLactea::quitApp()
{
//Problem not only in this
 QWebEnginePage *p =ui->webView->page();
 p->disconnect(ui->webView);
 delete p;
 std::cout<<"Deleted" << std::endl;


}

void ViaLactea::textSelected()
{

 std::cout<<"TextSelected" << std::endl;


}

void ViaLactea::updateVLKBSetting()
{

    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    QString vlkbtype = settings.value("vlkbtype", "public").toString();

    if (vlkbtype == "public")
    {
        qDebug()<<"public access to vlkb";
        settings.setValue("vlkburl", VLKB_URL_PUBLIC);
        settings.setValue("vlkbtableurl", TAP_URL_PUBLIC);
    }
    else if (vlkbtype == "private")
    {
        qDebug()<<"private access to vlkb";
        settings.setValue("vlkburl", VLKB_URL_PRIVATE);
        settings.setValue("vlkbtableurl", TAP_URL_PRIVATE);
    }
    else if (vlkbtype == "neanias") {
        qDebug() << "private access to vlkb through NEANIAS SSO";
        settings.setValue("vlkburl", VLKB_URL_NEANIAS);
        settings.setValue("vlkbtableurl", TAP_URL_NEANIAS);
    }
    settings.sync();

    VialacteaStringDictWidget *stringDictWidget = &Singleton<VialacteaStringDictWidget>::Instance();
    stringDictWidget->buildDict();
}

void ViaLactea::on_queryPushButton_clicked()
{

    VialacteaInitialQuery *vq;
    if (ui->saveToDiskCheckBox->isChecked())
    {
        if( ui->fileNameLineEdit->text()!="" )
            vq= new VialacteaInitialQuery(ui->fileNameLineEdit->text());
        else
        {
            QMessageBox::critical(this,"Error", "Insert filename");
            return;
        }
    }
    else
        vq= new VialacteaInitialQuery();

    vq->setL(ui->glonLineEdit->text());
    vq->setB(ui->glatLineEdit->text());
    if (ui->radiumLineEdit->text()!="")
        vq->setR(ui->radiumLineEdit->text());
    else
    {
        vq->setDeltaRect(ui->dlLineEdit->text(),ui->dbLineEdit->text());
    }

    QList < QPair<QString, QString> > selectedSurvey;

    QList<QCheckBox *> allButtons = ui->surveySelectorGroupBox->findChildren<QCheckBox *>();
    for(int i = 0; i < allButtons.size(); ++i)
    {
        qDebug()<<"i: "<<i<<" "<<allButtons.at(i);

        if(allButtons.at(i)->isChecked())
        {


            selectedSurvey.append(mapSurvey.value(i));
        }
    }

    //connettere la banda selezionata
    vq->setSpecies("Continuum");
    vq->setSurveyname(selectedSurvey.at(0).first);
    vq->setTransition(selectedSurvey.at(0).second);
    vq->setSelectedSurveyMap(selectedSurvey);
    vq->on_queryPushButton_clicked();


}


void ViaLactea::on_noneRadioButton_clicked(bool checked)
{
    if(checked)
    {
        ui->webView->page()->runJavaScript( "activatePointSelection(false)" );
        ui->webView->page()->runJavaScript( "activateRectangularSelection(false)" );
    }


}

void ViaLactea::on_saveToDiskCheckBox_clicked(bool checked)
{
    ui->fileNameLineEdit->setEnabled(checked);
    ui->selectFsPushButton->setEnabled(checked);
}

void ViaLactea::on_selectFsPushButton_clicked()
{
    QString fn = QFileDialog::getSaveFileName(this, "Save as...", QString(), "Fits images (*.fits)");

    if (!fn.isEmpty() && ! fn.endsWith(".fits", Qt::CaseInsensitive)  )
        fn += ".fits"; // default
    ui->fileNameLineEdit->setText(fn);


}


void ViaLactea::on_webViewStatusBarMessage( const QString &point, const QString &radius)
{

//    QObject e = ui->webView->page()-> ( ->findChild("div#selected_point");
   // QString result;
   /*ui->webView->page()->runJavaScript("function myFunction() {"
                                        "var el = document.getElementById('div#selected_point');"
                                        "return el;} myFunction();",
                                        [] (const QVariant &result) {
                   return result.toString();
      });*/

           const QString e=point;

             if (e!="")
             {
                 QStringList pieces = e.split( "," );
                 ui->glatLineEdit->setText(QString::number( pieces[1].toDouble(), 'f', 4 ));
                 ui->glonLineEdit->setText(QString::number( pieces[0].toDouble(), 'f', 4 ));
                 if(ui->radiumLineEdit->text()=="")
                     ui->radiumLineEdit->setText("0.1");
                 ui->dlLineEdit->setText("");
                 ui->dbLineEdit->setText("");

                 ui->noneRadioButton->setChecked(true);
                 on_noneRadioButton_clicked(true);
             }
/*     qDebug()<<"e: "<<e;
   ui->webView->page()->runJavaScript("function myFunction() {"
                                        "var el = document.getElementById('div#selected_radius');"
                                        "return el;} myFunction();",
                                        [] (const QVariant &result) {
                   return result.toString();
      });*/
            QString e_radius=radius;

//    QWebElement e_radius = ui->webView->page()->mainFrame()->findFirstElement("div#selected_radius");

    // qDebug()<<"e_radius: "<<e_radius.toPlainText();
    if (e_radius!="")
    {
        QStringList pieces = e_radius.split( "," );
        // qDebug()<<pieces;
        QString dl=QString::number( pieces[0].toDouble(), 'f', 4 );
        if(dl.toDouble()>4.0)
            dl=QString::number(4.0, 'f', 4 );
        QString db=QString::number( pieces[1].toDouble(), 'f', 4 );
        if(db.toDouble()>4.0)
            db=QString::number(4.0, 'f', 4 );
        ui->dlLineEdit->setText(dl);
        ui->dbLineEdit->setText(db);
        //  if(ui->radiumLineEdit->text()=="")
        ui->radiumLineEdit->setText("");
        //  ui->noneRadioButton->setChecked(true);
        //  on_noneRadioButton_clicked(true);
    }

}

void ViaLactea::on_glonLineEdit_textChanged(const QString &arg1)
{
    queryButtonStatusOnOff();
}

void ViaLactea::on_glatLineEdit_textChanged(const QString &arg1)
{
    queryButtonStatusOnOff();
}

void ViaLactea::on_radiumLineEdit_textChanged(const QString &arg1)
{
    queryButtonStatusOnOff();
}

void ViaLactea::queryButtonStatusOnOff()
{
    if (ui->glatLineEdit->text()!="" && ui->glonLineEdit->text()!="" && (ui->radiumLineEdit->text()!="" || (ui->dlLineEdit->text()!="" && ui->dbLineEdit->text()!="") ) )
        ui->queryPushButton->setEnabled(true);
    else
        ui->queryPushButton->setEnabled(false);
}

void ViaLactea::on_openLocalImagePushButton_clicked()
{
    QString fn = QFileDialog::getOpenFileName(this, "Open image file", QString(), "Fits images (*.fits)");

    if (!fn.isEmpty() )
    {
        MainWindow *w = &Singleton<MainWindow>::Instance();
        w->importFitsImage(fn);
    }
}



void ViaLactea::on_actionSettings_triggered()
{
    SettingForm *s = &Singleton<SettingForm>::Instance();
    s->readSettingsFromFile();
    s->show();

    s->activateWindow();

}

void ViaLactea::reload()
{
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);


    //tilePath = settings.value("tilepath", "").toString();

    if (settings.value("online",false) == true)
    {
        tilePath = settings.value("onlinetilepath", "").toString();
        ui->webView->load(QUrl(tilePath));

    }
    else
    {
       tilePath = settings.value("tilepath", "").toString();
       ui->webView->load(QUrl::fromLocalFile(tilePath));


    }
    updateVLKBSetting();



}

void ViaLactea::on_localDCPushButton_clicked()
{

    MainWindow *w = &Singleton<MainWindow>::Instance();
    w->on_actionTEST_DC3D_triggered();
}

void ViaLactea::on_actionExit_triggered()
{
    // QCoreApplication::exit(0);

    this->close();
}

void   ViaLactea::closeEvent(QCloseEvent*)
{

    //quitApp();
    qApp->closeAllWindows();
    //qApp->quit();

}

void ViaLactea::on_actionAbout_triggered()
{
    AboutForm *w = &Singleton<AboutForm>::Instance();
    w->show();
}

void ViaLactea::on_select3dPushButton_clicked()
{
    VLKBSimpleQueryComposer *skyregionquery = new VLKBSimpleQueryComposer(NULL);
    skyregionquery->setIs3dSelections();

    skyregionquery->setLongitude(0,360);
    skyregionquery->setLatitude(-1,1);
    skyregionquery->show();
}

void ViaLactea::on_actionLoad_SED_2_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load SED fits"), QDir::homePath(), tr("Archive (*.zip)"));
    if (fileName.isEmpty())
        return;

    QString sedZipPath=QDir::homePath()+"/VisIVODesktopTemp/tmp_download/SED.zip";
    QFile::copy(fileName, sedZipPath);
    QProcess process_unzip;
    process_unzip.start ("unzip "+sedZipPath+" -d "+QDir::homePath()+"/VisIVODesktopTemp/tmp_download");

    process_unzip.waitForFinished(); // sets current thread to sleep and waits for process end
    QString output_unzip(process_unzip.readAll());


    QDir tmp_download(QDir::homePath()+"/VisIVODesktopTemp/tmp_download");
    QStringList filters;
    filters << "SED*";
    tmp_download.setNameFilters(filters);
    QStringList dirList=tmp_download.entryList();


    QFile sedFile(QDir::homePath().append(QDir::separator()).append("/VisIVODesktopTemp/tmp_download/SEDList.dat"));
    sedFile.open(QIODevice::ReadOnly);
    QDataStream in(&sedFile);    // read the data serialized from the file
    QList<SED *> sed_list2;
    in >> sed_list2;
    /*
    foreach(SED* sed, sed_list2){
        qDebug()<<"SED Designation";
        qDebug()<<sed->getRootNode()->getDesignation();
    }
*/

    ViaLactea *vialactealWin = &Singleton<ViaLactea>::Instance();
    SEDVisualizerPlot *sedv= new SEDVisualizerPlot(sed_list2,0,vialactealWin);
    sedv->show();
    sedv->loadSavedSED(dirList);
}


void ViaLactea::on_pointRadioButton_clicked(bool checked)
{
    if(checked)
    {
        ui->webView->page()->runJavaScript( "activatePointSelection(true)" );
    }
}

void ViaLactea::on_rectRadioButton_clicked(bool checked)
{
    if(checked)
    {
        ui->webView->page()->runJavaScript("activateRectangularSelection(true)" );
    }
}

void ViaLactea::on_dlLineEdit_textChanged(const QString &arg1)
{
    queryButtonStatusOnOff();
}

void ViaLactea::on_dbLineEdit_textChanged(const QString &arg1)
{
    queryButtonStatusOnOff();
}
