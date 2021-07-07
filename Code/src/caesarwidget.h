#ifndef CAESARWIDGET_H
#define CAESARWIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QTableWidget>
#include <QGroupBox>

#include <vtkSmartPointer.h>
#include "vtkfitsreader.h"

#include "authwrapper.h"

namespace Ui {
class CaesarWidget;
}

class CaesarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CaesarWidget(QWidget *parent = nullptr, vtkSmartPointer<vtkFitsReader> parentImage = nullptr);
    ~CaesarWidget();

    static QString baseUrl();

private slots:
    void on_dataRefreshButton_clicked();
    void on_dataUploadButton_clicked();
    void on_dataDownloadButton_clicked();
    void on_dataDeleteButton_clicked();
    void on_dataTable_itemSelectionChanged();

    void on_jobRefreshButton_clicked();
    void on_jobDownloadButton_clicked();
    void on_jobCancelButton_clicked();
    void on_jobSubmitButton_clicked();

    void on_appsComboBox_currentTextChanged(const QString &app);

private:
    Ui::CaesarWidget *ui;
    vtkSmartPointer<vtkFitsReader> parentImage;
    AuthWrapper *auth;
    QNetworkAccessManager *nam;
    QMap<QString, QGroupBox*> boxes;
    QMap<QString, QPair<QVariant::Type, QWidget*>> inputs;
    int jobRefreshPeriod;

    void uploadData(const QString &fn);
    void updateDataTable(const QJsonArray &files);
    void updateJobsTable(const QJsonArray &jobs);
    void getSupportedApps();
    void buildJobForm(const QJsonObject &app);
    bool selectedItemId(const QTableWidget *table, QString &id);
    bool selectedDataFilename(QString &filename);
};

#endif // CAESARWIDGET_H
