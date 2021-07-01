#ifndef CAESARWIDGET_H
#define CAESARWIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>

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
    void on_refreshButton_clicked();
    void on_uploadButton_clicked();
    void on_downloadButton_clicked();
    void on_deleteButton_clicked();

private:
    Ui::CaesarWidget *ui;
    AuthWrapper *auth;
    QNetworkAccessManager *nam;

    void updateDataTable(const QJsonArray &files);
    bool selectedItemId(QString &id);
    bool selectedItemName(QString &name);
};

#endif // CAESARWIDGET_H
