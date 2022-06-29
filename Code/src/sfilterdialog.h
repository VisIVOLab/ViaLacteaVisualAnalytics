#ifndef SFILTERDIALOG_H
#define SFILTERDIALOG_H

#include <QDialog>

class Catalogue;
class QStringListModel;

namespace Ui {
class SFilterDialog;
}

class SFilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SFilterDialog(Catalogue *c, QWidget *parent = nullptr);
    ~SFilterDialog();

private slots:
    void on_pushButton_clicked();
    void on_comboBox_currentTextChanged(const QString &arg1);

private:
    Ui::SFilterDialog *ui;

    Catalogue *catalogue;

    QStringListModel *model;
    QStringList fields {
        "name",
        "class_label",
        "class_score",
        "sourceness_label",
        "sourceness_score",
        "tags",
        "ra",
        "border",
        "dec",
        "x",
        "y",
        "Smax",
        "Stot",
        "bkg",
        "npix",
        "min_bbox_par",
        "circ_ratio_par",
        "elongation_par",
        "beam_area_ratio_par",
        "max_size",
        "morph_label",
        "resolved",
        "fit_info",
        "ncomponents",
        "flux",
        "flux_err",
        "fit_quality",
        "chi2",
    };
};

#endif // SFILTERDIALOG_H
