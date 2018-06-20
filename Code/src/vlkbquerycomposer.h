#ifndef VLKBQUERYCOMPOSER_H
#define VLKBQUERYCOMPOSER_H

#include <QWidget>
#include <QNetworkReply>
#include "vlkbtable.h"

namespace Ui {
class VLKBQueryComposer;
}

class VLKBQueryComposer : public QWidget
{
    Q_OBJECT

public:
    explicit VLKBQueryComposer(QWidget *parent = 0);
    ~VLKBQueryComposer();

private:
    Ui::VLKBQueryComposer *ui;
    QString url;
    QString m_sSettingsFile;
    bool available=false;
    QList<VlkbTable *> table_list;
     QNetworkAccessManager * manager;
     QUrl urlRedirectedTo;

public slots:
    void tableReplyFinished (QNetworkReply *reply);
    void availReplyFinished (QNetworkReply *reply);


private slots:
    void on_connectButton_clicked();
    void checkAvailability();

    void on_tableListComboBox_currentIndexChanged(int index);
    void on_okButton_clicked();
    void onAuthenticationRequestSlot(QNetworkReply *aReply, QAuthenticator *aAuthenticator);
    void queryReplyFinished (QNetworkReply *reply);
    QUrl redirectUrl(const QUrl& possibleRedirectUrl, const QUrl& oldRedirectUrl) const;

};

#endif // VLKBQUERYCOMPOSER_H
