#-------------------------------------------------
#
# Project created by QtCreator 2015-05-18T11:03:07
#
#-------------------------------------------------

QT       += core gui network networkauth printsupport xml  widgets concurrent webenginewidgets
#QT       += core gui network printsupport xml  widgets concurrent webkitwidgets
#CONFIG   += static
QMAKE_MAC_SDK = macosx10.15
CONFIG-=app_bundle

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VisIVODesktop-vtk6_qt5
TEMPLATE = app

VTK_MAJOR_VERSION=9.0
DEFINES += VTK_MAJOR_VERSION


ICON = logo.icns
#win32:RC_ICONS += your_icon.ico


#INCLUDEPATH +=   /opt/vtk6.20/include/vtk-6.2/
#INCLUDEPATH += /opt/vtk-9.0.0/include/vtk-9.0/
#INCLUDEPATH +=   /opt/cfitsio3_1_0/include/ \
#                /opt/local/include \
#                /opt/boost_1_63_0/include/

#INCLUDEPATH += /Users/fxbio6600/OACT/VisIVOServer_svn_locale/branches/2.3/
#INCLUDEPATH += /Users/eva/Projects/NEANIAS/Code/VisIVOServer
#INCLUDEPATH +=/opt/hdf5-1.10.0-patch1/include
#LIBS  +=  /opt/cfitsio3_1_0/lib/libcfitsio.a

INCLUDEPATH += /usr/local/include/vtk-9.0 \
               /usr/local/lib/ \
               /home/gt/software/cfitsio-3.49/include \
               /home/gt/software/boost-1.75.0/include \
               /Users/evgeniya/neanias/cfitsio-3.49/include/ \
               /Users/evgeniya/neanias/cfitsio-3.49/lib/ \
               /usr/local/include/


#LIBS += -L/opt/vtk-9.0.0/lib
LIBS += -L/usr/local/lib/
LIBS += -L/home/gt/software/cfitsio-3.49/lib -lcfitsio
LIBS += -L/home/gt/software/boost-1.75.0/lib
LIBS += -L/Users/evgeniya/neanias/cfitsio-3.49/lib/ -lcfitsio
LIBS += -lvtkChartsCore-9.0 -lvtkCommonColor-9.0 -lvtkCommonComputationalGeometry-9.0 -lvtkCommonCore-9.0 -lvtkCommonDataModel-9.0 -lvtkCommonExecutionModel-9.0 -lvtkCommonMath-9.0 -lvtkCommonMisc-9.0 -lvtkCommonSystem-9.0 -lvtkCommonTransforms-9.0 -lvtkDICOMParser-9.0 -lvtkDomainsChemistry-9.0 -lvtkFiltersAMR-9.0 -lvtkFiltersCore-9.0 -lvtkFiltersExtraction-9.0 -lvtkFiltersFlowPaths-9.0 -lvtkFiltersGeneral-9.0 -lvtkFiltersGeneric-9.0 -lvtkFiltersGeometry-9.0 -lvtkFiltersHybrid-9.0 -lvtkFiltersHyperTree-9.0 -lvtkFiltersImaging-9.0 -lvtkFiltersModeling-9.0 -lvtkFiltersParallel-9.0 -lvtkFiltersParallelImaging-9.0 -lvtkFiltersProgrammable-9.0 -lvtkFiltersSMP-9.0 -lvtkFiltersSelection-9.0 -lvtkFiltersSources-9.0 -lvtkFiltersStatistics-9.0 -lvtkFiltersTexture-9.0 -lvtkFiltersVerdict-9.0 -lvtkGUISupportQt-9.0 -lvtkGUISupportQtSQL-9.0 -lvtkGeovisCore-9.0 -lvtkIOAMR-9.0 -lvtkIOCore-9.0 -lvtkIOEnSight-9.0 -lvtkIOExodus-9.0 -lvtkIOExport-9.0 -lvtkIOGeometry-9.0 -lvtkIOImage-9.0 -lvtkIOImport-9.0 -lvtkIOInfovis-9.0 -lvtkIOLSDyna-9.0 -lvtkIOLegacy-9.0 -lvtkIOMINC-9.0 -lvtkIOMovie-9.0 -lvtkIOPLY-9.0 -lvtkIOParallel-9.0 -lvtkIOParallelXML-9.0 -lvtkIOSQL-9.0 -lvtkIOVideo-9.0 -lvtkIOXML-9.0 -lvtkIOXMLParser-9.0 -lvtkImagingColor-9.0 -lvtkImagingCore-9.0 -lvtkImagingFourier-9.0 -lvtkImagingGeneral-9.0 -lvtkImagingHybrid-9.0 -lvtkImagingMath-9.0 -lvtkImagingMorphological-9.0 -lvtkImagingSources-9.0 -lvtkImagingStatistics-9.0 -lvtkImagingStencil-9.0 -lvtkInfovisCore-9.0 -lvtkInfovisLayout-9.0 -lvtkInteractionImage-9.0 -lvtkInteractionStyle-9.0 -lvtkInteractionWidgets-9.0 -lvtkParallelCore-9.0 -lvtkRenderingAnnotation-9.0 -lvtkRenderingContext2D-9.0  -lvtkRenderingCore-9.0 -lvtkRenderingFreeType-9.0 -lvtkRenderingFreeType-9.0 -lvtkRenderingGL2PSOpenGL2-9.0 -lvtkRenderingImage-9.0 -lvtkRenderingLOD-9.0 -lvtkRenderingLabel-9.0 -lvtkRenderingOpenGL2-9.0 -lvtkRenderingQt-9.0 -lvtkRenderingVolume-9.0 -lvtkRenderingVolumeOpenGL2-9.0 -lvtkViewsContext2D-9.0 -lvtkViewsCore-9.0 -lvtkViewsInfovis-9.0 -lvtkViewsQt-9.0 -lvtkexodusII-9.0 -lvtkfreetype-9.0 -lvtkgl2ps-9.0  -lvtkjsoncpp-9.0 -lvtkmetaio-9.0 -lvtkogg-9.0  -lvtksqlite-9.0 -lvtksys-9.0 -lvtkverdict-9.0
#LIBS REMOVED
#-lvtkRenderingContextOpenGL2-9.0 -lvtkproj-9.0

