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

    Qt::CheckState getInfoMopra(QString &tooltipText) const;
    Qt::CheckState getInfoChimps(QString &tooltipText) const;
    Qt::CheckState getInfoChamp(QString &tooltipText) const;
    Qt::CheckState getInfoHops(QString &tooltipText) const;
    Qt::CheckState getInfoGrs(QString &tooltipText) const;
    Qt::CheckState getInfoMalt(QString &tooltipText) const;
    Qt::CheckState getInfoThrumms(QString &tooltipText) const;
    Qt::CheckState getInfoNanten(QString &tooltipText) const;
    Qt::CheckState getInfoOgs(QString &tooltipText) const;
    Qt::CheckState getInfoCohrs(QString &tooltipText) const;
    Qt::CheckState getInfoVgps(QString &tooltipText) const;
    Qt::CheckState getInfoCgps(QString &tooltipText) const;
    Qt::CheckState getInfoSgps(QString &tooltipText) const;
    Qt::CheckState getInfoAro(QString &tooltipText) const;
    Qt::CheckState getInfoThor(QString &tooltipText) const;
    Qt::CheckState getInfoSedigism(QString &tooltipText) const;
    Qt::CheckState getInfoFugin(QString &tooltipText) const;

private:
    QString designation;
    double glon;
    double glat;
    QList<QMap<QString, QString>> searchResults;
    QList<QMap<QString, QString>> images;
    QList<QMap<QString, QString>> cubes;

    Qt::CheckState testSet(QString &tooltipText, const QSet<QString> &expected, const QSet<QString> &actual) const;
    Qt::CheckState testSingle(QString &tooltipText, const QString &expected, bool actual) const;

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

    QSet<QString> expectedMopra = {"12CO", "13CO", "C17O", "C18O"};
    QSet<QString> actualMopra;

    QSet<QString> expectedChimps = {"C18O", "13CO"};
    QSet<QString> actualChimps;

    QString expectedChamp = "HCO+";
    bool actualChamp;

    QSet<QString> expectedHops = {"H2O", "NH3"};
    QSet<QString> actualHops;

    QString expectedGrs = "13CO";
    bool actualGrs;

    QSet<QString> expectedMalt = {"N2H+", "13CS", "H", "CH3CN", "HC3N", "13C34S", "HNC", "HC13CCN", "HCO+", "HCN", "HNCO", "C2H", "HN13C", "SiO", "H13CO+"};
    QSet<QString> actualMalt;

    QSet<QString> expectedThrumms = {"12CO", "13CO", "C18O", "CN"};
    QSet<QString> actualThrumms;

    QString expectedNanten = "12CO";
    bool actualNanten;

    QSet<QString> expectedOgs = {"12CO", "13CO"};
    QSet<QString> actualOgs;

    QString expectedCohrs = "12CO";
    bool actualCohrs;

    QString expectedVgps = "HI";
    bool actualVgps;

    QString expectedCgps = "HI";
    bool actualCgps;

    QString expectedSgps = "HI";
    bool actualSgps;

    QSet<QString> expectedAro = {"12CO", "13CO"};
    QSet<QString> actualAro;

    QSet<QString> expectedThor = {"HI", "OH"};
    QSet<QString> actualThor;

    QSet<QString> expectedSedigism = {"13CO", "C18O"};
    QSet<QString> actualSedigism;

    QSet<QString> expectedFugin = {"12CO", "13CO", "C18O"};
    QSet<QString> actualFugin;
};

class UserTableWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit UserTableWindow(QString filepath, QWidget *parent = nullptr);
    ~UserTableWindow();

private slots:
    void on_queryButton_clicked();
    void on_selectionComboBox_activated(const QString &arg1);

private:
    Ui::UserTableWindow *ui;
    QString filepath;
    QList<QMap<QString, QString>> sources;
    QList<Source*> selectedSources;
    QSet<QString> filters;

    void readFile();
    void loadSourceTable(const QStringList &columns);
    void changeSelectionMode(const QString &selectionMode);
    void query(int index);
    void updateTables();
};

#endif // USERTABLEWINDOW_H
