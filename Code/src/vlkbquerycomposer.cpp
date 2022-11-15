#include "vlkbquerycomposer.h"
#include "ui_vlkbquerycomposer.h"

#include "authkeys.h"
#include "authwrapper.h"
#include "singleton.h"
#include "vlkbtable.h"

#include <QAuthenticator>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QDomNodeList>
#include <QFile>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QSettings>
#include <QUrl>

VLKBQueryComposer::VLKBQueryComposer(QWidget *parent)
    : QWidget(parent), ui(new Ui::VLKBQueryComposer)
{
    ui->setupUi(this);

    QHeaderView *header = ui->columnTableWidget->horizontalHeader();
    header->setVisible(true);
    header->sectionResizeMode(QHeaderView::Stretch);

    m_sSettingsFile = QDir::homePath()
            .append(QDir::separator())
            .append("VisIVODesktopTemp")
            .append("/setting.ini");
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);

    url = settings.value("vlkbtableurl", "").toString();
}

void VLKBQueryComposer::tableReplyFinished(QNetworkReply *reply)
{

    if (reply->error()) {
    } else {
        QDomDocument doc;
        doc.setContent(reply->readAll());
        QDomNodeList list = doc.elementsByTagName("table");

        for (int i = 0; i < list.size(); i++) {
            VlkbTable *table = new VlkbTable();
            QDomElement node = list.at(i).toElement();
            for (int j = 0; j < node.childNodes().size(); j++) {
                if (node.childNodes().at(j).toElement().tagName() == "name") {
                    table->setName(node.childNodes().at(j).toElement().text());
                } else if (node.childNodes().at(j).toElement().tagName() == "column") {
                    table->addColumn(node.childNodes()
                                     .at(j)
                                     .toElement()
                                     .childNodes()
                                     .at(0)
                                     .toElement()
                                     .text(),
                                     node.childNodes()
                                     .at(j)
                                     .toElement()
                                     .childNodes()
                                     .at(1)
                                     .toElement()
                                     .text());
                }
            }
            table_list.append(table);
            ui->tableListComboBox->addItem(table->getName());
        }
    }

    reply->deleteLater();
}

VLKBQueryComposer::~VLKBQueryComposer()
{
    delete ui;
}
void VLKBQueryComposer::availReplyFinished(QNetworkReply *reply)
{
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    if (reply->error()) {
        ui->tableListComboBox->setEnabled(false);
        ui->columnTableWidget->setEnabled(false);
        ui->syncCheckBox->setEnabled(false);
        ui->queryLangComboBox->setEnabled(false);
        ui->queryTextEdit->setEnabled(false);
        ui->okButton->setEnabled(false);
        available = false;
    } else {
        QDomDocument doc;
        doc.setContent(reply->readAll());

        QDomNodeList list = doc.elementsByTagName("vosi:available");
        if (list.at(0).toElement().text() == "true") {
            ui->tableListComboBox->setEnabled(true);
            ui->columnTableWidget->setEnabled(true);
            ui->syncCheckBox->setEnabled(true);
            ui->queryLangComboBox->setEnabled(true);
            ui->queryTextEdit->setEnabled(true);
            ui->okButton->setEnabled(true);
            available = true;
            QNetworkAccessManager *manager = new QNetworkAccessManager(this);
            connect(manager, SIGNAL(finished(QNetworkReply *)), this,
                    SLOT(tableReplyFinished(QNetworkReply *)));
            QNetworkRequest req(QUrl(url + "/tables"));
            auto auth = settings.value("vlkbtype", "ia2") == "ia2" ? &IA2VlkbAuth::Instance()
                                                                   : &NeaniasVlkbAuth::Instance();
            auth->putAccessToken(req);
            manager->get(req);
        }
    }

    reply->deleteLater();
}

void VLKBQueryComposer::checkAvailability()
{
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(availReplyFinished(QNetworkReply *)));
    QNetworkRequest req(QUrl(url + "/availability"));
    auto auth = settings.value("vlkbtype", "ia2") == "ia2" ? &IA2VlkbAuth::Instance()
                                                           : &NeaniasVlkbAuth::Instance();
    auth->putAccessToken(req);
    manager->get(req);
}

