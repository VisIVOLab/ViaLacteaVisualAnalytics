#-------------------------------------------------
#
# Project created by QtCreator 2015-05-18T11:03:07
#
#-------------------------------------------------

QT += core gui widgets network networkauth printsupport xml concurrent webenginewidgets
#QMAKE_MAC_SDK = macosx10.15
CONFIG+=sdk_no_version_check
#CONFIG-=app_bundle

TARGET = ViaLacteaVisualAnalytics
TEMPLATE = app

ICON = logo.icns

INCLUDEPATH += $$PWD/../../3rd_party/VTK-9.0.3/_install/include/vtk-9.0 \
               $$PWD/../../3rd_party/cfitsio-3.49/_install/include \
               $$PWD/../../3rd_party/boost_1_75_0

LIBS += -L$$PWD/../../3rd_party/cfitsio-3.49/_install/lib -lcfitsio
LIBS += -L$$PWD/../../3rd_party/VTK-9.0.3/_install/lib \
            -lvtkChartsCore-9.0 \
            -lvtkCommonColor-9.0 \
            -lvtkCommonComputationalGeometry-9.0 \
            -lvtkCommonCore-9.0 \
            -lvtkCommonDataModel-9.0 \
            -lvtkCommonExecutionModel-9.0 \
            -lvtkCommonMath-9.0 \
            -lvtkCommonMisc-9.0 \
            -lvtkCommonSystem-9.0 \
            -lvtkCommonTransforms-9.0 \
            -lvtkDICOMParser-9.0 \
            -lvtkDomainsChemistry-9.0 \
            -lvtkdoubleconversion-9.0 \
            -lvtkexpat-9.0 \
            -lvtkFiltersAMR-9.0 \
            -lvtkFiltersCore-9.0 \
            -lvtkFiltersExtraction-9.0 \
            -lvtkFiltersFlowPaths-9.0 \
            -lvtkFiltersGeneral-9.0 \
            -lvtkFiltersGeneric-9.0 \
            -lvtkFiltersGeometry-9.0 \
            -lvtkFiltersHybrid-9.0 \
            -lvtkFiltersHyperTree-9.0 \
            -lvtkFiltersImaging-9.0 \
            -lvtkFiltersModeling-9.0 \
            -lvtkFiltersParallel-9.0 \
            -lvtkFiltersParallelImaging-9.0 \
            -lvtkFiltersProgrammable-9.0 \
            -lvtkFiltersSMP-9.0 \
            -lvtkFiltersSelection-9.0 \
            -lvtkFiltersSources-9.0 \
            -lvtkFiltersStatistics-9.0 \
            -lvtkFiltersTexture-9.0 \
            -lvtkFiltersVerdict-9.0 \
            -lvtkglew-9.0 \
            -lvtkGUISupportQt-9.0 \
            -lvtkGUISupportQtSQL-9.0 \
            -lvtkGeovisCore-9.0 \
            -lvtkIOAMR-9.0 \
            -lvtkIOCore-9.0 \
            -lvtkIOEnSight-9.0 \
            -lvtkIOExodus-9.0 \
            -lvtkIOExport-9.0 \
            -lvtkIOGeometry-9.0 \
            -lvtkIOImage-9.0 \
            -lvtkIOImport-9.0 \
            -lvtkIOInfovis-9.0 \
            -lvtkIOLSDyna-9.0 \
            -lvtkIOLegacy-9.0 \
            -lvtkIOMINC-9.0 \
            -lvtkIOMovie-9.0 \
            -lvtkIOPLY-9.0 \
            -lvtkIOParallel-9.0 \
            -lvtkIOParallelXML-9.0 \
            -lvtkIOSQL-9.0 \
            -lvtkIOVideo-9.0 \
            -lvtkIOXML-9.0 \
            -lvtkIOXMLParser-9.0 \
            -lvtkImagingColor-9.0 \
            -lvtkImagingCore-9.0 \
            -lvtkImagingFourier-9.0 \
            -lvtkImagingGeneral-9.0 \
            -lvtkImagingHybrid-9.0 \
            -lvtkImagingMath-9.0 \
            -lvtkImagingMorphological-9.0 \
            -lvtkImagingSources-9.0 \
            -lvtkImagingStatistics-9.0 \
            -lvtkImagingStencil-9.0 \
            -lvtkInfovisCore-9.0 \
            -lvtkInfovisLayout-9.0 \
            -lvtkInteractionImage-9.0 \
            -lvtkInteractionStyle-9.0 \
            -lvtkInteractionWidgets-9.0 \
            -lvtkloguru-9.0 \
            -lvtklz4-9.0 \
            -lvtklzma-9.0 \
            -lvtkParallelCore-9.0 \
            -lvtkParallelDIY-9.0 \
            -lvtkRenderingAnnotation-9.0 \
            -lvtkRenderingContext2D-9.0  \
            -lvtkRenderingCore-9.0 \
            -lvtkRenderingFreeType-9.0 \
            -lvtkRenderingFreeType-9.0 \
            -lvtkRenderingGL2PSOpenGL2-9.0 \
            -lvtkRenderingImage-9.0 \
            -lvtkRenderingLOD-9.0 \
            -lvtkRenderingLabel-9.0 \
            -lvtkRenderingOpenGL2-9.0 \
            -lvtkRenderingQt-9.0 \
            -lvtkRenderingUI-9.0 \
            -lvtkRenderingVolume-9.0 \
            -lvtkRenderingVolumeOpenGL2-9.0 \
            -lvtkViewsContext2D-9.0 \
            -lvtkViewsCore-9.0 \
            -lvtkViewsInfovis-9.0 \
            -lvtkViewsQt-9.0 \
            -lvtkexodusII-9.0 \
            -lvtkfreetype-9.0 \
            -lvtkgl2ps-9.0  \
            -lvtkjsoncpp-9.0 \
            -lvtkmetaio-9.0 \
            -lvtkogg-9.0  \
            -lvtksqlite-9.0 \
            -lvtksys-9.0 \
            -lvtkverdict-9.0 \
            -lvtkzlib-9.0 \

