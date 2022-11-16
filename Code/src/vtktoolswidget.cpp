#include "vtktoolswidget.h"
#include "ui_vtktoolswidget.h"

#include "qdebug.h"
#include "vtkDataSet.h"
#include "vtkProperty2D.h"
#include "vtkSmartPointer.h"
#include "vtkXYPlotActor.h"

vtktoolswidget::vtktoolswidget(vtkwindow_new *v, QWidget *parent)
    : QWidget(parent), ui(new Ui::vtktoolswidget)
{
    ui->setupUi(this);
    vtkwin = v;
    for (unsigned int i = 0; i < vtkwin->table->getNumberOfColumns(); i++) {
        QString field = QString(vtkwin->table->getColName(i).c_str());
        ui->scalarComboBox->addItem(field);
    }
}

vtktoolswidget::~vtktoolswidget()
{
    delete ui;
}

void vtktoolswidget::plotHistogram(int i)
{

    QVector<double> y(vtkwin->table->getbinNumber());
    y = vtkwin->table->getHistogram(i);
    QVector<double> x(vtkwin->table->getbinNumber());
    x = vtkwin->table->getHistogramValue(i);
    range = new float[3];
    vtkwin->table->getRange(i, range);
}

void vtktoolswidget::drawLine(double from, double to)
{
    if (fromLine != 0 && toLine != 0) {

        ui->histogramWidget->removeItem(fromLine);
        ui->histogramWidget->removeItem(toLine);
    }

    fromLine = new QCPItemLine(ui->histogramWidget);
    toLine = new QCPItemLine(ui->histogramWidget);

    QPen pen;
    pen.setColor(Qt::red);

    fromLine->setPen(pen);
    pen.setColor(Qt::green);
    toLine->setPen(pen);

    fromLine->start->setCoords(from, 0);
    fromLine->end->setCoords(from, range[2]);

    toLine->start->setCoords(to, 0);
    toLine->end->setCoords(to, range[2]);
    ui->histogramWidget->replot();
    vtkwin->pp->setLookupTable(from, to);
}

void vtktoolswidget::on_ShowBoxCheckBox_clicked(bool checked)
{
    vtkwin->showBox(checked);
}

void vtktoolswidget::on_ShowAxesCheckBox_clicked(bool checked)
{
    vtkwin->showAxes(checked);
}

void vtktoolswidget::on_activateLutCheckBox_clicked(bool checked)
{
    ui->lutLabel->setEnabled(checked);
    ui->lutComboBox->setEnabled(checked);
    ui->scalarLabel->setEnabled(checked);
    ui->scalarComboBox->setEnabled(checked);

    std::string palette = "AllWhite";
    if (checked) {
        palette = ui->lutComboBox->currentText().toStdString().c_str();
    }
    changeLut(palette);
}

void vtktoolswidget::changeLut(std::string palette)
{
    vtkwin->changePalette(palette);
}

void vtktoolswidget::changeScalar(std::string scalar)
{
    vtkwin->changeScalar(scalar);
}

void vtktoolswidget::on_lutComboBox_currentIndexChanged(const QString &arg1)
{
    changeLut(ui->lutComboBox->currentText().toStdString().c_str());
}

void vtktoolswidget::on_ShowColorbarCheckBox_clicked(bool checked)
{
    vtkwin->showColorbar(checked);
}

void vtktoolswidget::on_scalarComboBox_currentIndexChanged(const QString &arg1)
{
}

void vtktoolswidget::on_scalarComboBox_currentIndexChanged(int index)
{
    plotHistogram(index);
}

void vtktoolswidget::on_fromSlider_sliderMoved(int position)
{
    ui->fromValue->setText(QString::number(position));
    drawLine(position, ui->toValue->text().toDouble());
}

void vtktoolswidget::on_toSlider_sliderMoved(int position)
{
    ui->toValue->setText(QString::number(position));
    drawLine(ui->fromValue->text().toDouble(), position);
}

void vtktoolswidget::on_fromValue_textChanged(const QString &arg1)
{
    ui->fromSlider->setValue(arg1.toDouble());
    drawLine(ui->fromValue->text().toDouble(), ui->toValue->text().toDouble());
}

void vtktoolswidget::on_toValue_textChanged(const QString &arg1)
{
    ui->toSlider->setValue(arg1.toDouble());
    drawLine(ui->fromValue->text().toDouble(), ui->toValue->text().toDouble());
}

void vtktoolswidget::on_scaleCheckBox_clicked(bool checked)
{
    vtkwin->scale(checked);
}

void vtktoolswidget::on_cameraLeft_clicked()
{
    vtkwin->setCameraAzimuth(-90);
}

void vtktoolswidget::on_cameraBack_clicked()
{
    vtkwin->setCameraAzimuth(-180);
}

void vtktoolswidget::on_cameraRight_clicked()
{
    vtkwin->setCameraAzimuth(90);
}

void vtktoolswidget::on_frontCamera_clicked()
{
    vtkwin->setCameraAzimuth(0);
}

void vtktoolswidget::on_topCamera_clicked()
{
    vtkwin->setCameraElevation(90);
}

void vtktoolswidget::on_bottomCamera_clicked()
{
    vtkwin->setCameraElevation(-90);
}

void vtktoolswidget::on_gridCheckBox_clicked(bool checked)
{
    vtkwin->showGrid(checked);
}

void vtktoolswidget::on_scalarComboBox_activated(const QString &arg1) { }

void vtktoolswidget::on_scaleCheckBox_clicked() { }

void vtktoolswidget::on_ShowColorbarCheckBox_clicked() { }
