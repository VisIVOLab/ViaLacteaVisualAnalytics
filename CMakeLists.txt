cmake_minimum_required(VERSION 3.22)

project(VisIVOVisualAnalytics
        VERSION 0.0.1
        LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Configuring ${PROJECT_NAME} v${PROJECT_VERSION}")

set(qt_components
        Widgets
        Network
        NetworkAuth
        PrintSupport
        WebEngineWidgets
)
foreach (module IN LISTS qt_components)
    list(APPEND qt_modules "Qt::${module}")
endforeach ()

find_package(Qt6 6.10.0 CONFIG REQUIRED COMPONENTS ${qt_components})
message(STATUS "Using Qt v${Qt6_VERSION}")

find_package(VTK 9.5.0 CONFIG REQUIRED COMPONENTS
        CommonColor
        FiltersModeling
        FiltersStatistics
        GUISupportQt
        InteractionStyle
        RenderingAnnotation
        RenderingImage
        RenderingVolumeOpenGL2
)
message(STATUS "Using VTK v${VTK_VERSION}")

find_package(cfitsio CONFIG REQUIRED)
message(STATUS "Using cfitsio v${cfitsio_VERSION}")

get_target_property(cfitsio_include_dirs CFITSIO::cfitsio INTERFACE_INCLUDE_DIRECTORIES)
if (cfitsio_include_dirs)
    list(FILTER cfitsio_include_dirs EXCLUDE REGEX
            "^/Library/Developer/CommandLineTools/SDKs/.*/usr/include$")
    set_target_properties(CFITSIO::cfitsio PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${cfitsio_include_dirs}"
    )
endif ()

qt_standard_project_setup()
set(CMAKE_AUTOUIC OFF)

configure_file(src/Version.h.in Version.h @ONLY)

set(PROJECT_SOURCES
        src/LayerListModel.cpp
        src/LayerListModel.h
        src/main.cpp
        src/Settings.cpp
        src/Settings.h
        src/WebViewProcess.cpp
        src/WebViewProcess.h
        src/auth/AuthKeys.h
        src/auth/AuthWrapper.cpp
        src/auth/AuthWrapper.h
        src/auth/HttpServerReplyHandler.cpp
        src/auth/HttpServerReplyHandler.h
        src/auth/OIDCAuthorizationCodeFlow.cpp
        src/auth/OIDCAuthorizationCodeFlow.h
        src/auth/VisIVOUrlSchemeHandler.cpp
        src/auth/VisIVOUrlSchemeHandler.h
        src/gui/AboutDialog.cpp
        src/gui/AboutDialog.h
        src/gui/CubeViewController.cpp
        src/gui/CubeViewController.h
        src/gui/DatasetWindowFactory.cpp
        src/gui/DatasetWindowFactory.h
        src/gui/ImageLayerController.cpp
        src/gui/ImageLayerController.h
        src/gui/LUTCustomizerDialog.cpp
        src/gui/LUTCustomizerDialog.h
        src/gui/MainWindow.cpp
        src/gui/MainWindow.h
        src/gui/ProfileWidget.cpp
        src/gui/ProfileWidget.h
        src/gui/SettingsDialog.cpp
        src/gui/SettingsDialog.h
        src/gui/vtkWindowCube.cpp
        src/gui/vtkWindowCube.h
        src/gui/vtkWindowImage.cpp
        src/gui/vtkWindowImage.h
)

set(PROJECT_UI
        ui/AboutDialog.ui
        ui/LUTCustomizerDialog.ui
        ui/MainWindow.ui
        ui/ProfileWidget.ui
        ui/SettingsDialog.ui
        ui/vtkWindowCube.ui
        ui/vtkWindowImage.ui
)

set(PROJECT_ASSETS
        icons/COLORIZE.png
        icons/FILE_OPEN.png
        icons/PIC_BACK.png
        icons/PIC_BOTTOM.png
        icons/PIC_FRONT.png
        icons/PIC_LEFT.png
        icons/PIC_RIGHT.png
        icons/PIC_TOP.png
        icons/RECT_SELECT.png
        icons/RESET.png
        icons/VisIVO.icns
        icons/VisIVO.svg
        icons/VisIVO_32.png
        icons/VisIVO_48.png
        icons/VisIVO_128.png
        icons/VisIVO_256.png
        icons/VisIVO_512.png
)

if (APPLE)
    set(PROJECT_ICON icons/VisIVO.icns)
    set(MACOSX_BUNDLE_ICON_FILE VisIVO.icns)
    set_source_files_properties(${PROJECT_ICON} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
    )
endif ()

qt_add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
        ${PROJECT_ICON}
)

qt_add_ui(${PROJECT_NAME}
        SOURCES ${PROJECT_UI}
)

qt_add_resources(${PROJECT_NAME} assets
        FILES ${PROJECT_ASSETS}
)

add_subdirectory(src/libwcs)

add_subdirectory(src/qcustomplot)

add_subdirectory(src/core)

add_subdirectory(src/vtk)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
)

target_include_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/auth"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/libwcs"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        visivo_shared_core
        visivo_shared_vtk
        ${qt_modules}
        QCustomPlot
)

if (APPLE)
    target_link_options(${PROJECT_NAME} PRIVATE
            "LINKER:-no_warn_duplicate_libraries"
    )
endif ()

vtk_module_autoinit(
        TARGETS ${PROJECT_NAME}
        MODULES ${VTK_LIBRARIES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER it.inaf.oact.VisIVO
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE TRUE
)

install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
        TARGET ${PROJECT_NAME}
        OUTPUT_SCRIPT deploy_script
)

install(SCRIPT ${deploy_script})
