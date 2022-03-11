#-------------------------------------------------
#
# Project created by QtCreator 2015-05-18T11:03:07
#
#-------------------------------------------------

QT += core gui widgets network networkauth printsupport xml concurrent webenginewidgets
CONFIG += c++11

#QMAKE_MAC_SDK = macosx10.15
CONFIG += sdk_no_version_check

CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
}

TARGET = ViaLacteaVisualAnalytics
TEMPLATE = app

ICON = logo.icns

INCLUDEPATH += /usr/local/include
LIBS += -L/usr/local/lib

exists(includes.pri) {
    include(includes.pri)
}

LIBS += -lcfitsio
LIBS += \
    -lvtkCommonColor-9.0 \
    -lvtkCommonComputationalGeometry-9.0 \
    -lvtkCommonCore-9.0 \
    -lvtkCommonDataModel-9.0 \
    -lvtkCommonExecutionModel-9.0 \
    -lvtkCommonMath-9.0 \
    -lvtkCommonMisc-9.0 \
    -lvtkCommonSystem-9.0 \
    -lvtkCommonTransforms-9.0 \
    -lvtkFiltersCore-9.0 \
    -lvtkFiltersExtraction-9.0 \
    -lvtkFiltersGeneral-9.0 \
    -lvtkFiltersGeometry-9.0 \
    -lvtkFiltersHybrid-9.0 \
    -lvtkFiltersModeling-9.0 \
    -lvtkFiltersSources-9.0 \
    -lvtkFiltersStatistics-9.0 \
    -lvtkGUISupportQt-9.0 \
    -lvtkIOCore-9.0 \
    -lvtkIOImage-9.0 \
    -lvtkIOLegacy-9.0 \
    -lvtkIOXML-9.0 \
    -lvtkIOXMLParser-9.0 \
    -lvtkImagingColor-9.0 \
    -lvtkImagingCore-9.0 \
    -lvtkImagingFourier-9.0 \
    -lvtkImagingGeneral-9.0 \
    -lvtkImagingMath-9.0 \
    -lvtkImagingSources-9.0 \
    -lvtkImagingStencil-9.0 \
    -lvtkInteractionImage-9.0 \
    -lvtkInteractionStyle-9.0 \
    -lvtkInteractionWidgets-9.0 \
    -lvtkParallelCore-9.0 \
    -lvtkParallelDIY-9.0 \
    -lvtkRenderingAnnotation-9.0 \
    -lvtkRenderingCore-9.0 \
    -lvtkRenderingFreeType-9.0 \
    -lvtkRenderingImage-9.0 \
    -lvtkRenderingLOD-9.0 \
    -lvtkRenderingLabel-9.0 \
    -lvtkRenderingOpenGL2-9.0 \
    -lvtkRenderingQt-9.0 \
    -lvtkRenderingUI-9.0 \
    -lvtkRenderingVolume-9.0 \
    -lvtkRenderingVolumeOpenGL2-9.0 \
    -lvtkdoubleconversion-9.0 \
    -lvtkexpat-9.0 \
    -lvtkfreetype-9.0 \
    -lvtkglew-9.0 \
    -lvtkloguru-9.0 \
    -lvtklz4-9.0 \
    -lvtklzma-9.0 \
    -lvtksys-9.0 \
    -lvtkzlib-9.0


SOURCES += \
    src/aboutform.cpp \
    src/astroutils.cpp \
    src/authwrapper.cpp \
    src/base64.cpp \
    src/caesarwindow.cpp \
    src/color.cpp \
    src/contour.cpp \
    src/dbquery.cpp \
    src/downloadmanager.cpp \
    src/extendedglyph3d.cpp \
    src/filtercustomize.cpp \
    src/fitsimagestatisiticinfo.cpp \
    src/higalselectedsources.cpp \
    src/histogram.cpp \
    src/interactors/vtkinteractorstyleimagecustom.cpp \
    src/libwcs/cel.c \
    src/libwcs/dateutil.c \
    src/libwcs/distort.c \
    src/libwcs/dsspos.c \
    src/libwcs/fileutil.c \
    src/libwcs/fitsfile.c \
    src/libwcs/fitswcs.c \
    src/libwcs/hget.c \
    src/libwcs/hput.c \
    src/libwcs/iget.c \
    src/libwcs/imhfile.c \
    src/libwcs/imio.c \
    src/libwcs/lin.c \
    src/libwcs/platepos.c \
    src/libwcs/proj.c \
    src/libwcs/sph.c \
    src/libwcs/tnxpos.c \
    src/libwcs/wcs.c \
    src/libwcs/wcscon.c \
    src/libwcs/wcsinit.c \
    src/libwcs/wcslib.c \
    src/libwcs/wcstrig.c \
    src/libwcs/worldpos.c \
    src/loadingwidget.cpp \
    src/lutcustomize.cpp \
    src/luteditor.cpp \
    src/lutselector.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/observedobject.cpp \
    src/operationqueue.cpp \
    src/pipe.cpp \
    src/plotwindow.cpp \
    src/pointspipe.cpp \
    src/qcustomplot.cpp \
    src/sed.cpp \
    src/sedfitgrid_thick.cpp \
    src/sedfitgrid_thin.cpp \
    src/sednode.cpp \
    src/sedplotpointcustom.cpp \
    src/sedvisualizerplot.cpp \
    src/selectedsourcefieldsselect.cpp \
    src/selectedsourcesform.cpp \
    src/settingform.cpp \
    src/treeitem.cpp \
    src/treemodel.cpp \
    src/vialactea.cpp \
    src/vialactea_fileload.cpp \
    src/vialacteainitialquery.cpp \
    src/vialacteasource.cpp \
    src/vialacteastringdictwidget.cpp \
    src/viewselectedsourcedataset.cpp \
    src/visivoimporterdesktop.cpp \
    src/visivoutilsdesktop.cpp \
    src/vispoint.cpp \
    src/vlkbquery.cpp \
    src/vlkbquerycomposer.cpp \
    src/vlkbsimplequerycomposer.cpp \
    src/vlkbtable.cpp \
    src/vlvaurlschemehandler.cpp \
    src/vsobjectdesktop.cpp \
    src/vstabledesktop.cpp \
    src/vtkellipse.cpp \
    src/vtkextracthistogram.cpp \
    src/vtkfitsreader.cpp \
    src/vtkfitstoolswidget.cpp \
    src/vtkfitstoolwidget_new.cpp \
    src/vtkfitstoolwidgetobject.cpp \
    src/vtklegendscaleactor.cpp \
    src/vtktoolswidget.cpp \
    src/vtkwindow_new.cpp \
    src/vtkwindowcube.cpp \
    src/vtkwindowimage.cpp \
    src/vtkwindowsources.cpp \
    src/xmlparser.cpp


