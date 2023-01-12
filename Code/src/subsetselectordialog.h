#ifndef SUBSETSELECTORDIALOG_H
#define SUBSETSELECTORDIALOG_H

#include <QDialog>

namespace Ui {
class SubsetSelectorDialog;
}

struct CubeSubset
{
    bool ReadSubExtent { false };
    int SubExtent[6] { -1, -1, -1, -1, -1, -1 };
    bool AutoScale { false };
    int CubeMaxSize { 1024 };
    int ScaleFactor { 1 };
};

class SubsetSelectorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SubsetSelectorDialog(QWidget *parent = nullptr,
                                  const CubeSubset &subset = CubeSubset());
    ~SubsetSelectorDialog();

signals:
    void subsetSelected(const CubeSubset &subset);

private slots:
    void btnOkClicked();

private:
    Ui::SubsetSelectorDialog *ui;
    CubeSubset subset;
};

#endif // SUBSETSELECTORDIALOG_H
