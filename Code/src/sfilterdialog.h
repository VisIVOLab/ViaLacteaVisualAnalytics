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
    void on_btnFilter_clicked();
    void append(const QString &arg);
    void on_comboFields_currentTextChanged(const QString &arg1);
    void on_comboMorphLabel_currentTextChanged(const QString &arg1);
    void on_comboSourcenessLabel_currentTextChanged(const QString &arg1);
    void on_comboClassLabel_currentTextChanged(const QString &arg1);
    void on_btnConfirm_clicked();

private:
    Ui::SFilterDialog *ui;

    Catalogue *catalogue;

    QStringListModel *model;
    QStringList fields { "name",
                         "class_label",
                         "class_score",
                         "sourceness_label",
                         "sourceness_score",
                         "tags",
                         "ra",
                         "dec",
                         "x",
                         "y",
                         "border",
                         "npix",
                         "min_bbox_par",
                         "circ_ratio_par",
                         "elongation_par",
                         "beam_area_ratio_par",
                         "max_size",
                         "morph_label",
                         "resolved",
                         "Stot",
                         "Smax",
                         "bkg",
                         "rms",
                         "spectral_info",
                         "alpha",
                         "alpha_err",
                         "fit",
                         "chi2",
                         "ndf",
                         "fit_info",
                         "ncomponents",
                         "flux",
                         "flux_err",
                         "fit_quality",
                         "crossmatch_info" };

    QStringList morph_label_val { "UNKNOWN",  "COMPACT", "POINT-LIKE",
                              "EXTENDED", "DIFFUSE", "COMPACT-EXTENDED" };
};

#endif // SFILTERDIALOG_H
