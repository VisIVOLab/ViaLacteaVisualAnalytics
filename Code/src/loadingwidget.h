#ifndef LOADINGWIDGET_H
#define LOADINGWIDGET_H

#include <QNetworkReply>
#include <QPointer>
#include <QProcess>
#include <QWidget>

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
    void setText(QString name);
    void setLoadingProcess(QNetworkReply *);
    void setLoadingProcess(QProcess *);
    void setButtonStatus(bool enabled);

public slots:
    void updateProgressBar(qint64 current, qint64 max);

private slots:
    void on_dismissPushButton_clicked();

private:
    Ui::LoadingWidget *ui;
    QPointer<QNetworkReply> reply;
    QPointer<QProcess> process;
};

#endif // LOADINGWIDGET_H
