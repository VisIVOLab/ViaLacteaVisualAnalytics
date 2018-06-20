#ifndef vtkwindow_new_H
#define vtkwindow_new_H

#include <QMainWindow>
#include "pointspipe.h"
#include "ui_vtkwindow_new.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkCommand.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkConeSource.h"
#include "vtkSphereSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkSphereSource.h"
#include "vtkConeSource.h"
#include "vtkCylinderSource.h"
#include "vtkCubeSource.h"
#include "vtkMarchingCubes.h"

#include "vtkCamera.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkLookupTable.h"
#include "contour.h"
#include "vtkLineSource.h"

#include "vtkFloatArray.h"
#include "vtkCellArray.h"
#include"vtkGlyph3D.h"
#include "vtkScalarBarActor.h"
#include "vtkOutlineCornerFilter.h"
#include "vtkProperty.h"

#include "vtkGenericRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkActor.h"
#include "vtkAxesActor.h"
#include "vtkLODActor.h"
#include <map>
#include "vtkImageActor.h"
#include "vtkImageViewer2.h"
#include <vtkSmartPointer.h>
#include "vtkfitsreader.h"
#include "vtkellipse.h"
#include "vtkOrientationMarkerWidget.h"
#include "vtkAxes.h"
#include "vtkEventQtSlotConnect.h"
#include <QGestureEvent>
#include "vtkResliceImageViewer.h"
#include "vtkfitstoolwidget_new.h"
#include "loadingwidget.h"
#include <QNetworkReply>
#include "vtkImageStack.h"

#include "vialacteastringdictwidget.h"

class vtkRenderer;
class vtkEventQtSlotConnect;
class vtkObject;
class vtkCommand;
class VSTableDesktop;
class vtkPiecewiseFunction;
class vtkColorTransferFunction;
class VisPoint;
class vtkfitstoolswidget;
class FitsImageStatisiticInfo;
class contour;

namespace Ui {
class vtkwindow_new;
}

class vtkwindow_new : public QMainWindow
{
    Q_OBJECT
    vtkLODActor *m_pActor;
    vtkVolumeProperty  *m_volumeProperty;
    //vtkPiecewiseFunction *m_opacityTransferFunction;
    vtkColorTransferFunction *m_colorTransferFunction;

public:
    explicit vtkwindow_new(QWidget *parent = 0, VisPoint *vis=0);
    explicit vtkwindow_new(QWidget *parent = 0, vtkSmartPointer<vtkFitsReader> vis=0, int b=0, vtkwindow_new *p=0);
    //explicit vtkwindow_new(QWidget *parent = 0, vtkImageActor *vis=0);
    ~vtkwindow_new();
    vtkRenderer* m_Ren1;
    vtkRenderWindow* renwin;

    vtkRenderer* m_Ren2;
    vtkRenderer* m_Ren3;
    vtkRenderWindow* renwin2;
    vtkwindow_new *vtkcontourwindow;
    vtkwindow_new *myParentVtkWindow;
    contour *contourWin;
    vtkSmartPointer<vtkLineSource> lineSource;

    vtkMarchingCubes *shellE;
    vtkActor *sliceA;

    VSTableDesktop * table;
    VisPoint * vispoint;
    void changePalette(std::string palette);
    void changeFitsPalette(std::string palette);
    void changeFitsScale(std::string palette, std::string scale);
    void changeScalar(std::string scalar);
    PointsPipe * pp;
    void resetCamera();
    void drawEllipse(QHash<QString, vtkEllipse *> ellipse , QString sourceFilename);
    void drawGlyphs(int index);
    Ui::vtkwindow_new *ui;
    static void SelectionChangedCallbackFunction (vtkObject* caller, long unsigned int eventId, void* clientData, void* callData );
    QString getWindowName();
    //  QString getFilenameWithPath();
    void setWindowName(QString name);
    //  void isVisible();
    QHash<QString,  vtkSmartPointer<vtkLODActor> > getEllipseActorList();
    QHash<QString,  vtkSmartPointer<vtkLODActor> > getVisualizedActorList();

    double r;
    double g;
    double b;
    bool isDatacube;
    QString survey;
    QString species;
    QString transition;
    double max;
    double min;
    long naxis3;
    double *z_range;
    vtkSmartPointer<vtkImageStack> imageStack ;

    vtkSmartPointer<vtkResliceImageViewer> viewer;

    vtkSmartPointer<vtkImageViewer2> imageViewer;
    void addSources(VSTableDesktop* m_VisIVOTable);
    void addFilaments(VSTableDesktop* m_VisIVOTable);
    void addSourcesFromBM(VSTableDesktop* m_VisIVOTable);
    void addBubble(VSTableDesktop* m_VisIVOTable);


