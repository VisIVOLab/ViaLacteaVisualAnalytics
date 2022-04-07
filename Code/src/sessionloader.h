#ifndef SESSIONLOADER_H
#define SESSIONLOADER_H

#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QMainWindow>

namespace Ui {
class SessionLoader;
}

class SessionLoader : public QMainWindow
{
    Q_OBJECT

public:
    explicit SessionLoader(QWidget *parent, const QString &sessionFilepath);
    ~SessionLoader();

private slots:
    void setOriginLayer();

    void on_btnLoad_clicked();

private:
    Ui::SessionLoader *ui;
    QString sessionFilepath;
    QDir sessionRootFolder;

    QJsonObject root;
    QJsonArray layers;
    int originLayerIdx;
    QJsonArray datacubes;

    QSet<int> overlapFailures;
    bool testOverlaps();

    void initTable();
    void setFitsRowBold(int row, bool bold);
    void setFitsRowColor(int row, const QBrush& b = QBrush());
};

#endif // SESSIONLOADER_H