HEADERS  += \
    src/aboutform.h \
    src/astroutils.h \
    src/authkeys.h \
    src/authwrapper.h \
    src/base64.h \
    src/caesarwindow.h \
    src/color.h \
    src/contour.h \
    src/dbquery.h \
    src/downloadmanager.h \
    src/extendedglyph3d.h \
    src/filtercustomize.h \
    src/fitsimagestatisiticinfo.h \
    src/higalselectedsources.h \
    src/histogram.h \
    src/interactors/vtkinteractorstyleimagecustom.h \
    src/libwcs/fitsfile.h \
    src/libwcs/wcs.h \
    src/loadingwidget.h \
    src/lutcustomize.h \
    src/luteditor.h \
    src/lutselector.h \
    src/mainwindow.h \
    src/observedobject.h \
    src/operationqueue.h \
    src/osxhelper.h \
    src/pipe.h \
    src/plotwindow.h \
    src/pointspipe.h \
    src/qcustomplot.h \
    src/sed.h \
    src/sedfitgrid_thick.h \
    src/sedfitgrid_thin.h \
    src/sednode.h \
    src/sedplotpointcustom.h \
    src/sedvisualizerplot.h \
    src/selectedsourcefieldsselect.h \
    src/selectedsourcesform.h \
    src/settingform.h \
    src/singleton.h \
    src/treeitem.h \
    src/treemodel.h \
    src/vialactea.h \
    src/vialactea_fileload.h \
    src/vialacteainitialquery.h \
    src/vialacteasource.h \
    src/vialacteastringdictwidget.h \
    src/viewselectedsourcedataset.h \
    src/visivoimporterdesktop.h \
    src/visivoutilsdesktop.h \
    src/vispoint.h \
    src/vlkbquery.h \
    src/vlkbquerycomposer.h \
    src/vlkbsimplequerycomposer.h \
    src/vlkbtable.h \
    src/vlvaurlschemehandler.h \
    src/vsobjectdesktop.h \
    src/vstabledesktop.h \
    src/vtkellipse.h \
    src/vtkextracthistogram.h \
    src/vtkfitsreader.h \
    src/vtkfitstoolswidget.h \
    src/vtkfitstoolwidget_new.h \
    src/vtkfitstoolwidgetobject.h \
    src/vtklegendscaleactor.h \
    src/vtktoolswidget.h \
    src/vtkwindow_new.h \
    src/vtkwindowcube.h \
    src/vtkwindowimage.h \
    src/vtkwindowsources.h \
    src/xmlparser.h


FORMS += \
    ui/aboutform.ui \
    ui/caesarwindow.ui \
    ui/contour.ui \
    ui/dbquery.ui \
    ui/filtercustomize.ui \
    ui/fitsimagestatisiticinfo.ui \
    ui/higalselectedsources.ui \
    ui/loadingwidget.ui \
    ui/lutcustomize.ui \
    ui/mainwindow.ui \
    ui/operationqueue.ui \
    ui/plotwindow.ui \
    ui/sedfitgrid_thick.ui \
    ui/sedfitgrid_thin.ui \
    ui/sedvisualizerplot.ui \
    ui/selectedsourcefieldsselect.ui \
    ui/selectedsourcesform.ui \
    ui/settingform.ui \
    ui/vialactea.ui \
    ui/vialactea_fileload.ui \
    ui/vialacteainitialquery.ui \
    ui/vialacteastringdictwidget.ui \
    ui/viewselectedsourcedataset.ui \
    ui/vlkbquery.ui \
    ui/vlkbquerycomposer.ui \
    ui/vlkbsimplequerycomposer.ui \
    ui/vtkfitstoolswidget.ui \
    ui/vtkfitstoolwidget_new.ui \
    ui/vtktoolswidget.ui \
    ui/vtkwindow_new.ui \
    ui/vtkwindowcube.ui \
    ui/vtkwindowimage.ui \
    ui/vtkwindowsources.ui


RESOURCES += \
    visivo.qrc


OBJECTIVE_SOURCES += \
    src/osxhelper.mm
