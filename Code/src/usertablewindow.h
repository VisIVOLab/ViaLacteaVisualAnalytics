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

    void on_glimpse_24_checkBox_clicked();
    void on_glimpse_8_checkBox_clicked();
    void on_glimpse_58_checkBox_clicked();
    void on_glimpse_45_checkBox_clicked();
    void on_glimpse_36_checkBox_clicked();

    void on_wise_22_checkBox_clicked();
    void on_wise_12_checkBox_clicked();
    void on_wise_46_checkBox_clicked();
    void on_wise_34_checkBox_clicked();

    void on_atlasgal_870_checkBox_clicked();

    void on_bolocam_11_checkBox_clicked();

    void on_cornish_5_checkBox_clicked();

    void on_selectionComboBox_activated(const QString &arg1);

private:
    Ui::UserTableWindow *ui;
    QString filepath;
    QList<QMap<QString, QString>> sources;
    QList<QMap<QString, QString>> images;
    QList<QMap<QString, QString>> cubes;
    // To avoid duplicate search results
    QSet<QString> set;
    QSet<QString> filters;


    void readFile();
    void changeSelectionMode(const QString &selectionMode);
    void query(QList<QPair<double, double>> *coords);
    void parseResults(const QList<QMap<QString, QString>> &results);
    void toggleFilter(QString transition);
    void updateTables();
};

#endif // USERTABLEWINDOW_H