macx:LIBS +=  -framework \
    Foundation \
    -framework \
    Cocoa \
    -framework \
    GLUT \
    -framework \
    QTKit \
    -framework \
    OpenGL \
    -framework \
    AGL \
    -framework \
    IOKit \
    -framework \
    QtPrintSupport



SOURCES += src/main.cpp \
    src/caesarwindow.cpp \
    src/authwrapper.cpp \
    src/mainwindow.cpp \
    src/treemodel.cpp \
    src/treeitem.cpp \
    src/vlvaurlschemehandler.cpp \
    src/vtkfitsreader.cpp \
    src/vispoint.cpp \
    src/observedobject.cpp \
    src/operationqueue.cpp \
    src/sednode.cpp \
    src/sed.cpp \
    src/pointspipe.cpp \
    src/pipe.cpp \
    src/vtkellipse.cpp \
    src/base64.cpp \
    src/astroutils.cpp \
    src/libwcs/fitsfile.c \
    src/libwcs/hget.c \
    src/libwcs/fileutil.c \
    src/libwcs/fitswcs.c \
    src/libwcs/distort.c \
    src/libwcs/hput.c \
    src/libwcs/iget.c \
    src/libwcs/imio.c \
    src/libwcs/imhfile.c \
    src/libwcs/dateutil.c \
    src/libwcs/wcsinit.c \
    src/libwcs/dsspos.c \
    src/libwcs/wcs.c \
    src/libwcs/wcstrig.c \
    src/libwcs/wcscon.c \
    src/libwcs/lin.c \
    src/libwcs/platepos.c \
    src/libwcs/tnxpos.c \
    src/libwcs/wcslib.c \
    src/libwcs/cel.c \
    src/libwcs/proj.c \
    src/libwcs/sph.c \
    src/libwcs/worldpos.c \
    src/vialacteasource.cpp \
    src/vlkbquery.cpp \
    src/loadingwidget.cpp \
    src/sedvisualizerplot.cpp \
    src/qcustomplot.cpp \
    src/sedplotpointcustom.cpp \
    src/sedfitgrid_thin.cpp \
    src/sedfitgrid_thick.cpp \
    src/luteditor.cpp \
    src/vtklegendscaleactor.cpp \
    src/vtktoolswidget.cpp \
    src/color.cpp \
    src/vtkfitstoolswidget.cpp \
    src/higalselectedsources.cpp \
    src/plotwindow.cpp \
    src/vlkbquerycomposer.cpp \
    src/vlkbtable.cpp \
    src/fitsimagestatisiticinfo.cpp \
    src/vlkbsimplequerycomposer.cpp \
    src/dbquery.cpp \
    src/xmlparser.cpp \
    src/vialactea_fileload.cpp \
    src/selectedsourcefieldsselect.cpp \
    src/downloadmanager.cpp \
    src/viewselectedsourcedataset.cpp \
    src/vialactea.cpp \
    src/contour.cpp \
    src/histogram.cpp \
    src/lutselector.cpp \
    src/vtkwindow_new.cpp \
    src/vtkfitstoolwidget_new.cpp \
    src/vtkfitstoolwidgetobject.cpp \
    src/vialacteainitialquery.cpp \
    src/settingform.cpp \
    src/aboutform.cpp \
    src/selectedsourcesform.cpp \
    src/lutcustomize.cpp \
    src/vtkextracthistogram.cpp \
    src/extendedglyph3d.cpp \
    src/visivoimporterdesktop.cpp \
    src/vstabledesktop.cpp \
    src/visivoutilsdesktop.cpp \
    src/vsobjectdesktop.cpp \
    src/filtercustomize.cpp \
    src/vialacteastringdictwidget.cpp \


