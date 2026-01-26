#ifndef LUTCustomizerDialog_h
#define LUTCustomizerDialog_h

#include <vtkSmartPointer.h>

#include <QDialog>

class QAbstractButton;
class QCPAbstractItem;
class QCPItemLine;
class vtkImageData;
class vtkLookupTable;

QT_BEGIN_NAMESPACE
namespace Ui {
class LUTCustomizerDialog;
}
QT_END_NAMESPACE

class LUTCustomizerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LUTCustomizerDialog(QWidget *parent = nullptr);
    ~LUTCustomizerDialog();

    void init(vtkImageData *dataset, vtkLookupTable *lut);

signals:
    void lutUpdated();

private slots:
    void resetMin();
    void resetMax();

    void selectReferenceLine(QCPAbstractItem *item);
    void moveReferenceLine(QMouseEvent *event);
    void deselectReferenceLine();
    void updateReferenceLines();

    void updateLut();
    void buttonClicked(QAbstractButton *btn);

private:
    Ui::LUTCustomizerDialog *ui;
    QCPItemLine *refLineMin;
    QCPItemLine *refLineMax;
    QCPItemLine *selectedLine;

    vtkSmartPointer<vtkImageData> dataset;
    vtkSmartPointer<vtkLookupTable> lut;
    double datasetRange[2];

    void setupPlot();
    void plotHistogram();
};

#endif
