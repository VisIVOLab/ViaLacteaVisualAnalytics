#ifndef VLKBSIMPLEQUERYCOMPOSER_H
#define VLKBSIMPLEQUERYCOMPOSER_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "vlkbtable.h"
#include "loadingwidget.h"
#include "vtkwindow_new.h"
namespace Ui {
class VLKBSimpleQueryComposer;
}

class VLKBSimpleQueryComposer : public QWidget
{
    Q_OBJECT

public:
    explicit VLKBSimpleQueryComposer(vtkwindow_new *v, QWidget *parent = 0);
    ~VLKBSimpleQueryComposer();
    void setLongitude(float long1, float long2);
    void setLatitude(float lat1, float lat2);
    void setVtkWindow(vtkwindow_new *v);
    void setIsFilament();
    void setIs3dSelections();
    void setIsBubble();


private slots:
    void on_connectPushButton_clicked();
    void availReplyFinished (QNetworkReply *reply);
    void tableReplyFinished (QNetworkReply *reply);
    void queryReplyFinished (QNetworkReply *reply);
    void onAuthenticationRequestSlot(QNetworkReply *aReply, QAuthenticator *aAuthenticator);
    void on_queryPushButton_clicked();
    QUrl redirectUrl(const QUrl& possibleRedirectUrl, const QUrl& oldRedirectUrl) const;
    void on_tableComboBox_currentIndexChanged(int index);
    void on_savedatasetCheckBox_clicked(bool checked);
    void on_navigateFSButtono_clicked();
    void closeSlot();
    void closeEvent ( QCloseEvent * event );

private:
    Ui::VLKBSimpleQueryComposer *ui;
    QNetworkAccessManager *manager;
    QString url;
    QString m_sSettingsFile;

    bool available;
    QList<VlkbTable *> table_list;
    QUrl urlRedirectedTo;
    LoadingWidget *loading;
    vtkwindow_new *vtkwin;
    void doQuery(QString band);
    bool isConnected;
    bool isBandmerged;
    bool isFilament;
    bool isBubble;
    bool is3dSelections;
};

#endif // VLKBSIMPLEQUERYCOMPOSER_H
