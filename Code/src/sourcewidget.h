#ifndef SOURCEWIDGET_H
#define SOURCEWIDGET_H

#include <QPointer>
#include <QWidget>

class QListWidgetItem;
class Catalogue;
class Island;
class vtkwindow_new;

namespace Ui {
class SourceWidget;
}

class SourceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SourceWidget(QWidget *parent, Catalogue *catalogue, vtkwindow_new *win);
    ~SourceWidget();

public slots:
    void showIslandInfo(const Island *island);

private slots:
    void on_listExtracted_itemClicked(QListWidgetItem *item);
    void on_btnEdit_clicked();
    void on_btnSave_clicked();
    void on_btnDelete_clicked();
    void on_btnRename_clicked();
    void on_listFiltered_itemClicked(QListWidgetItem *item);

    void on_btnMorphLabel_clicked();

    void on_btnDeleteAll_clicked();

private:
    Ui::SourceWidget *ui;
    QPointer<Catalogue> catalogue;
    vtkwindow_new *win;

    QString iau_name;
    bool editing;
};

#endif // SOURCEWIDGET_H
