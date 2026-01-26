#ifndef vtkWindowImage_h
#define vtkWindowImage_h

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class vtkWindowImage;
}
QT_END_NAMESPACE

class vtkWindowImage : public QMainWindow
{
    Q_OBJECT

public:
    explicit vtkWindowImage(QWidget *parent = nullptr);
    ~vtkWindowImage() override;

private:
    Ui::vtkWindowImage *ui;

    void setupRenderer();
};

#endif
