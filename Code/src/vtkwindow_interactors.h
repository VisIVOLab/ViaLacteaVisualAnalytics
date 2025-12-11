#ifndef VTKWINDOW_INTERACTORS_H
#define VTKWINDOW_INTERACTORS_H

#include <vtkInteractorStyleDrawPolygon.h>
#include <vtkInteractorStyleRubberBand2D.h>
#include <vtkInteractorStyleRubberBandPick.h>
#include <vtkInteractorStyleImage.h>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkPropPicker.h>
#include <vtkPointPicker.h>
#include <vtkSmartPointer.h>
#include <functional>

class vtkwindow_new;
class vtkPolyData;
class vtkActor;
class vtkPolyDataMapper;
class vtkPoints;
class vtkLODActor;
class vtkCoordinate;
class vtkRenderWindow;

class InteractorStyleFreeHandOn3DVisualization : public vtkInteractorStyleDrawPolygon
{
private:
    vtkwindow_new *vtkwin;
    vtkSmartPointer<vtkPolyData> Points;
    vtkSmartPointer<vtkPolyData> Points_ori;
    vtkSmartPointer<vtkActor> SelectedActor;
    vtkSmartPointer<vtkPolyDataMapper> SelectedMapper;

public:
    static InteractorStyleFreeHandOn3DVisualization *New();
    vtkTypeMacro(InteractorStyleFreeHandOn3DVisualization, vtkInteractorStyleDrawPolygon);

    InteractorStyleFreeHandOn3DVisualization();
    void OnLeftButtonUp() override;
    void SetPoints(vtkSmartPointer<vtkPolyData> points);
    void setVtkWin(vtkwindow_new *w);

    void PrintSelf(std::ostream &os, vtkIndent indent) override { }
    void PrintHeader(std::ostream &os, vtkIndent indent) { }
    void PrintTrailer(std::ostream &os, vtkIndent indent) { }
    void CollectRevisions(std::ostream &os) { }
};

class InteractorStyleSelctionPointOn3DVisualization : public vtkInteractorStyleRubberBandPick
{
private:
    vtkwindow_new *vtkwin;
    vtkSmartPointer<vtkPolyData> Points;
    vtkSmartPointer<vtkPolyData> Points_ori;
    vtkSmartPointer<vtkActor> SelectedActor;
    vtkSmartPointer<vtkPolyDataMapper> SelectedMapper;

public:
    static InteractorStyleSelctionPointOn3DVisualization *New();
    vtkTypeMacro(InteractorStyleSelctionPointOn3DVisualization, vtkInteractorStyleRubberBandPick);

    InteractorStyleSelctionPointOn3DVisualization();
    void OnLeftButtonUp() override;
    void SetPoints(vtkSmartPointer<vtkPolyData> points);
    void setVtkWin(vtkwindow_new *w);

    void PrintSelf(std::ostream &os, vtkIndent indent) override { }
    void PrintHeader(std::ostream &os, vtkIndent indent) { }
    void PrintTrailer(std::ostream &os, vtkIndent indent) { }
    void CollectRevisions(std::ostream &os) { }
};

class MyRubberBand : public vtkInteractorStyleRubberBand2D
{
private:
    vtkwindow_new *vtkwin;

public:
    static MyRubberBand *New();
    vtkTypeMacro(MyRubberBand, vtkInteractorStyleRubberBand2D);

    void setVtkWin(vtkwindow_new *w);
    void OnMouseMove() override;
};

class myVtkInteractorStyleImage : public vtkInteractorStyleImage
{
private:
    vtkwindow_new *vtkwin;
    int mouseButtonDown;
    double window_init;
    double level_init;
    bool isSlice;

public:
    static myVtkInteractorStyleImage *New();
    vtkTypeMacro(myVtkInteractorStyleImage, vtkInteractorStyleImage);
    myVtkInteractorStyleImage();

    void setVtkWin(vtkwindow_new *w);
    void setIsSlice(bool value = true);
    void OnMouseMove() override;
    void OnLeftButtonDown() override;
    void OnLeftButtonUp() override;
    void OnMiddleButtonDown() override;
    void OnMiddleButtonUp() override;
    void OnChar() override;

    void PrintSelf(std::ostream &os, vtkIndent indent) override { }
    void PrintHeader(std::ostream &os, vtkIndent indent) { }
    void PrintTrailer(std::ostream &os, vtkIndent indent) { }
    void CollectRevisions(std::ostream &os) { }
};

class SkyRegionSelector : public vtkInteractorStyleRubberBand2D
{
private:
    vtkwindow_new *vtkwin;
    bool is3D, isFilament, is3dSelections, isBubble;

public:
    static SkyRegionSelector *New();
    vtkTypeMacro(SkyRegionSelector, vtkInteractorStyleRubberBand2D);

    SkyRegionSelector();

    void setIsFilament();
    void setIsBubble();
    void setIs3dSelections();
    void setIs3D();
    void setVtkWin(vtkwindow_new *w);
    vtkwindow_new *getVtkWin();

    void OnLeftButtonUp() override;

    void PrintSelf(std::ostream &os, vtkIndent indent) override { }
    void PrintHeader(std::ostream &os, vtkIndent indent) { }
    void PrintTrailer(std::ostream &os, vtkIndent indent) { }
    void CollectRevisions(std::ostream &os) { }
};

class InteractorStyleExtractSources : public vtkInteractorStyleRubberBand2D
{
public:
    static InteractorStyleExtractSources *New();
    vtkTypeMacro(InteractorStyleExtractSources, vtkInteractorStyleRubberBand2D);

    std::function<void(int rect[4])> Callback;

    void OnLeftButtonUp() override;

protected:
    InteractorStyleExtractSources() = default;

private:
    InteractorStyleExtractSources(const InteractorStyleExtractSources &) = delete;
    void operator=(const InteractorStyleExtractSources &) = delete;
};

class InteractorStyleEditSource : public vtkInteractorStyleTrackballActor
{
public:
    static InteractorStyleEditSource *New();
    vtkTypeMacro(InteractorStyleEditSource, vtkInteractorStyleTrackballActor);

    void OnMouseMove() override;
    void OnLeftButtonDown() override;
    void OnLeftButtonUp() override;
    void OnMiddleButtonDown() override { }
    void OnMiddleButtonUp() override { }
    void OnRightButtonDown() override { }
    void OnRightButtonUp() override { }

    void SetSource(vtkSmartPointer<vtkPoints> points, vtkSmartPointer<vtkLODActor> actor);
    void setActorFilter(vtkLODActor *act);
    void setActorPicker(vtkPropPicker *actorPicker);
    void setPointPicker(vtkPointPicker *pointPicker);
    void setRenderer(vtkRenderer *ren);
    void SetRenderWindow(vtkRenderWindow *rw);

protected:
    InteractorStyleEditSource();
    vtkSmartPointer<vtkCoordinate> Coordinate;
    vtkSmartPointer<vtkPropPicker> ActorPicker;
    vtkSmartPointer<vtkPointPicker> PointPicker;
    vtkSmartPointer<vtkPoints> Points;
    vtkLODActor *Actor;
    vtkLODActor *ActorFilter;
    vtkRenderer *Renderer;
    vtkRenderWindow *RenderWindow = nullptr;
    bool moving;
    vtkIdType PointId;

private:
    InteractorStyleEditSource(const InteractorStyleEditSource &) = delete;
    void operator=(const InteractorStyleEditSource &) = delete;
};

#endif // VTKWINDOW_INTERACTORS_H
