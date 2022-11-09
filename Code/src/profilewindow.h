#ifndef PROFILEWINDOW_H
#define PROFILEWINDOW_H

#include <QWidget>

namespace Ui {
class ProfileWindow;
}

class ProfileWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileWindow(QWidget *parent = nullptr);
    ~ProfileWindow();
     Ui::ProfileWindow *ui;

private:
    void closeEvent(QCloseEvent *bar);
};

#endif // PROFILEWINDOW_H
