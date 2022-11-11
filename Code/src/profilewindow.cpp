#include "profilewindow.h"
#include "ui_profilewindow.h"

#include "vtkwindow_new.h"
#include "ui_vtkwindow_new.h"

#include <vtkRendererCollection.h>
#include <vtkRenderer.h>

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

void ProfileWindow::closeEvent(QCloseEvent *event)
{
    vtkwin->ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(vtkwin->actor_x);
    vtkwin->ui->qVTK1->renderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(vtkwin->actor_y);
    vtkwin->ui->qVTK1->renderWindow()->GetInteractor()->Render();
    QWidget::closeEvent(event);
}
