#include "vialacteainitialquery.h"
#include "ui_vialacteainitialquery.h"

#include "astroutils.h"
#include "authwrapper.h"
#include "downloadmanager.h"
#include "loadingwidget.h"
#include "mainwindow.h"
#include "vtkwindow_new.h"
#include "vtkwindowcube.h"
#include "xmlparser.h"

VialacteaInitialQuery::VialacteaInitialQuery(QString fn, QWidget *parent)
    : QWidget(parent), ui(new Ui::VialacteaInitialQuery)
{
    ui->setupUi(this);

    ui->rectGroupBox->hide();

    QSettings settings(QDir::homePath()
                               .append(QDir::separator())
                               .append("VisIVODesktopTemp")
                               .append(QDir::separator())
                               .append("setting.ini"),
                       QSettings::IniFormat);
    vlkbUrl = settings.value("vlkburl").toString();

    nam = new QNetworkAccessManager(this);
    QObject::connect(nam, SIGNAL(finished(QNetworkReply *)), this,
                     SLOT(finishedSlot(QNetworkReply *)));

    parser = new xmlparser();
    loading = new LoadingWidget();
    outputFile = fn;
    species = "";
    p = parent;
    myCallingVtkWindow = 0;

    transitions.insert("PLW", "500um");
    transitions.insert("PMW", "350um");
    transitions.insert("PSW", "250um");
    transitions.insert("red", "160um");
    transitions.insert("blue", "70um");
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
    if (checked) {
        ui->rectGroupBox->hide();
        ui->pointGroupBox->show();
    } else {
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
    ui->b_lineEdit->setText(b.replace(" ", ""));
}
void VialacteaInitialQuery::setR(QString r)
{
    isRadius = true;
    ui->r_lineEdit->setText(r);
}

void VialacteaInitialQuery::setDeltaRect(QString dl, QString db)
{
    isRadius = false;
    ui->dlLineEdit->setText(dl);
    ui->dbLineEdit->setText(db);
}

void VialacteaInitialQuery::setSurveyname(QString s)
{
    surveyname = s;
}

void VialacteaInitialQuery::setTransition(QString s)
{
    transition = s;
}

QString VialacteaInitialQuery::posCutoutString(double l, double b, double r)
{
    if (l < 0)
        l += 360.;

    return QString("CIRCLE %1 %2 %3")
            .arg(QString::number(l), QString::number(b), QString::number(r));
}

QString VialacteaInitialQuery::posCutoutString(double l1, double l2, double b1, double b2)
{
    if (l1 < 0)
        l1 += 360.;
    if (l2 < 0)
        l2 += 360.;

    return QString("RANGE %1 %2 %3 %4")
            .arg(QString::number(l1), QString::number(l2), QString::number(b1),
                 QString::number(b2));
}

void VialacteaInitialQuery::searchRequest(double l, double b, double dl, double db)
{
    double l1 = l - dl;
    if (l1 < 0)
        l1 += 360.;

    double l2 = l + dl;
    if (l2 < 0)
        l2 += 360.;

    QString range = QString("RANGE %1 %2 %3 %4")
                            .arg(QString::number(l1), QString::number(l2), QString::number(b - db),
                                 QString::number(b + db));

    QUrlQuery q;
    q.addQueryItem("POS", range);
    q.addQueryItem("POSSYS", "GALACTIC");

    QUrl url(this->vlkbUrl + "/siav2/query");
    url.setQuery(q);

    qDebug() << Q_FUNC_INFO << url.toString();
    this->searchRequest(url.toString());
}

void VialacteaInitialQuery::cutoutRequest(const QString &id, const QDir &dir, double l1, double l2,
                                          double b1, double b2, const Cutout &src)
{
    if (l1 < 0)
        l1 += 360.;

    if (l2 < 0)
        l2 += 360.;

    QString range = QString("RANGE %1 %2 %3 %4")
                            .arg(QString::number(l1), QString::number(l2), QString::number(b1),
                                 QString::number(b2));

    QUrlQuery q;
    q.addQueryItem("ID", id);
    q.addQueryItem("POS", range);
    q.addQueryItem("POSSYS", "GALACTIC");

    QUrl url(this->vlkbUrl + "/soda/sync");
    url.setQuery(q);

    qDebug() << Q_FUNC_INFO << url.toString();
    this->cutoutRequest(url.toString(), dir, src);
}

void VialacteaInitialQuery::searchRequest(double l, double b, double r)
{
    if (l < 0)
        l += 360.;

    QString circle = QString("CIRCLE %1 %2 %3")
                             .arg(QString::number(l), QString::number(b), QString::number(r));

    QUrlQuery q;
    q.addQueryItem("POS", circle);
    q.addQueryItem("POSSYS", "GALACTIC");

    QUrl url(this->vlkbUrl + "/siav2/query");
    url.setQuery(q);

    qDebug() << Q_FUNC_INFO << url.toString();
    this->searchRequest(url.toString());
}

void VialacteaInitialQuery::cutoutRequest(const QString &id, const QDir &dir, double l, double b,
                                          double r, const Cutout &src)
{
    if (l < 0)
        l += 360.;

    QString circle = QString("CIRCLE %1 %2 %3")
                             .arg(QString::number(l), QString::number(b), QString::number(r));

    QUrlQuery q;
    q.addQueryItem("ID", id);
    q.addQueryItem("POS", circle);
    q.addQueryItem("POSSYS", "GALACTIC");

    QUrl url(this->vlkbUrl + "/soda/sync");
    url.setQuery(q);

    qDebug() << Q_FUNC_INFO << url.toString();
    this->cutoutRequest(url.toString(), dir, src);
}

void VialacteaInitialQuery::cutoutRequest(const QString &id, const QDir &dir, const QString &pos,
                                          const Cutout &src)
{
    QUrlQuery q;
    q.addQueryItem("ID", id);
    q.addQueryItem("POS", pos);
    q.addQueryItem("POSSYS", "GALACTIC");

    QUrl url(this->vlkbUrl + "/soda/sync");
    url.setQuery(q);

    qDebug() << Q_FUNC_INFO << url.toString();
    this->cutoutRequest(url.toString(), dir, src);
}

void VialacteaInitialQuery::searchRequest(const QString &url)
{
    loading->setText("Querying search service");
    loading->show();
    loading->activateWindow();

    auto nam = new QNetworkAccessManager(this);
    connect(nam, &QNetworkAccessManager::finished, this, [this, nam](QNetworkReply *reply) {
        reply->deleteLater();
        nam->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
            emit this->searchDoneVO(reply->readAll());
        } else {
            QMessageBox::critical(nullptr, QObject::tr("Error"),
                                  QObject::tr(qPrintable(reply->errorString())));
        }

        loading->loadingEnded();
        loading->hide();
    });

    QNetworkRequest req(url);
    IA2VlkbAuth::Instance().putAccessToken(req);

    QNetworkReply *reply = nam->get(req);
    loading->setLoadingProcess(reply);
}

