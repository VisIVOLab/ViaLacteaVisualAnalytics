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
    -lvtkCommonColor-9.1 \
    -lvtkCommonComputationalGeometry-9.1 \
    -lvtkCommonCore-9.1 \
    -lvtkCommonDataModel-9.1 \
    -lvtkCommonExecutionModel-9.1 \
    -lvtkCommonMath-9.1 \
    -lvtkCommonMisc-9.1 \
    -lvtkCommonSystem-9.1 \
    -lvtkCommonTransforms-9.1 \
    -lvtkDICOMParser-9.1 \
    -lvtkFiltersCore-9.1 \
    -lvtkFiltersExtraction-9.1 \
    -lvtkFiltersGeneral-9.1 \
    -lvtkFiltersGeometry-9.1 \
    -lvtkFiltersHybrid-9.1 \
    -lvtkFiltersModeling-9.1 \
    -lvtkFiltersSources-9.1 \
    -lvtkFiltersStatistics-9.1 \
    -lvtkFiltersTexture-9.1 \
    -lvtkGUISupportQt-9.1 \
    -lvtkIOCore-9.1 \
    -lvtkIOImage-9.1 \
    -lvtkIOLegacy-9.1 \
    -lvtkIOXML-9.1 \
    -lvtkIOXMLParser-9.1 \
    -lvtkImagingColor-9.1 \
    -lvtkImagingCore-9.1 \
    -lvtkImagingFourier-9.1 \
    -lvtkImagingGeneral-9.1 \
    -lvtkImagingHybrid-9.1 \
    -lvtkImagingMath-9.1 \
    -lvtkImagingSources-9.1 \
    -lvtkImagingStencil-9.1 \
    -lvtkInteractionImage-9.1 \
    -lvtkInteractionStyle-9.1 \
    -lvtkInteractionWidgets-9.1 \
    -lvtkParallelCore-9.1 \
    -lvtkParallelDIY-9.1 \
    -lvtkRenderingAnnotation-9.1 \
    -lvtkRenderingContext2D-9.1 \
    -lvtkRenderingCore-9.1 \
    -lvtkRenderingFreeType-9.1 \
    -lvtkRenderingImage-9.1 \
    -lvtkRenderingLOD-9.1 \
    -lvtkRenderingLabel-9.1 \
    -lvtkRenderingOpenGL2-9.1 \
    -lvtkRenderingQt-9.1 \
    -lvtkRenderingUI-9.1 \
    -lvtkRenderingVolume-9.1 \
    -lvtkRenderingVolumeOpenGL2-9.1 \
    -lvtkdoubleconversion-9.1 \
    -lvtkexpat-9.1 \
    -lvtkfmt-9.1 \
    -lvtkfreetype-9.1 \
    -lvtkglew-9.1 \
    -lvtkjpeg-9.1 \
    -lvtkkissfft-9.1 \
    -lvtkloguru-9.1 \
    -lvtklz4-9.1 \
    -lvtklzma-9.1 \
    -lvtkmetaio-9.1 \
    -lvtkpng-9.1 \
    -lvtkpugixml-9.1 \
    -lvtksys-9.1 \
    -lvtktiff-9.1 \
    -lvtkzlib-9.1


SOURCES += \
    src/aboutform.cpp \
    src/astroutils.cpp \
    src/authwrapper.cpp \
    src/base64.cpp \
    src/caesarwindow.cpp \
    src/catalogue.cpp \
    src/color.cpp \
    src/contour.cpp \
    src/dbquery.cpp \
    src/downloadmanager.cpp \
    src/ds9region/DS9Region.cpp \
    src/ds9region/DS9RegionParser.cpp \
    src/extendedglyph3d.cpp \
    src/filtercustomize.cpp \
    src/fitsheadermodifierdialog.cpp \
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
    src/libwcs/imutil.c \
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
    src/mcutoutsummary.cpp \
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
    src/sessionloader.cpp \
    src/settingform.cpp \
    src/sfilterdialog.cpp \
    src/source.cpp \
    src/sourcewidget.cpp \
    src/simcollapsedialog.cpp \
    src/simcube/imresize.cpp \
    src/simcube/simcube_projection-collapse/api-collapse.cpp \
    src/simcube/simcube_projection-collapse/fits_simcube.cpp \
    src/simcube/simcube_projection-collapse/image.cpp \
    src/simcube/simcube_projection-collapse/projection.cpp \
    src/simcube/simcube_projection-collapse/utils.cpp \
    src/simcube/simcube_projection-galactic/api-galactic.cpp \
    src/simcube/simcube_projection-galactic/fits_simimg.cpp \
    src/simplacedialog.cpp \
    src/treeitem.cpp \
    src/treemodel.cpp \
    src/usertablewindow.cpp \
    src/vialactea.cpp \
    src/vialacteainitialquery.cpp \
    src/vialacteasource.cpp \
    src/vialacteastringdictwidget.cpp \
    src/vialactea_fileload.cpp \
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
    src/vtkfitsreader2.cpp \
    src/vtkfitstoolswidget.cpp \
    src/vtkfitstoolwidgetobject.cpp \
    src/vtkfitstoolwidget_new.cpp \
    src/vtkfitswriter.cpp \
    src/vtklegendscaleactorwcs.cpp \
    src/vtktoolswidget.cpp \
    src/vtkwindow_new.cpp \
    src/xmlparser.cpp \
    src/vtkwindowcube.cpp \
    src/profilewindow.cpp


