#ifndef STARTUPWINDOW_H
#define STARTUPWINDOW_H

#include <QWidget>

namespace Ui {
class StartupWindow;
}

class StartupWindow : public QWidget
{
    Q_OBJECT

public:
    explicit StartupWindow(QWidget *parent = nullptr);
    ~StartupWindow();

private slots:
    void on_localOpenPushButton_clicked();
    void openLocalDC(const QString &fn);


private:
    Ui::StartupWindow *ui;
    void PopulateHistory();

};



#endif // STARTUPWINDOW_H
