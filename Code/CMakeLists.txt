cmake_minimum_required(VERSION 3.16)

project(ViaLacteaVisualAnalytics VERSION 1.6.1 LANGUAGES CXX C)

message(STATUS "Configuring ${PROJECT_NAME} ${PROJECT_VERSION}")

# Set a default build type if none was specified
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
        STRING "Choose the type of build." FORCE
    )
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
    )
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(qt_components Widgets Concurrent Xml Network NetworkAuth WebEngineWidgets PrintSupport)

find_package(Qt5 5.15 REQUIRED QUIET COMPONENTS ${qt_components})
message(STATUS "Qt ${Qt5_VERSION} - ${qt_components}")

find_package(VTK 9.1.0 REQUIRED QUIET COMPONENTS
    CommonColor
    FiltersExtraction
    FiltersGeometry
    FiltersModeling
    GUISupportQt
    InteractionImage
    InteractionStyle
    IOImage
    IOXML
    RenderingAnnotation
    RenderingImage
    RenderingLabel
    RenderingLOD
    RenderingVolumeOpenGL2
)
message(STATUS "VTK ${VTK_VERSION} - ${VTK_LIBRARIES}")

find_package(CFITSIO 4.1.0 QUIET REQUIRED)
message(STATUS "CFITSIO ${CFITSIO_VERSION}")

find_package(Boost 1.78.0 QUIET REQUIRED)
message(STATUS "BOOST ${Boost_VERSION}")

include(FetchContent)
FetchContent_Declare(pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.10.4
)
FetchContent_MakeAvailable(pybind11)

configure_file(src/version.h.in version.h)

set(PROJECT_SOURCES
    src/aboutform.cpp
    src/aboutform.h
    src/astroutils.cpp
    src/astroutils.h
    src/authkeys.h
    src/authwrapper.cpp
    src/authwrapper.h
    src/caesarwindow.cpp
    src/caesarwindow.h
    src/catalogue.cpp
    src/catalogue.h
    src/color.cpp
    src/color.h
    src/dbquery.cpp
    src/dbquery.h
    src/downloadmanager.cpp
    src/downloadmanager.h
    src/ds9region/CodeUtils.h
    src/ds9region/Consts.h
    src/ds9region/DS9Region.cpp
    src/ds9region/DS9Region.h
    src/ds9region/DS9RegionParser.cpp
    src/ds9region/DS9RegionParser.h
    src/extendedglyph3d.cpp
    src/extendedglyph3d.h
    src/fitsheadermodifierdialog.cpp
    src/fitsheadermodifierdialog.h
    src/fitsimagestatisiticinfo.cpp
    src/fitsimagestatisiticinfo.h
    src/higalselectedsources.cpp
    src/higalselectedsources.h
    src/interactors/vtkinteractorstyledrawarrow.h
    src/interactors/vtkinteractorstyledrawarrow.cpp
    src/interactors/vtkinteractorstyleimagecustom.h
    src/interactors/vtkinteractorstyleimagecustom.cpp
    src/interactors/vtkinteractorstyleimagecustom.h
    src/interactors/vtkinteractorstyleprofile.cpp
    src/interactors/vtkinteractorstyleprofile.h
    src/loadingwidget.cpp
    src/loadingwidget.h
    src/lutcustomize.cpp
    src/lutcustomize.h
    src/luteditor.cpp
    src/luteditor.h
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mcutoutsummary.cpp
    src/mcutoutsummary.h
    src/observedobject.cpp
    src/observedobject.h
    src/operationqueue.cpp
    src/operationqueue.h
    src/pipe.cpp
    src/pipe.h
    src/plotwindow.cpp
    src/plotwindow.h
    src/pointspipe.cpp
    src/pointspipe.h
    src/profilewindow.cpp
    src/profilewindow.h
    src/qcustomplot.cpp
    src/qcustomplot.h
    src/sed.cpp
    src/sed.h
    src/sedfitgrid_thick.cpp
    src/sedfitgrid_thick.h
    src/sedfitgrid_thin.cpp
    src/sedfitgrid_thin.h
    src/sednode.cpp
    src/sednode.h
    src/sedplotpointcustom.cpp
    src/sedplotpointcustom.h
    src/sedvisualizerplot.cpp
    src/sedvisualizerplot.h
    src/selectedsourcefieldsselect.cpp
    src/selectedsourcefieldsselect.h
    src/sessionloader.cpp
    src/sessionloader.h
    src/settingform.cpp
    src/settingform.h
    src/sfilterdialog.cpp
    src/sfilterdialog.h
    src/simcollapsedialog.cpp
    src/simcollapsedialog.h
    src/simcube/imresize.cpp
    src/simcube/imresize.h
    src/simcube/simcube_projection-collapse/api-collapse.cpp
    src/simcube/simcube_projection-collapse/fits_simcube.cpp
    src/simcube/simcube_projection-collapse/fits_simcube.hpp
    src/simcube/simcube_projection-collapse/image.cpp
    src/simcube/simcube_projection-collapse/image.hpp
    src/simcube/simcube_projection-collapse/projection.cpp
    src/simcube/simcube_projection-collapse/projection.hpp
    src/simcube/simcube_projection-collapse/utils.cpp
    src/simcube/simcube_projection-collapse/utils.hpp
    src/simcube/simcube_projection-galactic/api-galactic.cpp
    src/simcube/simcube_projection-galactic/fits_simimg.cpp
    src/simcube/simcube_projection-galactic/fits_simimg.hpp
    src/simcube/simcube_projection.hpp
    src/simpleconesearchform.cpp
    src/simpleconesearchform.h
    src/singleton.h
    src/source.cpp
    src/source.h
    src/sourcewidget.cpp
    src/sourcewidget.h
    src/treeitem.cpp
    src/treeitem.h
    src/treemodel.cpp
    src/treemodel.h
    src/usertablewindow.cpp
    src/usertablewindow.h
    src/vialactea.cpp
    src/vialactea.h
    src/vialactea_fileload.cpp
    src/vialactea_fileload.h
    src/vialacteainitialquery.cpp
    src/vialacteainitialquery.h
    src/vialacteasource.cpp
    src/vialacteasource.h
    src/vialacteastringdictwidget.cpp
    src/vialacteastringdictwidget.h
    src/viewselectedsourcedataset.cpp
    src/viewselectedsourcedataset.h
    src/visivoimporterdesktop.cpp
    src/visivoimporterdesktop.h
    src/visivoutilsdesktop.cpp
    src/visivoutilsdesktop.h
    src/vispoint.cpp
    src/vispoint.h
    src/vlkbquery.cpp
    src/vlkbquery.h
    src/vlkbsimplequerycomposer.cpp
    src/vlkbsimplequerycomposer.h
    src/vlvaurlschemehandler.cpp
    src/vlvaurlschemehandler.h
    src/vsobjectdesktop.cpp
    src/vsobjectdesktop.h
    src/vstabledesktop.cpp
    src/vstabledesktop.h
    src/vtkellipse.cpp
    src/vtkellipse.h
    src/vtkextracthistogram.cpp
    src/vtkextracthistogram.h
    src/vtkfitsreader.cpp
    src/vtkfitsreader.h
    src/vtkfitsreader2.cpp
    src/vtkfitsreader2.h
    src/vtkfitstoolswidget.cpp
    src/vtkfitstoolswidget.h
    src/vtkfitstoolwidgetobject.cpp
    src/vtkfitstoolwidgetobject.h
    src/vtklegendscaleactorwcs.cpp
    src/vtklegendscaleactorwcs.h
    src/vtktoolswidget.cpp
    src/vtktoolswidget.h
    src/vtkwindow_new.cpp
    src/vtkwindow_new.h
    src/vtkwindowcube.cpp
    src/vtkwindowcube.h
    src/vtkwindowpv.cpp
    src/vtkwindowpv.h
    src/xmlparser.cpp
    src/xmlparser.h
)

