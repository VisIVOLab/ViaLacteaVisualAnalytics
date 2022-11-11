#include "profilewindow.h"
#include "ui_profilewindow.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkRenderer.h"
#include <QCloseEvent>
#include <QDebug>


ProfileWindow::ProfileWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProfileWindow)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    this->setWindowTitle("1D Profile");

}

ProfileWindow::~ProfileWindow()
{
    delete ui;
}

void ProfileWindow::closeEvent (QCloseEvent *event)
{
    qDebug()<<"on close";
}