void VLKBQueryComposer::on_connectButton_clicked()
{
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(availReplyFinished(QNetworkReply *)));
    QNetworkRequest req(QUrl(url + "/availability"));
    auto auth = settings.value("vlkbtype", "ia2") == "ia2" ? &IA2VlkbAuth::Instance()
                                                           : &NeaniasVlkbAuth::Instance();
    auth->putAccessToken(req);
    manager->get(req);
}

void VLKBQueryComposer::on_tableListComboBox_currentIndexChanged(int index)
{
    int row;
    for (int i = 0; i < table_list.at(index)->getColumnList().size(); i++) {
        row = ui->columnTableWidget->model()->rowCount();
        ui->columnTableWidget->insertRow(row);

        QTableWidgetItem *name_item = new QTableWidgetItem();
        name_item->setText(table_list.at(index)->getColumnList().at(i).name);
        ui->columnTableWidget->setItem(row, 0, name_item);

        QTableWidgetItem *datatype_item = new QTableWidgetItem();
        datatype_item->setText(table_list.at(index)->getColumnList().at(i).type);
        ui->columnTableWidget->setItem(row, 1, datatype_item);
    }
}

void VLKBQueryComposer::on_okButton_clicked()
{
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    manager = new QNetworkAccessManager(this);
    QByteArray postData;
    postData.append("REQUEST=doQuery&");
    postData.append("VERSION=1.0&");
    postData.append("LANG=ADQL&");
    postData.append("FORMAT=tsv&");
    postData.append("QUERY=SELECT%20*%20FROM%20vlkb_compactsources.band500um%20WHERE%20(glon>=316."
                    "009%20and%20glon<=317.178)%20AND%20(glat>=-0.78796%20and%20glat<=0.68552)");

    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this,
            SLOT(onAuthenticationRequestSlot(QNetworkReply *, QAuthenticator *)));
    connect(manager, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(queryReplyFinished(QNetworkReply *)));
    QNetworkRequest req(url + "/sync");
    auto auth = settings.value("vlkbtype", "ia2") == "ia2" ? &IA2VlkbAuth::Instance()
                                                           : &NeaniasVlkbAuth::Instance();
    auth->putAccessToken(req);
    manager->post(req, postData);
}

void VLKBQueryComposer::onAuthenticationRequestSlot(QNetworkReply *aReply,
                                                    QAuthenticator *aAuthenticator)
{
    Q_UNUSED(aReply);
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    if (settings.value("vlkbtype", "ia2").toString() == "ia2") {
        aAuthenticator->setUser(IA2_TAP_USER);
        aAuthenticator->setPassword(IA2_TAP_PASS);
    }
}

void VLKBQueryComposer::queryReplyFinished(QNetworkReply *reply)
{
    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    if (reply->error()) {
    } else {
        QVariant possibleRedirectUrl =
                reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

        /* We'll deduct if the redirection is valid in the redirectUrl function */
        urlRedirectedTo = redirectUrl(possibleRedirectUrl.toUrl(), urlRedirectedTo);
        /* If the URL is not empty, we're being redirected. */
        if (!urlRedirectedTo.isEmpty()) {
            /* We'll do another request to the redirection url. */
            QNetworkRequest req(urlRedirectedTo);
            auto auth = settings.value("vlkbtype", "ia2") == "ia2" ? &IA2VlkbAuth::Instance()
                                                                   : &NeaniasVlkbAuth::Instance();
            auth->putAccessToken(req);
            manager->get(req);
        } else {
            QByteArray bytes = reply->readAll();
            QFile file("/Users/fxbio6600/test_out.txt");
            file.open(QIODevice::WriteOnly);
            file.write(bytes);
            file.close();
        }
        /* Clean up. */
        reply->deleteLater();
    }
}

QUrl VLKBQueryComposer::redirectUrl(const QUrl &possibleRedirectUrl,
                                    const QUrl &oldRedirectUrl) const
{
    QUrl redirectUrl;
    /*
     * Check if the URL is empty and
     * that we aren't being fooled into a infinite redirect loop.
     * We could also keep track of how many redirects we have been to
     * and set a limit to it, but we'll leave that to you.
     */
    if (!possibleRedirectUrl.isEmpty() && possibleRedirectUrl != oldRedirectUrl) {
        redirectUrl = possibleRedirectUrl;
    }

    return redirectUrl;
}
