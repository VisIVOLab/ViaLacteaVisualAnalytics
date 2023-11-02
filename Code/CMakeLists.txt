cmake_minimum_required(VERSION 3.8)

project(ViaLacteaVisualAnalytics LANGUAGES CXX C VERSION 1.6.0)

# Set a default build type if none was specified
set(default_build_type "Release")
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
            STRING "Choose the type of build." FORCE
            )
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
            )
endif ()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(ParaView REQUIRED)
find_package(CFITSIO REQUIRED)
find_package(Boost REQUIRED)

set(qt_components Widgets Network NetworkAuth WebEngineWidgets Concurrent Xml PrintSupport)
find_package(Qt5 QUIET REQUIRED COMPONENTS ${qt_components})

foreach (_qt_comp IN LISTS qt_components)
    list(APPEND qt_modules "Qt5::${_qt_comp}")
endforeach ()

if (NOT PARAVIEW_USE_QT)
    message(FATAL_ERROR
            "${CMAKE_PROJECT_NAME} requires PARAVIEW_USE_QT "
            "to be enabled. Please rebuild ParaView (or point to a different build of "
            "ParaView) with PARAVIEW_USE_QT set to TRUE")
endif ()

include(GNUInstallDirs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ui)

set(sources
        src/aboutform.cpp
        src/astroutils.cpp
        src/authwrapper.cpp
        src/base64.cpp
        src/caesarwindow.cpp
        src/color.cpp
        src/contour.cpp
        src/dbquery.cpp
        src/downloadmanager.cpp
        src/ds9region/DS9Region.cpp
        src/ds9region/DS9RegionParser.cpp
        src/extendedglyph3d.cpp
        src/filtercustomize.cpp
        src/fitsimagestatisiticinfo.cpp
        src/higalselectedsources.cpp
        src/libwcs/cel.c
        src/libwcs/dateutil.c
        src/libwcs/distort.c
        src/libwcs/dsspos.c
        src/libwcs/fileutil.c
        src/libwcs/fitsfile.c
        src/libwcs/fitswcs.c
        src/libwcs/hget.c
        src/libwcs/hput.c
        src/libwcs/iget.c
        src/libwcs/imhfile.c
        src/libwcs/imio.c
        src/libwcs/lin.c
        src/libwcs/platepos.c
        src/libwcs/proj.c
        src/libwcs/sph.c
        src/libwcs/tnxpos.c
        src/libwcs/wcs.c
        src/libwcs/wcscon.c
        src/libwcs/wcsinit.c
        src/libwcs/wcslib.c
        src/libwcs/wcstrig.c
        src/libwcs/worldpos.c
        src/loadingwidget.cpp
        src/qcustomplot.cpp
        src/lutcustomize.cpp
        src/luteditor.cpp
        src/main.cpp
        src/mainwindow.cpp
        src/mcutoutsummary.cpp
        src/observedobject.cpp
        src/operationqueue.cpp
        src/pipe.cpp
        src/plotwindow.cpp
        src/pointspipe.cpp
        src/pqmeminspector.cpp
        src/pqwindowcube.cpp
        src/pqwindowimage.cpp
        src/sed.cpp
        src/sedfitgrid_thick.cpp
        src/sedfitgrid_thin.cpp
        src/sednode.cpp
        src/sedplotpointcustom.cpp
        src/sedvisualizerplot.cpp
        src/selectedsourcefieldsselect.cpp
        src/sessionloader.cpp
        src/settingform.cpp
        src/subsetselectordialog.cpp
        src/treeitem.cpp
        src/treemodel.cpp
        src/usertablewindow.cpp
        src/vialactea.cpp
        src/vialactea_fileload.cpp
        src/vialacteainitialquery.cpp
        src/vialacteasource.cpp
        src/vialacteastringdictwidget.cpp
        src/viewselectedsourcedataset.cpp
        src/visivoimporterdesktop.cpp
        src/visivoutilsdesktop.cpp
        src/vispoint.cpp
        src/vlkbquery.cpp
        src/vlkbsimplequerycomposer.cpp
        src/vlkbtable.cpp
        src/vlvastackimage.cpp
        src/vlvaurlschemehandler.cpp
        src/vsobjectdesktop.cpp
        src/vstabledesktop.cpp
        src/vtkellipse.cpp
        src/vtkextracthistogram.cpp
        src/vtkfitsreader.cpp
        src/vtkfitstoolswidget.cpp
        src/vtkfitstoolwidget_new.cpp
        src/vtkfitstoolwidgetobject.cpp
        src/vtklegendscaleactor.cpp
        src/vtktoolswidget.cpp
        src/vtkwindow_new.cpp
        src/xmlparser.cpp
        src/interactors/vtkinteractorstyleimagecustom.cpp
        )

