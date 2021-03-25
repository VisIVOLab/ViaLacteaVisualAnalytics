#ifndef LOADINGWIDGET_H
#define LOADINGWIDGET_H

#include <QWidget>
#include <QNetworkReply>
#include <QPointer>

namespace Ui {
class LoadingWidget;
}

class LoadingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoadingWidget(QWidget *parent = 0);
    ~LoadingWidget();
    void loadingEnded();
    void init();
    void setFileName(QString name);
    void setLoadingProcess(QNetworkReply *);


private slots:
    void on_dismissPushButton_clicked();

private:
    Ui::LoadingWidget *ui;
    QPointer<QNetworkReply> reply;
};

#endif // LOADINGWIDGET_H
