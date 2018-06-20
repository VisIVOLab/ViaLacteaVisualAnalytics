#ifndef VIALACTEA_FILELOAD_H
#define VIALACTEA_FILELOAD_H

#include <QWidget>
#include "vtkwindow_new.h"
namespace Ui {
class Vialactea_FileLoad;
}

class Vialactea_FileLoad : public QWidget
{
    Q_OBJECT

public:
    explicit Vialactea_FileLoad(QString f, bool silent=false, QWidget *parent = 0);
    explicit Vialactea_FileLoad(QString f, vtkwindow_new *v,QWidget *parent = 0);
    void init(QString f);
    void setVtkWin(vtkwindow_new *v){vtkwin=v;}
    void importBandMerged();
    void setCatalogueActive();
    void setWavelength(QString w);
    void importFilaments();
    void import3dSelection();
    void importBubbles();


    QString filename;
    ~Vialactea_FileLoad();

public slots:
    void on_okPushButton_clicked();

private slots:
    void on_groupBox_toggled(bool arg1);

private:
    Ui::Vialactea_FileLoad *ui;
    vtkwindow_new *vtkwin;
    bool silentLoad;
};

#endif // VIALACTEA_FILELOAD_H
