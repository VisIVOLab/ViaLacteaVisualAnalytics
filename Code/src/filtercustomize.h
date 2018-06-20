#ifndef FILTERCUSTOMIZE_H
#define FILTERCUSTOMIZE_H
#include <QWidget>
#include "vtkwindow_new.h"

namespace Ui {
class FilterCustomize;
}

class FilterCustomize : public QWidget
{
    Q_OBJECT

public:
    explicit FilterCustomize(vtkwindow_new *v, QWidget *parent = 0);

    ~FilterCustomize();

private slots:
    void on_fieldComboBox_activated(const QString &arg1);
    void setRangeId(int id);

private:
    Ui::FilterCustomize *ui;
    vtkwindow_new *vtkwin;

};

#endif // FILTERCUSTOMIZE_H
