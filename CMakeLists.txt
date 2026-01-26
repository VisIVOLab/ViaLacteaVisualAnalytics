cmake_minimum_required(VERSION 3.23)

project(VisIVOVisualAnalytics
        VERSION 0.0.1
        LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Configuring ${PROJECT_NAME} v${PROJECT_VERSION}")

set(qt_components
        Widgets
)
foreach (module IN LISTS qt_components)
    list(APPEND qt_modules "Qt::${module}")
endforeach ()

find_package(Qt6 6.10.0 CONFIG REQUIRED COMPONENTS ${qt_components})
message(STATUS "Using Qt v${Qt6_VERSION}")

find_package(VTK 9.5.0 CONFIG REQUIRED COMPONENTS
        GUISupportQt
        InteractionStyle
        RenderingAnnotation
        RenderingImage
)
message(STATUS "Using VTK v${VTK_VERSION}")

find_package(cfitsio CONFIG REQUIRED)
message(STATUS "Using cfitsio ${cfitsio_VERSION}")

qt_standard_project_setup()
set(CMAKE_AUTOUIC OFF)

configure_file(src/Version.h.in Version.h @ONLY)

set(PROJECT_SOURCES
        src/main.cpp
        src/AstroUtils.h
        src/AstroUtils.cpp
        src/LayerListModel.cpp
        src/LayerListModel.h
        src/gui/MainWindow.cpp
        src/gui/MainWindow.h
        src/gui/vtkWindowImage.cpp
        src/gui/vtkWindowImage.h
        src/vtk/ColorMaps.cpp
        src/vtk/ColorMaps.h
        src/vtk/ImageLayer.cpp
        src/vtk/ImageLayer.h
        src/vtk/vtkFITSReader.cpp
        src/vtk/vtkFITSReader.h
)

set(PROJECT_UI
        ui/MainWindow.ui
        ui/vtkWindowImage.ui
)

set(PROJECT_ASSETS
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

target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
)

target_include_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/vtk"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${qt_modules}
        ${VTK_LIBRARIES}
        CFITSIO::cfitsio
        wcs
)

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
