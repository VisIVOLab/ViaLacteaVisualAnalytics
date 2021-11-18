#ifndef USERTABLEWINDOW_H
#define USERTABLEWINDOW_H

#include <QDir>
#include <QList>
#include <QMainWindow>
#include <QMap>
#include <QPair>
#include <QSet>
#include <QSettings>

namespace Ui {
class UserTableWindow;
}

class Survey : public QObject
{
    Q_OBJECT
public:
    explicit Survey(const QString &name,
                    const QString &species,
                    const QString &transition,
                    QObject *parent = nullptr);

    const QString &getName() const;
    const QStringList &getSpecies() const;
    const QStringList &getTransitions() const;

    void addSpecies(const QString &s);
    void addTransition(const QString &t);

private:
    QString name;
    QStringList species;
    QStringList transitions;
};

class Source : public QObject
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

    Qt::CheckState surveyInfo(QString &tooltipText, const Survey *survey, bool is3D = false) const;

private:
    QString designation;
    double glon;
    double glat;
    QList<QMap<QString, QString>> images;
    QList<QMap<QString, QString>> cubes;

    QMap<QString, QSet<QString> *> transitions;
    QMap<QString, QSet<QString> *> species;
};

class UserTableWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit UserTableWindow(const QString &filepath,
                             const QString &settingsFile,
                             QWidget *parent = nullptr);
    ~UserTableWindow();

private slots:
    void on_queryButton_clicked();
    void on_selectionComboBox_activated(const QString &arg1);
    void on_downloadImagesButton_clicked();
    void on_downloadCubesButton_clicked();

private:
    Ui::UserTableWindow *ui;
    QString filepath;
    QSettings settings;

    QMap<QString, Survey *> surveys2d;
    QMap<QString, Survey *> surveys3d;

    QList<QMap<QString, QString>> sources;
    QList<Source *> selectedSources;

    QMultiMap<QString, QPair<QString, QString>> filters;

    void getSurveysData();
    void buildUI(const QStringList &surveysData);
    void readFile();
    void loadSourceTable(const QStringList &columns);
    void changeSelectionMode(const QString &selectionMode);
    void query(int index);
    void updateTables();
    void setFilter(const QString &survey, const QString &transition, bool on);
    void downloadImages(const QMap<QString, QStringList> &cutouts, const QDir &dir);
};

#endif // USERTABLEWINDOW_H
