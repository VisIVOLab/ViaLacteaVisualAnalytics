#ifndef SELECTEDSOURCEFIELDSSELECT_H
#define SELECTEDSOURCEFIELDSSELECT_H

#include <QWidget>
#include "vstabledesktop.h"
#include <QCheckBox>
#include <QHash>
#include <QListWidgetItem>
#include "vtkwindow_new.h"

namespace Ui {
class selectedSourceFieldsSelect;
}

class selectedSourceFieldsSelect : public QWidget
{
    Q_OBJECT

public:
    explicit selectedSourceFieldsSelect(vtkwindow_new *v, QList<QListWidgetItem*> sel, QWidget *parent = 0);
    ~selectedSourceFieldsSelect();

private slots:
    void checkboxClicked(int cb);
    void on_selectAllButton_clicked();

    void on_deselectAllButton_clicked();

    void on_okButton_clicked();

private:
    Ui::selectedSourceFieldsSelect *ui;
    VSTableDesktop *table;
    QVector<QCheckBox *> checkboxList;
    QHash <int,QString> selectedFields;
     QList<QListWidgetItem*> selectedSources;
    vtkwindow_new *vtkwin;
};

#endif // SELECTEDSOURCEFIELDSSECECT_H
