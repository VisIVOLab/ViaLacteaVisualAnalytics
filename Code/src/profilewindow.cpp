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

    auto renWin_x = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->xPlot->setRenderWindow(renWin_x);
    auto interactor_x = renWin_x->GetInteractor();
    auto m_Ren0 = vtkSmartPointer<vtkRenderer>::New();
    m_Ren0->SetBackground(0.21, 0.23, 0.25);
    renWin_x->AddRenderer(m_Ren0);

    auto renWin_y = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->yPlot->setRenderWindow(renWin_y);
    auto interactor_y = renWin_y->GetInteractor();
    auto m_Ren1 = vtkSmartPointer<vtkRenderer>::New();
    m_Ren1->SetBackground(0.21, 0.23, 0.25);
    renWin_y->AddRenderer(m_Ren1);
}

ProfileWindow::~ProfileWindow()
{
    delete ui;
}

void ProfileWindow::closeEvent (QCloseEvent *event)
{
    qDebug()<<"on close";
}
