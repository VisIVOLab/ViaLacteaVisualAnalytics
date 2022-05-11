cmake_minimum_required(VERSION 3.8)
project(ViaLacteaVisualAnalytics)

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

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(ParaView REQUIRED)
find_package(CFITSIO REQUIRED)


if (NOT PARAVIEW_USE_QT)
  message(STATUS
    "Skipping example: ${CMAKE_PROJECT_NAME} example requires PARAVIEW_USE_QT "
    "to be enabled. Please rebuild ParaView (or point to a different build of "
    "ParaView) with PARAVIEW_USE_QT set to TRUE")
  return ()
endif()

include(GNUInstallDirs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_AUTOUIC_SEARCH_PATHS ui)

find_package(Qt5 REQUIRED COMPONENTS                      Charts                    Help                      Purchasing                QuickTemplates2           UiPlugin
3DAnimation               ClipboardSupport                       Qml                       QuickTest                 UiTools
3DCore                    Concurrent                Location                  QmlDebug                  QuickWidgets              VirtualKeyboard
3DExtras                  Core                      MacExtras                                RemoteObjects             WebChannel
3DInput                   DBus                      Multimedia                          RepParser                 WebEngine
3DLogic                   DataVisualization         MultimediaQuick           QmlModels                 Script                    WebEngineCore
3DQuick                   Designer                  MultimediaWidgets         QmlWorkerScript           ScriptTools               WebEngineWidgets
3DQuickAnimation          DesignerComponents        Network                   Quick                     Scxml                     WebSockets
3DQuickExtras             DeviceDiscoverySupport    NetworkAuth               Quick3D                   Sensors                   WebView
3DQuickInput                                Nfc                       Quick3DAssetImport        SerialBus                 Widgets
3DQuickRender             EdidSupport               OpenGL                    Quick3DRender             SerialPort                Xml
3DQuickScene2D            EventDispatcherSupport    OpenGLExtensions          Quick3DRuntimeRender      ServiceSupport            XmlPatterns
3DRender                  FbSupport                 PacketProtocol            Quick3DUtils              Sql
AccessibilitySupport      FontDatabaseSupport       PlatformCompositorSupport              Svg
  Gamepad                   Positioning               QuickControls2            Test
Bluetooth                 GraphicsSupport           PositioningQuick          QuickParticles            TextToSpeech
Bodymovin                 Gui                       PrintSupport              QuickShapes               ThemeSupport)

set(CMAKE_AUTOMOC 1)
set(CMAKE_AUTOUIC 1)

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
    src/histogram.cpp 
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
    src/lutcustomize.cpp 
    src/luteditor.cpp 
    src/lutselector.cpp 
    src/main.cpp 
    src/mainwindow.cpp 
    src/mcutoutsummary.cpp 
    src/observedobject.cpp 
    src/operationqueue.cpp 
    src/pipe.cpp 
    src/plotwindow.cpp 
    src/pointspipe.cpp 
    src/qcustomplot.cpp 
    src/sed.cpp 
    src/sedfitgrid_thick.cpp 
    src/sedfitgrid_thin.cpp 
    src/sednode.cpp 
    src/sedplotpointcustom.cpp 
    src/sedvisualizerplot.cpp 
    src/selectedsourcefieldsselect.cpp 
    src/selectedsourcesform.cpp 
    src/sessionloader.cpp 
    src/settingform.cpp 
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
    src/vlkbquerycomposer.cpp 
    src/vlkbsimplequerycomposer.cpp 
    src/vlkbtable.cpp 
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
)

set (headers 
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
    src/extendedglyph3d.h 
    src/filtercustomize.h 
    src/fitsimagestatisiticinfo.h 
    src/higalselectedsources.h 
    src/histogram.h 
    src/libwcs/fitsfile.h 
    src/libwcs/wcs.h 
    src/loadingwidget.h 
    src/lutcustomize.h 
    src/luteditor.h 
    src/lutselector.h 
    src/mainwindow.h 
    src/mcutoutsummary.h 
    src/observedobject.h 
    src/operationqueue.h 
    src/osxhelper.h 
    src/pipe.h 
    src/plotwindow.h 
    src/pointspipe.h 
    src/qcustomplot.h 
    src/sed.h 
    src/sedfitgrid_thick.h 
    src/sedfitgrid_thin.h 
    src/sednode.h 
    src/sedplotpointcustom.h 
    src/sedvisualizerplot.h 
    src/selectedsourcefieldsselect.h 
    src/selectedsourcesform.h 
    src/sessionloader.h 
    src/settingform.h 
    src/singleton.h 
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
    src/vlkbquerycomposer.h 
    src/vlkbsimplequerycomposer.h 
    src/vlkbtable.h 
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
    src/xmlparser.h)