    //void drawSingleEllipse(QList<vtkEllipse *> ellipse, QString sourceFilename );
    //void drawSingleEllipse(vtkEllipse * ellipse );
    void drawSingleEllipse(vtkSmartPointer<vtkLODActor> ellipseActor);
    void removeSingleEllipse(vtkSmartPointer<vtkLODActor> ellipseActor);
    void setContourVisualized(bool s) {contourVisualized=s;}
    bool getContourVisualized() {return contourVisualized;}
    QList<vtkfitstoolwidgetobject*> getLayerListImages(){return imgLayerList;}
    QList<vtkfitstoolwidgetobject*> getLayerListElements(){return elementLayerList;}

    void addLayerImage(vtkSmartPointer<vtkFitsReader> vis, QString survey="", QString species="", QString transition="");

    QString getSelectedScale(){return selected_scale;}

    // QList<vtkEllipse*> ellipse_list;
    QHash<QString, vtkEllipse* > getEllipseList(){return ellipse_list;}
    QHash<QString, vtkEllipse* > getFtEllipseList(){return ft_ellipse_list;}
    QHash<QString,  QString > getDesignation2fileMap() {return designation2fileMap;}

    void printSelf();

    void changeWCS(bool galaptic);
    std::string filenameWithPath;




    QHash<QString, int> file_wavelength;
    vtkSmartPointer<vtkActor> selectedActor;
    void addActor(vtkProp *actor);
    void removeActor(vtkProp *actor);

    void setCallingL(QString v){called_l=v;};
    void setCallingB(QString v){called_b=v;};
    void setCallingR(QString v){called_r=v;};
    void setCallingDl(QString v){called_dl=v;};
    void setCallingDb(QString v){called_db=v;};

    vtkSmartPointer<vtkFitsReader> getFitsImage(){return myfits;};
    vtkSmartPointer<vtkActor> getGlyphActor(){return glyph_actor;};

signals:
    void speciesChanged();
    void surveyChanged();
    void transitionChanged();


private:
    QHash<QString, QPair<double,double> > addedFilter;

    QString selected_scale;
    double cam_init_pos[3];
    double cam_init_foc[3];
    bool scaleActivate;
    bool fitsViewer;
    vtkSmartPointer<vtkRenderer> back_ren=0;
    //contour *contourWin; //=0;
    vtkSmartPointer<vtkFitsReader> myfits;
    LoadingWidget *loading;
    bool contourVisualized=false;
    bool coloBarVisualized=false;
    bool contourWinActivated=false;
    vtkActor *outline_actor=0;
    vtkActor *contour_actor=0;
    vtkActor *data_geometry_actor=0;
    vtkSmartPointer<vtkActor> glyph_actor=0;
    QList<QMap<QString, QString> > classElementsOnDb;
    QString called_l;
    QString called_b;
    QString called_r;
    QString called_dl;
    QString called_db;
    int vtkwintype;

    vtkfitstoolwidget_new *vtkfitstoolwindow;

    QString windowName;
    //QString filenameWithPath;
    QHash<QString,  vtkSmartPointer<vtkLODActor> > ellipse_actor_list;
    QHash<QString,  vtkSmartPointer<vtkLODActor> > visualized_actor_list;

    QHash<QString,  QString > designation2fileMap;
    QHash<QString,   vtkSmartPointer<vtkLODActor> > VisualizedEllipseSourcesList;
    vtkfitstoolswidget *vtkfitstoolsw;
    QHash<QString, vtkEllipse* > ellipse_list;
    QHash<QString, vtkEllipse* > ft_ellipse_list;
    vtkSmartPointer<vtkOrientationMarkerWidget> vtkAxesWidget;
    vtkSmartPointer<vtkAxesActor> vtkAxes;
    FitsImageStatisiticInfo *info;
    vtkSmartPointer<vtkEventQtSlotConnect> Connections;
    vtkfitstoolwidgetobject *imageObject;
    QList<vtkfitstoolwidgetobject*> imgLayerList;
    QList<vtkfitstoolwidgetobject*> elementLayerList;
    vtkSmartPointer<vtkLODActor> rectangleActor=0;
    vtkSmartPointer<vtkActor> currentContourActor;
    vtkSmartPointer<vtkActor> currentContourActorForMainWindow;
    QString vlkbUrl;
    QString selectedCubeVelocityUnit;
    void drawRectangleFootprint(double points[8]);
    VialacteaStringDictWidget *stringDictWidget;
    void addCombinedLayer(QString name,  vtkSmartPointer<vtkLODActor>actor, int objtype, bool active);

public slots:
    //void updateCoords(vtkObject*);
    //void popup(vtkObject * obj, unsigned long, void * client_data, void *, vtkCommand * command);
    void loadObservedObject(VisPoint *vis);
    void updateScene();
    void showBox(bool checked);
    void showAxes(bool checked);
    void showColorbar(bool checked);
    void setCameraElevation(double el);
    void setCameraAzimuth(double az);
    void scale(bool checked);
    void showGrid(bool checked);
    FitsImageStatisiticInfo* getInfoWindow();
    void createInfoWindow();
    void setSelectionFitsViewerInteractorStyle();
    void setSkyRegionSelectorInteractorStyle();
    void slot_clicked(vtkObject*, unsigned long, void*, void*);
    void setVtkInteractorStyleImage();
    void setVtkInteractorStyleImageContour();

