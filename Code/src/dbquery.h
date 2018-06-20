#ifndef DBQUERY_H
#define DBQUERY_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QWidget>
#include "xmlparser.h"
#include <QComboBox>
#include "loadingwidget.h"
#include "dcvisualizer.h"
#include "vtkwindow_new.h"

namespace Ui {
class dbquery;
}

class dbquery : public QDialog
{
    Q_OBJECT

public:
    explicit dbquery(QWidget *parent = 0);
    QNetworkAccessManager* nam;
    xmlparser *parser;
    LoadingWidget *loading;
    dcvisualizer *visualizer;
    QString downloadedFile;
    ~dbquery();
    void setCoordinate(QString l, QString b);
    void setCoordinate(char* l, char* b);
    void on_mapInteraction(QString &l, QString &b);
    void addDatacubesToUI(QList< QMap<QString,QString> >& datacubes);
    void setVtkWindow(vtkwindow_new *v);


private slots:

    void finishedSlot(QNetworkReply* reply);
    void finishedSlot2(QNetworkReply* reply);

    void on_comboBox_surveys_activated(const QString &arg1);

    void enableAllItems(QComboBox *, int iItems);
    void disableItems(QComboBox *, int nItems, int* indexes, int size);
    void on_comboBox_species_activated(const QString &arg1);
    void on_comboBox_transitions_activated(const QString &arg1);
    void on_queryPushButton_clicked();
    void on_pushButton_map_clicked();
    void handleButton(int i);
    
    void on_horizontalSlider_sliderMoved(int position);
    void on_vtkwindow_button_clicked();

    void on_spinBox_valueChanged(int arg1);


public slots:
    void on_download_completed();
    

private:
    Ui::dbquery *ui;
    vtkwindow_new *vtkwin, *vtkcontourwindow;
    int n_surveys;
    int n_species;
    int n_transitions;
    QString species;
    QString survey;
    QString transition;
    QList< QMap<QString,QString> > datacubes_list;
    QString vlkbUrl;
    QString vlkbsearchUrl;
    QString vlkbcutoutUrl;
};

#endif // DBQUERY_H