set(CMAKE_AUTOUIC_SEARCH_PATHS ui)
set(PROJECT_UI
    ui/aboutform.ui
    ui/caesarwindow.ui
    ui/dbquery.ui
    ui/fitsheadermodifierdialog.ui
    ui/fitsimagestatisiticinfo.ui
    ui/higalselectedsources.ui
    ui/loadingwidget.ui
    ui/lutcustomize.ui
    ui/mainwindow.ui
    ui/mcutoutsummary.ui
    ui/operationqueue.ui
    ui/plotwindow.ui
    ui/profilewindow.ui
    ui/sedfitgrid_thick.ui
    ui/sedfitgrid_thin.ui
    ui/sedvisualizerplot.ui
    ui/selectedsourcefieldsselect.ui
    ui/sessionloader.ui
    ui/settingform.ui
    ui/sfilterdialog.ui
    ui/simcollapsedialog.ui
    ui/simpleconesearchform.ui
    ui/sourcewidget.ui
    ui/usertablewindow.ui
    ui/vialactea.ui
    ui/vialactea_fileload.ui
    ui/vialacteainitialquery.ui
    ui/vialacteastringdictwidget.ui
    ui/viewselectedsourcedataset.ui
    ui/vlkbquery.ui
    ui/vlkbsimplequerycomposer.ui
    ui/vtkfitstoolswidget.ui
    ui/vtktoolswidget.ui
    ui/vtkwindow_new.ui
    ui/vtkwindowcube.ui
    ui/vtkwindowpv.ui
)

qt_add_resources(PROJECT_RESOURCES visivo.qrc)

if(WIN32)
    set(ICON_RESOURCE visivo.rc)
elseif(APPLE)
    set(ICON_RESOURCE logo.icns)
    set(MACOSX_BUNDLE_ICON_FILE logo.icns)
    set(app_icon_macos "${CMAKE_SOURCE_DIR}/logo.icns")
    set_source_files_properties(${app_icon_macos} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
endif()

add_executable(ViaLacteaVisualAnalytics
    ${PROJECT_SOURCES}
    ${PROJECT_UI}
    ${PROJECT_RESOURCES}
    ${ICON_RESOURCE}
)

add_subdirectory(src/libwcs)

target_compile_definitions(ViaLacteaVisualAnalytics PRIVATE $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>)

foreach(_qt_comp IN LISTS qt_components)
    list(APPEND qt_modules "Qt5::${_qt_comp}")
endforeach()

target_link_libraries(ViaLacteaVisualAnalytics PRIVATE
    ${qt_modules}
    ${VTK_LIBRARIES}
    CFITSIO::CFITSIO
    Boost::headers
    wcs
    pybind11::embed
)

set_target_properties(ViaLacteaVisualAnalytics PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER it.inaf.oact.visivo
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
    MACOSX_BUNDLE ON
    WIN32_EXECUTABLE ON
)

vtk_module_autoinit(
    TARGETS ViaLacteaVisualAnalytics
    MODULES ${VTK_LIBRARIES}
)
