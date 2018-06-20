#ifndef VIALACTEASTRINGDICTWIDGET_H
#define VIALACTEASTRINGDICTWIDGET_H

#include <QHash>
#include <QWidget>

#include <QNetworkAccessManager>
#include <QNetworkRequest>

namespace Ui {
class VialacteaStringDictWidget;
}

class VialacteaStringDictWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VialacteaStringDictWidget(QWidget *parent = 0);
    void buildDict();
    ~VialacteaStringDictWidget();
    QHash<QString, QString> getColUtypeStringDict(){return colUtypeStringDict;}
    QHash<QString, QString> getColDescStringDict(){return colDescStringDict;}
    QHash<QString, QString> getTableDescStringDict(){return tableDescStringDict;}
    QHash<QString, QString> getTableUtypeStringDict(){return tableUtypeStringDict;}
    QHash<QString, QString> tableUtypeStringDict;
    QHash<QString, QString> tableDescStringDict;

    QHash<QString, QString> colUtypeStringDict;
    QHash<QString, QString> colDescStringDict;

private:
    Ui::VialacteaStringDictWidget *ui;

    QUrl urlRedirectedTo;
    QNetworkAccessManager *manager;
    QString m_sSettingsFile;

private slots:
    void availReplyFinished (QNetworkReply *reply);
    void  executeQueryTapSchemaTables();
    void executeQueryTapSchemaColumns();
    void onAuthenticationRequestSlot(QNetworkReply *aReply, QAuthenticator *aAuthenticator);
    void queryReplyFinishedTapSchemaTables (QNetworkReply *reply);
    void queryReplyFinishedTapSchemaColumns(QNetworkReply *reply);
    QUrl redirectUrl(const QUrl& possibleRedirectUrl, const QUrl& oldRedirectUrl) const;




};

#endif // VIALACTEASTRINGDICTWIDGET_H
