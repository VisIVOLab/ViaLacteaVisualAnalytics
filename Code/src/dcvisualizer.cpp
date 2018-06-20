#include "dcvisualizer.h"
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <qdebug.h>
#include "vtkwindow.h"
#include <vtkSmartPointer.h>

dcvisualizer::dcvisualizer()
{

}

dcvisualizer::~dcvisualizer()
{

}

void dcvisualizer::visualize(const QString &filename){


    // create a window to render into
    vtkRenderWindow *renWin = vtkRenderWindow::New();
    //renWin = vtkRenderWindow::New();
    vtkRenderer *ren1 = vtkRenderer::New();
    //ren1->SetBackground(1.0, 1.0, 1.0);
    renWin->AddRenderer(ren1);
    vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
    iren->SetRenderWindow(renWin);

    // vtk pipeline
    vtkFitsReader *fitsReader = vtkFitsReader::New();
    fitsReader->is3D=true;
    qDebug()<<"******************   is3D: "<<fitsReader->is3D;
    qDebug()<<"******************   reading file: "<<filename;

    //sistemare qui
    //fitsReader->SetFileName("/Users/mariba/Dropbox/University/ViaLactea/Code/VisIVODesktop/trunk/VisIVODesktop.app/Contents/MacOS/vlkb-cutout_2015-04-17_10-19-36_478143_HOPS_G310.3-320.0-H2O-cube.fits");
    //fitsReader->SetFileName("/Users/mariba/Dropbox/University/ViaLactea/Code/VisIVODesktop/trunk/VisIVODesktop.app/Contents/MacOS/"+filename.toLocal8Bit().data());
    //QString cazzucazzu="/Users/mariba/Dropbox/University/ViaLactea/Code/VisIVODesktop/trunk/VisIVODesktop.app/Contents/MacOS/"+QString::fromAscii(filename);
    fitsReader->SetFileName(filename.toLocal8Bit().data());


    qDebug()<<"******************   1 ";
    fitsReader->Update();
    qDebug()<<"******************   2 ";


    // outline
    vtkOutlineFilter *outlineF = vtkOutlineFilter::New();
    outlineF->SetInputConnection(fitsReader->GetOutputPort());

    // outlineF->SetInput(fitsReader->GetOutput());
    vtkPolyDataMapper *outlineM = vtkPolyDataMapper::New();
    outlineM->SetInput(outlineF->GetOutput());
    outlineM->ScalarVisibilityOff();

    vtkActor *outlineA = vtkActor::New();
    outlineA->SetMapper(outlineM);
    //outlineA->GetProperty()->SetColor(0.0, 0.0, 0.0);

    // isosurface
    vtkMarchingCubes *shellE = vtkMarchingCubes::New();
    shellE->SetInput(fitsReader->GetOutput());
    shellE->ComputeNormalsOn();
    shellE->SetValue(0, 2.0f);

    // shellE->SetInputConnection(fitsReader->GetOutputPort());
    //shellE->SetValue(0, 10.0f);


    // decimate
    //vtkDecimate *shellD = vtkDecimate::New();
    //shellD->SetInput(shellE->GetOutput());
    //shellD->SetTargetReduction(0.7);

    vtkPolyDataMapper *shellM = vtkPolyDataMapper::New();
    shellM->SetInput(shellE->GetOutput());
    //shellM->SetInput(shellD->GetOutput());
    shellM->ScalarVisibilityOff();

    vtkActor *shellA = vtkActor::New();
    shellA->SetMapper(shellM);
    shellA->GetProperty()->SetColor(1.0, 0.5, 1.0);


    // slice
    vtkImageDataGeometryFilter *sliceE =
                    vtkImageDataGeometryFilter::New();
    // values are clamped
    sliceE->SetExtent(0, 5000, 0, 5000, 13, 13);
    sliceE->SetInputConnection(fitsReader->GetOutputPort());

    vtkPolyDataMapper *sliceM = vtkPolyDataMapper::New();
    sliceM->SetInput(sliceE->GetOutput());
    sliceM->ScalarVisibilityOn();
    double *range;
    range = fitsReader->GetOutput()->GetScalarRange();
    sliceM->SetScalarRange(range);

    vtkActor *sliceA = vtkActor::New();
    sliceA->SetMapper(sliceM);

    // add actors to renderer
    ren1->AddActor(outlineA);
    ren1->AddActor(shellA);
    //Commentato per ora
    ren1->AddActor(sliceA);



    /*vtkSmartPointer<vtkCallbackCommand> keypressCallback = vtkSmartPointer<vtkCallbackCommand>::New();
      keypressCallback->SetCallback (KeypressCallbackFunction2);
      iren->AddObserver (
        vtkCommand::KeyPressEvent,
        keypressCallback );*/

    // Render an image; since no lights/cameras specified, created automatically
    renWin->Render();

       // uncomment to write VRML
       //vtkVRMLExporter *vrml = vtkVRMLExporter::New();
       //vrml->SetRenderWindow(renWin);
       //vrml->SetFileName("out.wrl");
       //vrml->Write();

       // Begin mouse interaction
       iren->Start();
}