set(headers
        src/aboutform.h
        src/astroutils.h
        src/authkeys.h
        src/authwrapper.h
        src/base64.h
        src/caesarwindow.h
        src/color.h
        src/contour.h
        src/dbquery.h
        src/downloadmanager.h
        src/ds9region/CodeUtils.h
        src/ds9region/Consts.h
        src/ds9region/DS9Region.h
        src/ds9region/DS9RegionParser.h
        src/errorMessage.h
        src/extendedglyph3d.h
        src/filtercustomize.h
        src/fitsimagestatisiticinfo.h
        src/higalselectedsources.h
        src/libwcs/fitsfile.h
        src/libwcs/wcs.h
        src/loadingwidget.h
        src/qcustomplot.h
        src/lutcustomize.h
        src/luteditor.h
        src/mainwindow.h
        src/mcutoutsummary.h
        src/observedobject.h
        src/operationqueue.h
        src/osxhelper.h
        src/pipe.h
        src/plotwindow.h
        src/pointspipe.h
        src/pqmeminspector.h
        src/pqwindowcube.h
        src/pqwindowimage.h
        src/sed.h
        src/sedfitgrid_thick.h
        src/sedfitgrid_thin.h
        src/sednode.h
        src/sedplotpointcustom.h
        src/sedvisualizerplot.h
        src/selectedsourcefieldsselect.h
        src/sessionloader.h
        src/settingform.h
        src/singleton.h
        src/subsetselectordialog.h
        src/treeitem.h
        src/treemodel.h
        src/usertablewindow.h
        src/vialactea.h
        src/vialactea_fileload.h
        src/vialacteainitialquery.h
        src/vialacteasource.h
        src/vialacteastringdictwidget.h
        src/viewselectedsourcedataset.h
        src/visivoimporterdesktop.h
        src/visivoutilsdesktop.h
        src/vispoint.h
        src/vlkbquery.h
        src/vlkbsimplequerycomposer.h
        src/vlkbtable.h
        src/vlvastackimage.h
        src/vlvaurlschemehandler.h
        src/vsobjectdesktop.h
        src/vstabledesktop.h
        src/vtkellipse.h
        src/vtkextracthistogram.h
        src/vtkfitsreader.h
        src/vtkfitstoolswidget.h
        src/vtkfitstoolwidget_new.h
        src/vtkfitstoolwidgetobject.h
        src/vtklegendscaleactor.h
        src/vtktoolswidget.h
        src/vtkwindow_new.h
        src/xmlparser.h
        src/interactors/vtkinteractorstyleimagecustom.h
        )

set(ui_files
        ui/aboutform.ui
        ui/caesarwindow.ui
        ui/contour.ui
        ui/dbquery.ui
        ui/filtercustomize.ui
        ui/fitsimagestatisiticinfo.ui
        ui/higalselectedsources.ui
        ui/loadingwidget.ui
        ui/mainwindow.ui
        ui/mcutoutsummary.ui
        ui/operationqueue.ui
        ui/plotwindow.ui
        ui/pqmeminspector.ui
        ui/pqwindowcube.ui
        ui/pqwindowimage.ui
        ui/sedfitgrid_thick.ui
        ui/sedfitgrid_thin.ui
        ui/sedvisualizerplot.ui
        ui/selectedsourcefieldsselect.ui
        ui/sessionloader.ui
        ui/settingform.ui
        ui/subsetselectordialog.ui
        ui/usertablewindow.ui
        ui/vialactea.ui
        ui/vialactea_fileload.ui
        ui/vialacteainitialquery.ui
        ui/vialacteastringdictwidget.ui
        ui/viewselectedsourcedataset.ui
        ui/lutcustomize.ui
        ui/vlkbquery.ui
        ui/vlkbsimplequerycomposer.ui
        ui/vtkfitstoolswidget.ui
        ui/vtkfitstoolwidget_new.ui
        ui/vtktoolswidget.ui
        ui/vtkwindow_new.ui
        )

set(ICON_RESOURCE icons/logo_256_beta.icns)
set(MACOSX_BUNDLE_ICON_FILE logo_256_beta.icns)
set(app_icon_macos "${CMAKE_SOURCE_DIR}/icons/logo_256_beta.icns")
set_source_files_properties(${app_icon_macos} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
)

set(resources visivo.qrc ${ICON_RESOURCE})

add_executable(ViaLacteaVisualAnalytics ${ui_files} ${resources} ${sources} ${headers})

target_include_directories(ViaLacteaVisualAnalytics PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(ViaLacteaVisualAnalytics PRIVATE
        ${qt_modules}
        ${VTK_LIBRARIES}
        ParaView::pqApplicationComponents
        ParaView::RemotingViews
        CFITSIO::CFITSIO
)

set_target_properties(ViaLacteaVisualAnalytics PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER it.inaf.oact.visivo
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
    MACOSX_BUNDLE ON
)
