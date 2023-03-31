#ifndef SIMCOLLAPSEDIALOG_H
#define SIMCOLLAPSEDIALOG_H

#include <QDialog>

#include <vtkSmartPointer.h>

namespace Ui {
class SimCollapseDialog;
}

class vtkFitsReader;

class SimCollapseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SimCollapseDialog(vtkSmartPointer<vtkFitsReader> reader, double *angles,
                               QWidget *parent = nullptr);
    ~SimCollapseDialog();

public slots:
    void accept() override;

private:
    Ui::SimCollapseDialog *ui;
    vtkSmartPointer<vtkFitsReader> reader;
    double rotAngles[3];
    double minDistance;

    void showResultingImage(const QString &filepath);
};

#endif // SIMCOLLAPSEDIALOG_H
