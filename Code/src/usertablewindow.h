#ifndef USERTABLEWINDOW_H
#define USERTABLEWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QList>
#include <QPair>
#include <QSet>

namespace Ui {
class UserTableWindow;
}

class Source: public QObject
{
    Q_OBJECT

public:
    explicit Source(const QString &designation, double glon, double glat, QObject *parent = nullptr);
    void parseSearchResults(const QList<QMap<QString, QString>> &results);

    const QString &getDesignation() const;
    double getGlon() const;
    double getGlat() const;

    const QList<QMap<QString, QString>> &getImages() const;
    const QList<QMap<QString, QString>> &getCubes() const;

    Qt::CheckState getInfoHiGal(QString &tooltipText) const;
    Qt::CheckState getInfoGlimpse(QString &tooltipText) const;
    Qt::CheckState getInfoWise(QString &tooltipText) const;
    Qt::CheckState getInfoMipsgal(QString &tooltipText) const;
    Qt::CheckState getInfoAtlasgal(QString &tooltipText) const;
    Qt::CheckState getInfoBgps(QString &tooltipText) const;
    Qt::CheckState getInfoCornish(QString &tooltipText) const;

private:
    QString designation;
    double glon;
    double glat;
    QList<QMap<QString, QString>> searchResults;
    QList<QMap<QString, QString>> images;
    QList<QMap<QString, QString>> cubes;

    QSet<QString> expectedHiGal = {"70 um", "160 um", "250 um", "350 um", "500 um"};
    QSet<QString> actualHiGal;

    QSet<QString> expectedGlimpse = {"3.6 um", "4.5 um", "5.8 um", "8.0 um"};
    QSet<QString> actualGlimpse;

    QSet<QString> expectedWise = {"3.4 um", "4.6 um", "12 um", "22 um"};
    QSet<QString> actualWise;

    QString expectedMipsgal = "24 um";
    bool actualMipsgal;

    QString expectedAtlasgal = "870 um";
    bool actualAtlasgal;

    QString expectedBgps = "1.1 mm";
    bool actualBgps;

    QString expectedCornish = "5 GHz";
    bool actualCornish;
};

class UserTableWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit UserTableWindow(QString filepath, QWidget *parent = nullptr);
    ~UserTableWindow();

private slots:
    void on_loadButton_clicked();
    void on_queryButton_clicked();

    void on_higal_70_checkBox_clicked();
    void on_higal_160_checkBox_clicked();
    void on_higal_250_checkBox_clicked();
    void on_higal_350_checkBox_clicked();
    void on_higal_500_checkBox_clicked();

    void on_glimpse_8_checkBox_clicked();
    void on_glimpse_58_checkBox_clicked();
    void on_glimpse_45_checkBox_clicked();
    void on_glimpse_36_checkBox_clicked();

    void on_wise_22_checkBox_clicked();
    void on_wise_12_checkBox_clicked();
    void on_wise_46_checkBox_clicked();
    void on_wise_34_checkBox_clicked();

    void on_mipsgal_24_checkBox_clicked();

    void on_atlasgal_870_checkBox_clicked();

    void on_bolocam_11_checkBox_clicked();

    void on_cornish_5_checkBox_clicked();

    void on_selectionComboBox_activated(const QString &arg1);

private:
    Ui::UserTableWindow *ui;
    QString filepath;
    QList<QMap<QString, QString>> sources;
    QList<Source*> selectedSources;
    QSet<QString> filters;

    void readFile();
    void changeSelectionMode(const QString &selectionMode);
    void query(int index);
    //
    void parseResults(const QList<QMap<QString, QString>> &results);
    void toggleFilter(QString transition);
    void updateTables();
    //
};

#endif // USERTABLEWINDOW_H
