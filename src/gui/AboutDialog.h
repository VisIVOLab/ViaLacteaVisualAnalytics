#ifndef AboutDialog_h
#define AboutDialog_h

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class AboutDialog;
}
QT_END_NAMESPACE

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private:
    Ui::AboutDialog *ui;
};

#endif
