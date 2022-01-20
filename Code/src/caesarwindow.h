#ifndef CAESARWINDOW_H
#define CAESARWINDOW_H

#include "authwrapper.h"

#include <QGroupBox>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QTableWidget>
#include <QTimer>

namespace Ui {
class CaesarWindow;
}

class CaesarWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CaesarWindow(QWidget *parent = nullptr, const QString &imagePath = QString());
    ~CaesarWindow();

    static const QString baseUrl;

private slots:
    void on_jobRefreshButton_clicked();
    void on_jobAutoRefreshCheckbox_clicked(bool checked);
    void on_dataRefreshButton_clicked();
    void on_dataDeleteButton_clicked();
    void on_dataDownloadButton_clicked();
    void on_dataUploadButton_clicked();
    void on_jobDownloadButton_clicked();
    void on_jobCancelButton_clicked();
    void on_dataTable_itemSelectionChanged();
    void on_appsComboBox_currentTextChanged(const QString &app);

    void on_jobSubmitButton_clicked();

private:
    Ui::CaesarWindow *ui;
    AuthWrapper *auth;
    QNetworkAccessManager *nam;
    QMap<QString, QPair<QVariant::Type, QWidget *>> inputs;
    QMap<QString, QGroupBox *> boxes;

    QTimer *refreshTimer;
    bool autoRefresh;
    int jobRefreshPeriod;

    QString formatDate(const QString &date);
    QString formatElapsedTime(const int &elapsedTime);
    bool selectedItemId(const QTableWidget *table, QString &id);
    bool selectedDataFilename(QString &filename);

    void getSupportedApps();
    void updateJobsTable(const QJsonArray &jobs);
    void updateDataTable(const QJsonArray &files);
    void uploadData(const QString &fn);
    void buildJobForm(const QJsonObject &app);
};

#endif // CAESARWINDOW_H