#LIBS += -lvtkChartsCore-6.2 -lvtkCommonColor-6.2 -lvtkCommonComputationalGeometry-6.2 -lvtkCommonCore-6.2 -lvtkCommonDataModel-6.2 -lvtkCommonExecutionModel-6.2 -lvtkCommonMath-6.2 -lvtkCommonMisc-6.2 -lvtkCommonSystem-6.2 -lvtkCommonTransforms-6.2 -lvtkDICOMParser-6.2 -lvtkDomainsChemistry-6.2 -lvtkFiltersAMR-6.2 -lvtkFiltersCore-6.2 -lvtkFiltersExtraction-6.2 -lvtkFiltersFlowPaths-6.2 -lvtkFiltersGeneral-6.2 -lvtkFiltersGeneric-6.2 -lvtkFiltersGeometry-6.2 -lvtkFiltersHybrid-6.2 -lvtkFiltersHyperTree-6.2 -lvtkFiltersImaging-6.2 -lvtkFiltersModeling-6.2 -lvtkFiltersParallel-6.2 -lvtkFiltersParallelImaging-6.2 -lvtkFiltersProgrammable-6.2 -lvtkFiltersSMP-6.2 -lvtkFiltersSelection-6.2 -lvtkFiltersSources-6.2 -lvtkFiltersStatistics-6.2 -lvtkFiltersTexture-6.2 -lvtkFiltersVerdict-6.2 -lvtkGUISupportQt-6.2 -lvtkGUISupportQtOpenGL-6.2 -lvtkGUISupportQtSQL-6.2 -lvtkGUISupportQtWebkit-6.2 -lvtkGeovisCore-6.2 -lvtkIOAMR-6.2 -lvtkIOCore-6.2 -lvtkIOEnSight-6.2 -lvtkIOExodus-6.2 -lvtkIOExport-6.2 -lvtkIOGeometry-6.2 -lvtkIOImage-6.2 -lvtkIOImport-6.2 -lvtkIOInfovis-6.2 -lvtkIOLSDyna-6.2 -lvtkIOLegacy-6.2 -lvtkIOMINC-6.2 -lvtkIOMovie-6.2 -lvtkIONetCDF-6.2 -lvtkIOPLY-6.2 -lvtkIOParallel-6.2 -lvtkIOParallelXML-6.2 -lvtkIOSQL-6.2 -lvtkIOVideo-6.2 -lvtkIOXML-6.2 -lvtkIOXMLParser-6.2 -lvtkImagingColor-6.2 -lvtkImagingCore-6.2 -lvtkImagingFourier-6.2 -lvtkImagingGeneral-6.2 -lvtkImagingHybrid-6.2 -lvtkImagingMath-6.2 -lvtkImagingMorphological-6.2 -lvtkImagingSources-6.2 -lvtkImagingStatistics-6.2 -lvtkImagingStencil-6.2 -lvtkInfovisCore-6.2 -lvtkInfovisLayout-6.2 -lvtkInteractionImage-6.2 -lvtkInteractionStyle-6.2 -lvtkInteractionWidgets-6.2 -lvtkNetCDF-6.2 -lvtkNetCDF_cxx-6.2 -lvtkParallelCore-6.2 -lvtkRenderingAnnotation-6.2 -lvtkRenderingContext2D-6.2 -lvtkRenderingContextOpenGL-6.2 -lvtkRenderingCore-6.2 -lvtkRenderingFreeType-6.2 -lvtkRenderingFreeTypeFontConfig-6.2 -lvtkRenderingFreeTypeOpenGL-6.2 -lvtkRenderingGL2PS-6.2 -lvtkRenderingImage-6.2 -lvtkRenderingLIC-6.2 -lvtkRenderingLOD-6.2 -lvtkRenderingLabel-6.2 -lvtkRenderingOpenGL-6.2 -lvtkRenderingQt-6.2 -lvtkRenderingVolume-6.2 -lvtkRenderingVolumeOpenGL-6.2 -lvtkViewsContext2D-6.2 -lvtkViewsCore-6.2 -lvtkViewsInfovis-6.2 -lvtkViewsQt-6.2 -lvtkalglib-6.2 -lvtkexoIIc-6.2 -lvtkexpat-6.2 -lvtkfreetype-6.2 -lvtkftgl-6.2 -lvtkgl2ps-6.2 -lvtkhdf5-6.2 -lvtkhdf5_hl-6.2 -lvtkjpeg-6.2 -lvtkjsoncpp-6.2 -lvtklibxml2-6.2 -lvtkmetaio-6.2 -lvtkoggtheora-6.2 -lvtkpng-6.2 -lvtkproj4-6.2 -lvtksqlite-6.2 -lvtksys-6.2 -lvtktiff-6.2 -lvtkverdict-6.2 -lvtkzlib-6.2
#LIBS += -L/Users/fxbio6600/OACT/develop/VisIVODesktop6/trunk/lib/ -lsamp -lVOApps -lVO -lVOTable -lVOClient
#LIBS += -lm -lc  -lpthread -lcurl
#LIBS += -L/opt/hdf5-1.10.0-patch1/lib -lhdf5
#LIBS += /usr/lib/libc++.dylib

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



SOURCES += src/main.cpp\
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
 #   src/vosamp.cpp \
    src/visivoimporterdesktop.cpp \
    src/vstabledesktop.cpp \
    src/visivoutilsdesktop.cpp \
    src/vsobjectdesktop.cpp \
    src/filtercustomize.cpp \
    src/vialacteastringdictwidget.cpp


HEADERS  += src/mainwindow.h \
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
   # src/vosamp.h \
    src/visivoimporterdesktop.h \
    src/vstabledesktop.h \
    src/visivoutilsdesktop.h \
    src/vsobjectdesktop.h \
    src/visivofilterdesktop.h \
    src/filtercustomize.h \
    src/vialacteastringdictwidget.h


FORMS    += ui/mainwindow.ui \
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
