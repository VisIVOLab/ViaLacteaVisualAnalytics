#ifndef VLKBSIMPLEQUERYCOMPOSER_H
#define VLKBSIMPLEQUERYCOMPOSER_H

#include "loadingwidget.h"
#include "vtkwindow_new.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QWidget>

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
    void on_queryPushButton_clicked();
    void on_tableComboBox_currentIndexChanged(int index);
    void on_savedatasetCheckBox_clicked(bool checked);
    void on_navigateFSButtono_clicked();
    void closeEvent(QCloseEvent *event);

private:
    Ui::VLKBSimpleQueryComposer *ui;
    QString url;
    QString m_sSettingsFile;
    QString vlkbtype;
    QString table_prefix;

    LoadingWidget *loading;
    vtkwindow_new *vtkwin;
    void doQuery(QString band);
    bool isConnected;
    bool isBandmerged;
    bool isFilament;
    bool isBubble;
    bool is3dSelections;

    void updateUI(bool enabled);
    void fetchBandTables();
    QString generateQuery();
};

#endif // VLKBSIMPLEQUERYCOMPOSER_H