HEADERS  += \
    src/aboutform.h \
    src/astroutils.h \
    src/authkeys.h \
    src/authwrapper.h \
    src/base64.h \
    src/caesarwindow.h \
    src/catalogue.h \
    src/color.h \
    src/contour.h \
    src/dbquery.h \
    src/downloadmanager.h \
    src/ds9region/CodeUtils.h \
    src/ds9region/Consts.h \
    src/ds9region/DS9Region.h \
    src/ds9region/DS9RegionParser.h \
    src/extendedglyph3d.h \
    src/filtercustomize.h \
    src/fitsheadermodifierdialog.h \
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
    src/mcutoutsummary.h \
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
    src/sessionloader.h \
    src/settingform.h \
    src/sfilterdialog.h \
    src/simcollapsedialog.h \
    src/simcube/imresize.h \
    src/simcube/simcube_projection-collapse/fits_simcube.hpp \
    src/simcube/simcube_projection-collapse/image.hpp \
    src/simcube/simcube_projection-collapse/projection.hpp \
    src/simcube/simcube_projection-collapse/utils.hpp \
    src/simcube/simcube_projection-galactic/fits_simimg.hpp \
    src/simcube/simcube_projection.hpp \
    src/simplacedialog.h \
    src/singleton.h \
    src/source.h \
    src/sourcewidget.h \
    src/treeitem.h \
    src/treemodel.h \
    src/usertablewindow.h \
    src/vialactea.h \
    src/vialacteainitialquery.h \
    src/vialacteasource.h \
    src/vialacteastringdictwidget.h \
    src/vialactea_fileload.h \
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
    src/vtkfitsreader2.h \
    src/vtkfitstoolswidget.h \
    src/vtkfitstoolwidgetobject.h \
    src/vtkfitstoolwidget_new.h \
    src/vtkfitswriter.h \
    src/vtklegendscaleactorwcs.h \
    src/vtktoolswidget.h \
    src/vtkwindow_new.h \
    src/xmlparser.h \
    src/vtkwindowcube.h \
    src/profilewindow.h


FORMS += \
    ui/aboutform.ui \
    ui/caesarwindow.ui \
    ui/contour.ui \
    ui/dbquery.ui \
    ui/filtercustomize.ui \
    ui/fitsheadermodifierdialog.ui \
    ui/fitsimagestatisiticinfo.ui \
    ui/higalselectedsources.ui \
    ui/loadingwidget.ui \
    ui/lutcustomize.ui \
    ui/mainwindow.ui \
    ui/mcutoutsummary.ui \
    ui/operationqueue.ui \
    ui/plotwindow.ui \
    ui/sedfitgrid_thick.ui \
    ui/sedfitgrid_thin.ui \
    ui/sedvisualizerplot.ui \
    ui/selectedsourcefieldsselect.ui \
    ui/selectedsourcesform.ui \
    ui/sessionloader.ui \
    ui/settingform.ui \
    ui/sfilterdialog.ui \
    ui/sourcewidget.ui \
    ui/simcollapsedialog.ui \
    ui/simplacedialog.ui \
    ui/usertablewindow.ui \
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
    ui/profilewindow.ui


RESOURCES += \
    visivo.qrc


OBJECTIVE_SOURCES += \
    src/osxhelper.mm
