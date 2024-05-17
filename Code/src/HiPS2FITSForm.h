#ifndef HIPS2FITSFORM_H
#define HIPS2FITSFORM_H

#include <QWidget>

namespace Ui {
class HiPS2FITSForm;
}

class QNetworkAccessManager;

class HiPS2FITSForm : public QWidget
{
    Q_OBJECT

public:
    explicit HiPS2FITSForm(QWidget *parent = nullptr);
    ~HiPS2FITSForm();

private slots:
    void sendQuery();
    void openFile(const QString &filepath);
    void on_comboFormat_currentIndexChanged(int index);

private:
    Ui::HiPS2FITSForm *ui;
    QNetworkAccessManager *nam;
    QString endpoint;
};

#endif // HIPS2FITSFORM_H
