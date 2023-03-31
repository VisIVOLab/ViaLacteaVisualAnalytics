#include "simcollapsedialog.h"
#include "ui_simcollapsedialog.h"

#include "simcube/imresize.h"
#include "simcube/simcube_projection.hpp"

#include "vtkfitsreader.h"
#include "vtkwindow_new.h"

#include <QDebug>
#include <QDir>
#include <QDoubleValidator>
#include <QFileInfo>
#include <QMessageBox>

#include <cmath>

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

    // Projection section
    minDistance = 5.0 * reader->GetCdelt(0) * reader->GetNaxes(0);
    ui->spinDistance->setMinimum(0);
    ui->spinDistance->setMaximum(std::numeric_limits<double>::max());
    ui->spinDistance->setValue(minDistance);

    // Validators
    // auto v = new QDoubleValidator(this);
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

    double distance = ui->spinDistance->value();
    if (ui->checkPlace->isChecked() && distance < minDistance) {
        auto res =
                QMessageBox::warning(this, QString(),
                                     "The distance should be much larger than the total dimension "
                                     "of the simulated cube. Do you want to continue anyway?",
                                     QMessageBox::Yes | QMessageBox::No);
        if (res == QMessageBox::No) {
            return;
        }
    }

    double fwhm = ui->textFWHM->text().toDouble();
    double r2d = 180.0 / M_PI;
    double minFwhm = r2d * 3.0 * reader->getMaxCDelt() / distance;
    if (!ui->textFWHM->text().isEmpty() && fwhm < minFwhm) {
        QMessageBox::warning(this, QString(),
                             "Invalid FWHM value. The FWHM must be at least three times the Pixel "
                             "Size (FWHM > 3xCDELT).");
        ui->textFWHM->setText(QString::number(minFwhm + 1.0e-9, 'g', 9));
        return;
    }

    QString outDir = QDir::home().absoluteFilePath("VisIVODesktopTemp/tmp_download");
    QString outFile =
            QFileInfo(QString::fromStdString(reader->GetFileName())).baseName() + "_collapsed.fits";
    QString outputFilePath = QDir(outDir).absoluteFilePath(outFile);
    double scale = ui->spinPixelSize->value() / reader->getMaxCDelt();

    try {
        simcube::rotate_and_collapse(outputFilePath.toStdString(), reader->GetFileName(), rotAngles,
                                     scale);
        if (!ui->checkPlace->isChecked()) {
            // We are done
            showResultingImage(outputFilePath);
            QDialog::accept();
            return;
        }

        // Projection
        double center[2] { ui->textLon->text().toDouble(), ui->textLat->text().toDouble() };
        simcube::collapsed_to_galactic(outputFilePath.toStdString(), distance, center);
        if (ui->textFWHM->text().isEmpty()) {
            // We are done
            showResultingImage(outputFilePath);
            QDialog::accept();
            return;
        }

        // Filter and Resize
        double newCDelt = fwhm / 3.0;
        int resizeFactor = newCDelt / (r2d * reader->getMaxCDelt() / distance);
        double sigma = fwhm / 2.35;
        imresize(outputFilePath.toUtf8().data(), resizeFactor, sigma);
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