void VialacteaInitialQuery::cutoutRequest(const QString &url, const QDir &dir, const Cutout &src)
{
    loading->setText("Querying cutout service");
    loading->show();
    loading->activateWindow();

    auto nam = new QNetworkAccessManager(this);

    QString reqUrl = QUrl::toPercentEncoding(url, { ":/&?=" }, { " " });
    QNetworkRequest req(reqUrl);
    IA2VlkbAuth::Instance().putAccessToken(req);

    auto reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, nam, reply, dir, src]() {
        reply->deleteLater();
        nam->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
            QString filepath =
                    dir.absoluteFilePath(QUuid::createUuid().toString(QUuid::WithoutBraces))
                            .append(".fits");

            QFile f(filepath);
            f.open(QIODevice::WriteOnly);
            f.write(reply->readAll());
            f.close();
            emit this->cutoutDone(filepath, src);
        } else {
            QMessageBox::critical(nullptr, "Error", reply->readAll());
        }

        loading->loadingEnded();
        loading->hide();
    });
    loading->setLoadingProcess(reply);
}

void VialacteaInitialQuery::selectedStartingLayersRequest(QUrl url)
{
    loading->show();
    loading->setText("Querying cutout services");

    QSettings settings(QDir::homePath()
                               .append(QDir::separator())
                               .append("VisIVODesktopTemp")
                               .append(QDir::separator())
                               .append("setting.ini"),
                       QSettings::IniFormat);

    QNetworkRequest req(url);

    IA2VlkbAuth::Instance().putAccessToken(req);

    QNetworkReply *reply = nam->get(req);
    loading->setLoadingProcess(reply);
}
