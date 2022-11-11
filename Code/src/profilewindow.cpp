#include "profilewindow.h"
#include "ui_profilewindow.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkRenderer.h"
#include <QCloseEvent>
#include <QDebug>
#include "ui_vtkwindow_new.h"
#include <vtkRendererCollection.h>


ProfileWindow::ProfileWindow(vtkwindow_new *v, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProfileWindow)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    vtkwin=v;
    this->setWindowTitle("1D Profile");

}

ProfileWindow::~ProfileWindow()
{
    delete ui;
}

void ProfileWindow::closeEvent (QCloseEvent *event)
{
    vtkwin->profileMode=false;
    vtkwin->ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(vtkwin->actor_x);
    vtkwin->ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(vtkwin->actor_y);
    vtkwin->actor_y->Delete();
    vtkwin->actor_x->Delete();
    vtkwin->ui->qVTK1->update();
    vtkwin->ui->qVTK1->renderWindow()->GetInteractor()->Render();

}
