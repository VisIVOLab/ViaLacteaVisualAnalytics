#ifndef USERTABLEWINDOW_H
#define USERTABLEWINDOW_H

#include "VLKBInventoryTree.h"

#include <QDir>
#include <QList>
#include <QMainWindow>
#include <QMap>
#include <QPair>
#include <QSet>
#include <QSettings>
#include <QTableWidget>

namespace Ui {
class UserTableWindow;
}

class Survey : public QObject
{
    Q_OBJECT
public:
    explicit Survey(const QString &name, const QString &desc, const QString &species,
                    const QString &transition, QObject *parent = nullptr);

    const QString &getName() const;
    const QString &getDescription() const;
    const QStringList &getSpecies() const;
    const QStringList &getTransitions() const;

    void addSpecies(const QString &s);
    void addTransition(const QString &t);

    int count() const;

private:
    QString name;
    QString description;
    QStringList species;
    QStringList transitions;
};

class SourceCutouts : public QObject
{
    Q_OBJECT

public:
    explicit SourceCutouts(const QString &designation, double glon, double glat,
                           QObject *parent = nullptr);
    void parseSearchResults(const QList<Cutout> &results);

    const QString &getDesignation() const;
    double getGlon() const;
    double getGlat() const;

    bool getBestCutout(const QString &survey, const QString &species, const QString &transition,
                       Cutout &cutout) const;

    const QList<Cutout> &getImages() const;
    int getImagesCount() const;

    const QList<Cutout> &getCubes() const;
    int getCubesCount() const;

    Qt::CheckState surveyInfo(QString &tooltipText, const Survey *survey, bool is3D = false) const;

private:
    QString designation;
    double glon;
    double glat;
    QList<Cutout> images;
    QList<Cutout> cubes;

    QMap<QString, QSet<QString> *> transitions;
    QMap<QString, QSet<QString> *> species;
};

class UserTableWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit UserTableWindow(const QString &filepath, const QString &settingsFile,
                             QWidget *parent = nullptr);
    ~UserTableWindow();

private slots:
    void on_queryButton_clicked();
    void on_selectionComboBox_activated(const QString &arg1);
    void on_tabWidget_currentChanged(int index);
    void on_btnSendRequest_clicked();

private:
    Ui::UserTableWindow *ui;
    QString filepath;
    QSettings settings;

    QMap<QString, Survey *> surveys2d;
    QMultiMap<QString, QPair<QString, QString>> selectedSurveys2d;

    QMap<QString, Survey *> surveys3d;
    QMultiMap<QString, QPair<QString, QString>> selectedSurveys3d;

    QList<QMap<QString, QString>> sources;
    QList<SourceCutouts *> selectedSources;

    void getSurveysData();
    void buildUI(const QStringList &surveysData);
    void buildUI(const QMap<QString, Survey *> &surveys,
                 QMultiMap<QString, QPair<QString, QString>> &selectedSurveys, QTableWidget *table,
                 QWidget *scrollArea);
    void readFile();
    void loadSourceTable(const QStringList &columns);
    void changeSelectionMode(const QString &selectionMode);
    void query(int index = 0);
    void updateTables();
    QList<Cutout> getCutoutsList(int t);
    void initMCutoutRequest();
};

#endif // USERTABLEWINDOW_H