HEADERS += src/mainwindow.h \
    src/authkeys.h \
    src/caesarwindow.h \
    src/authwrapper.h \
    src/singleton.h \
    src/treemodel.h \
    src/treeitem.h \
    src/vlvaurlschemehandler.h \
    src/vtkfitsreader.h \
    src/vispoint.h \
    src/observedobject.h \
    src/operationqueue.h \
    src/sednode.h \
    src/sed.h \
    src/pointspipe.h \
    src/pipe.h \
    src/vtkellipse.h \
    src/base64.h \
    src/astroutils.h \
    src/libwcs/fitsfile.h \
    src/libwcs/wcs.h \
    src/vialacteasource.h \
    src/vlkbquery.h \
    src/loadingwidget.h \
    src/sedvisualizerplot.h \
    src/qcustomplot.h \
    src/sedplotpointcustom.h \
    src/sedfitgrid_thin.h \
    src/sedfitgrid_thick.h \
    src/luteditor.h \
    src/vtklegendscaleactor.h \
    src/vtktoolswidget.h \
    src/color.h \
    src/vtkfitstoolswidget.h \
    src/higalselectedsources.h \
    src/plotwindow.h \
    src/vlkbquerycomposer.h \
    src/vlkbtable.h \
    src/fitsimagestatisiticinfo.h \
    src/vlkbsimplequerycomposer.h \
    src/dbquery.h \
    src/xmlparser.h \
    src/extendedglyph3d.h \
    src/vialactea_fileload.h \
    src/selectedsourcefieldsselect.h \
    src/downloadmanager.h \
    src/viewselectedsourcedataset.h \
    src/vialactea.h \
    src/contour.h   \
    src/histogram.h \
    src/lutselector.h \
    src/vtkwindow_new.h \
    src/vtkfitstoolwidget_new.h \
    src/vtkfitstoolwidgetobject.h \
    src/vialacteainitialquery.h \
    src/settingform.h \
    src/aboutform.h \
    src/selectedsourcesform.h \
    src/lutcustomize.h \
    src/vtkextracthistogram.h \
    src/osxhelper.h \
    src/visivoimporterdesktop.h \
    src/vstabledesktop.h \
    src/visivoutilsdesktop.h \
    src/vsobjectdesktop.h \
    src/visivofilterdesktop.h \
    src/filtercustomize.h \
    src/vialacteastringdictwidget.h \


FORMS += ui/mainwindow.ui \
    ui/caesarwindow.ui \
    ui/operationqueue.ui \
    ui/vtkwindow.ui \
    ui/vlkbquery.ui \
    ui/loadingwidget.ui \
    ui/sedvisualizerplot.ui \
    ui/sedfitgrid_thin.ui \
    ui/sedfitgrid_thick.ui \
    ui/vtktoolswidget.ui \
    ui/vtkfitstoolswidget.ui \
    ui/higalselectedsources.ui \
    ui/plotwindow.ui \
    ui/vlkbquerycomposer.ui \
    ui/fitsimagestatisiticinfo.ui \
    ui/vlkbsimplequerycomposer.ui \
    ui/dbquery.ui \
    ui/vialactea_fileload.ui \
    ui/selectedsourcefieldsselect.ui \
    ui/viewselectedsourcedataset.ui \
    ui/vialactea.ui \
    ui/contour.ui \
    ui/vtkwindow_new.ui \
    ui/vtkfitstoolwidget_new.ui \
    ui/vialacteainitialquery.ui \
    ui/settingform.ui \
    ui/aboutform.ui \
    ui/selectedsourcesform.ui \
    ui/lutcustomize.ui \
    ui/filtercustomize.ui \
    ui/vialacteastringdictwidget.ui

RESOURCES += \
   visivo.qrc

DISTFILES +=

OBJECTIVE_SOURCES += \
    src/osxhelper.mm
