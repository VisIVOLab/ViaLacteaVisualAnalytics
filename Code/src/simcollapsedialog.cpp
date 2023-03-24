#include "simcollapsedialog.h"
#include "ui_simcollapsedialog.h"

#include "simcube/simcube_projection.hpp"

#include "vtkfitsreader.h"
#include "vtkwindow_new.h"

#include <QDebug>
#include <QDir>
#include <QDoubleValidator>
#include <QFileInfo>
#include <QMessageBox>

SimCollapseDialog::SimCollapseDialog(vtkSmartPointer<vtkFitsReader> reader, double *angles,
                                     QWidget *parent)
    : QDialog(parent), ui(new Ui::SimCollapseDialog), reader(reader)
{
    ui->setupUi(this);

    for (int i = 0; i < 3; ++i) {
        rotAngles[i] = angles[i];
    }

    // Collapse section
    ui->textX->setText(QString::number(rotAngles[0], 'g', 4));
    ui->textY->setText(QString::number(rotAngles[1], 'g', 4));
    ui->textZ->setText(QString::number(rotAngles[2], 'g', 4));
    ui->labelPixelSize->setText(QString("Pixel size (%1):").arg(reader->getCUnit3().simplified()));
    ui->spinPixelSize->setValue(reader->getMaxCDelt());

    // Validators
    auto v = new QDoubleValidator(this);
    // ui->textPixelSize->setValidator(v);
    //  ui->textLon->setValidator(v);
    //  ui->textLat->setValidator(v);
    //  ui->textDistance->setValidator(v);
    //  ui->textSigma->setValidator(v);
}

SimCollapseDialog::~SimCollapseDialog()
{
    qDebug() << Q_FUNC_INFO;
    delete ui;
}

void SimCollapseDialog::accept()
{
    if (ui->spinPixelSize->value() < reader->getMaxCDelt()) {
        QMessageBox::warning(
                this, QString(),
                "Pixel size must be greater than or equal to the size of the input fits file.");
        return;
    }

    double scale = ui->spinPixelSize->value() / reader->getMaxCDelt();
    qDebug() << scale;

    // Save output image in the temp dir
    QString outDir = QDir::home().absoluteFilePath("VisIVODesktopTemp/tmp_download");
    QString outFile =
            QFileInfo(QString::fromStdString(reader->GetFileName())).baseName() + "_collapsed.fits";
    QString outputFilePath = QDir(outDir).absoluteFilePath(outFile);

    try {
        simcube::rotate_and_collapse(outputFilePath.toStdString(), reader->GetFileName(), rotAngles,
                                     scale);

        showResultingImage(outputFilePath);
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error", e.what());
        return;
    }

    QDialog::accept();
}

void SimCollapseDialog::showResultingImage(const QString &filepath)
{
    auto fits = vtkSmartPointer<vtkFitsReader>::New();
    fits->SetFileName(filepath.toStdString());
    auto win = new vtkwindow_new(nullptr, fits);
    win->show();
    win->activateWindow();
    win->raise();
}
