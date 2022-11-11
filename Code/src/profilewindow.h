#ifndef PROFILEWINDOW_H
#define PROFILEWINDOW_H

#include <QWidget>

class vtkwindow_new;

namespace Ui {
class ProfileWindow;
}

class ProfileWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileWindow(vtkwindow_new *v, QWidget *parent = nullptr);
    ~ProfileWindow();
     Ui::ProfileWindow *ui;

private:
    void closeEvent(QCloseEvent *event);
    vtkwindow_new *vtkwin;

};

#endif // PROFILEWINDOW_H