    void setVtkInteractorStyle3DPicker(vtkSmartPointer<vtkPolyData> points);
    void setSelectedActor(vtkSmartPointer<vtkActor> sel);
    void setSkyRegionSelectorInteractorStyleFor3D();
    void setVtkInteractorStyleFreehand();
    void setVtkInteractorStyle3DFreehand(vtkSmartPointer<vtkPolyData> points);
    void setVtkInteractorContourWindow();
    void setSliceDatacube(int i);

    void plotSlice(vtkSmartPointer<vtkFitsReader> visvis, int arg1);
    void updateSpecies();
    void updateSurvey();
    void updateTransition();

    void setSurvey(QString q);
    void setSpecies(QString q);
    void setTransition(QString q);

    void setDbElements(QList<QMap<QString, QString> > elementsOnDb);
    void setSelectedCubeVelocityUnit (QString v){selectedCubeVelocityUnit=v;}
    //void setCuttingPlane(int value);
    void downloadStartingLayers(QList<QPair<QString, QString> > selectedSurvey);




protected:
    //vtkEventQtSlotConnect* Connections;
    vtkVolume *m_volume;
    void closeEvent(QCloseEvent *event);

private slots:
    void addLayer(vtkfitstoolwidgetobject *o, bool enabled = true);
    void addTreeChild(QTreeWidgetItem *parent, QString name, QBrush brush);
    QTreeWidgetItem* addTreeRoot(QString name);

    void on_actionTools_triggered();
    void on_actionInfo_triggered();
    void addLocalSources();
    void cutoutDatacube(QString c );


    void on_cameraLeft_clicked();
    void on_bottomCamera_clicked();
    void on_topCamera_clicked();
    void on_frontCamera_clicked();
    void on_cameraRight_clicked();
    void on_cameraBack_clicked();
    void on_resetPushButton_clicked();
    void on_confirmPushButton_clicked();
    void on_rectSelection_clicked();
    void on_freehandPushButton_clicked();
    void on_cuttingPlane_Slider_valueChanged(int value);
    void on_spinBox_cuttingPlane_valueChanged(int arg1);
    void handleButton(int i);

    void setCuttingPlaneValue(int arg1);


    //void on_cuttingPlane_Slider_sliderMoved(int position);
    // void on_horizontalSlider_threshold_sliderMoved(int position);


    void on_spinBox_contour_valueChanged(int arg1);
    void on_contour_pushButton_clicked();
    void on_cuttingPlane_Slider_sliderMoved(int position);
    //void on_horizontalSlider_threshold_actionTriggered(int action);
    void on_PVPlotPushButton_clicked();
    //void on_generatePushButton_clicked();
    void on_PVPlot_radioButton_clicked(bool checked);
    void on_horizontalSlider_threshold_valueChanged(int value);
    void on_horizontalSlider_threshold_sliderReleased();
    void on_rectangularSelectionCS_clicked();
    void on_colorPushButton_clicked();
    void addToList(vtkfitstoolwidgetobject *o, bool enabled=true);
    void addImageToList( vtkfitstoolwidgetobject *o);

    void checkboxImageClicked(int cb);
    void checkboxClicked(int cb, bool status =false);

    void on_tableWidget_doubleClicked(const QModelIndex &index);
    void on_fil_rectPushButton_clicked();


    void on_lutComboBox_activated(const QString &arg1);
    void on_logRadioButton_toggled(bool checked);
    void on_tdRectPushButton_clicked();
    void on_ElementListWidget_doubleClicked(const QModelIndex &index);
    void on_thresholdValueLineEdit_editingFinished();
    void on_upperBoundLineEdit_editingFinished();
    void on_lowerBoundLineEdit_editingFinished();
    void removeContour();
    void goContour();

    void on_levelLineEdit_editingFinished();
    void on_contourCheckBox_clicked(bool checked);
    void on_lut3dActivateCheckBox_clicked(bool checked);
    void on_scalarComboBox_activated(const QString &arg1);
    void on_lut3dComboBox_activated(const QString &arg1);
    void on_toolButton_clicked();
    void on_glyphActivateCheckBox_clicked(bool checked);
    void on_linear3dRadioButton_toggled(bool checked);
    void on_glyphShapeComboBox_activated(int index);
    void on_horizontalSlider_valueChanged(int value);
    void on_glyphScalarComboBox_activated(const QString &arg1);
    void on_glyphScalingLineEdit_returnPressed();
    void on_ElementListWidget_clicked(const QModelIndex &index);
    bool eventFilter(QObject *object, QEvent *event);
    void on_listWidget_clicked(const QModelIndex &index);
    void on_listWidget_itemChanged(QListWidgetItem *item);
    void movedLayersRow( const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow );
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);
    void on_listWidget_customContextMenuRequested(const QPoint &pos);
    void sendImageTo(QString id);

    void on_bubblePushButton_clicked();
    void on_filterMoreButton_clicked();
};

#endif // vtkwindow_new_H