set(ui_files
    ui/aboutform.ui 
    ui/caesarwindow.ui 
    ui/contour.ui 
    ui/dbquery.ui 
    ui/filtercustomize.ui 
    ui/fitsimagestatisiticinfo.ui 
    ui/higalselectedsources.ui 
    ui/loadingwidget.ui 
    ui/lutcustomize.ui 
    ui/mainwindow.ui 
    ui/mcutoutsummary.ui 
    ui/operationqueue.ui 
    ui/plotwindow.ui 
    ui/sedfitgrid_thick.ui 
    ui/sedfitgrid_thin.ui 
    ui/sedvisualizerplot.ui 
    ui/selectedsourcefieldsselect.ui 
    ui/selectedsourcesform.ui 
    ui/sessionloader.ui 
    ui/settingform.ui 
    ui/usertablewindow.ui 
    ui/vialactea.ui 
    ui/vialactea_fileload.ui 
    ui/vialacteainitialquery.ui 
    ui/vialacteastringdictwidget.ui 
    ui/viewselectedsourcedataset.ui 
    ui/vlkbquery.ui 
    ui/vlkbquerycomposer.ui 
    ui/vlkbsimplequerycomposer.ui 
    ui/vtkfitstoolswidget.ui 
    ui/vtkfitstoolwidget_new.ui 
    ui/vtktoolswidget.ui 
    ui/vtkwindow_new.ui)

set(xml_files
  SupportedReader.xml)

add_executable(ViaLacteaVisualAnalytics ${ui_files} ${sources} ${headers} )

target_link_libraries(ViaLacteaVisualAnalytics
 PRIVATE
   CFITSIO::CFITSIO
   ParaView::pqApplicationComponents
   ParaView::RemotingViews
   Qt5::Charts
   Qt5::Help
   Qt5::Purchasing
   Qt5::QuickTemplates2
   Qt5::UiPlugin
   Qt5::3DAnimation
   Qt5::ClipboardSupport
   Qt5::Qml
   Qt5::QuickTest
   Qt5::UiTools
   Qt5::3DCore
   Qt5::Concurrent
   Qt5::Location
   Qt5::QmlDebug
   Qt5::QuickWidgets
   Qt5::VirtualKeyboard
   Qt5::3DExtras
   Qt5::Core
   Qt5::MacExtras
   Qt5::RemoteObjects
   Qt5::WebChannel
   Qt5::3DInput
   Qt5::DBus
   Qt5::Multimedia
   Qt5::RepParser
   Qt5::WebEngine
   Qt5::3DLogic
   Qt5::DataVisualization
   Qt5::MultimediaQuick
   Qt5::QmlModels
   Qt5::Script
   Qt5::WebEngineCore
   Qt5::3DQuick
   Qt5::Designer
   Qt5::MultimediaWidgets
   Qt5::QmlWorkerScript
   Qt5::ScriptTools
   Qt5::WebEngineWidgets
   Qt5::3DQuickAnimation
   Qt5::DesignerComponents
   Qt5::Network
   Qt5::Quick
   Qt5::Scxml
   Qt5::WebSockets
   Qt5::3DQuickExtras
   Qt5::DeviceDiscoverySupport
   Qt5::NetworkAuth
   Qt5::Quick3D
   Qt5::Sensors
   Qt5::WebView
   Qt5::3DQuickInput
   Qt5::Nfc
   Qt5::Quick3DAssetImport
   Qt5::SerialBus
   Qt5::Widgets
   Qt5::3DQuickRender
   Qt5::EdidSupport
   Qt5::OpenGL
   Qt5::Quick3DRender
   Qt5::SerialPort
   Qt5::Xml
   Qt5::3DQuickScene2D
   Qt5::EventDispatcherSupport
   Qt5::OpenGLExtensions
   Qt5::Quick3DRuntimeRender
   Qt5::ServiceSupport
   Qt5::XmlPatterns
   Qt5::3DRender
   Qt5::FbSupport
   Qt5::PacketProtocol
   Qt5::Quick3DUtils
   Qt5::Sql
   Qt5::AccessibilitySupport
   Qt5::FontDatabaseSupport
   Qt5::PlatformCompositorSupport
   Qt5::Svg
   Qt5::Gamepad
   Qt5::Positioning
   Qt5::QuickControls2
   Qt5::Test
   Qt5::Bluetooth
   Qt5::GraphicsSupport
   Qt5::PositioningQuick
   Qt5::QuickParticles
   Qt5::TextToSpeech
   Qt5::Bodymovin
   Qt5::Gui
   Qt5::PrintSupport
   Qt5::QuickShapes
   Qt5::ThemeSupport
   )