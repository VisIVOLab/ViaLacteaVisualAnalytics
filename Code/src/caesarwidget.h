#ifndef CAESARWIDGET_H
#define CAESARWIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QTableWidget>

#include "authwrapper.h"

namespace Ui {
class CaesarWidget;
}

class CaesarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CaesarWidget(QWidget *parent = nullptr);
    ~CaesarWidget();

    static QString baseUrl();

private slots:
    void on_dataRefreshButton_clicked();
    void on_dataUploadButton_clicked();
    void on_dataDownloadButton_clicked();
    void on_dataDeleteButton_clicked();

    void on_jobRefreshButton_clicked();
    void on_jobDownloadButton_clicked();
    void on_jobCancelButton_clicked();

private:
    Ui::CaesarWidget *ui;
    AuthWrapper *auth;
    QNetworkAccessManager *nam;

    void updateDataTable(const QJsonArray &files);
    void updateJobsTable(const QJsonArray &jobs);
    bool selectedItemId(const QTableWidget *table, QString &id);
    bool selectedItemName(QString &name);
};

#endif // CAESARWIDGET_H
