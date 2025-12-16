cmake_minimum_required(VERSION 3.16)

# Need both C and CXX so that FindMPI generates MPI::MPI_C (VTK::mpi depends on it)
project(VisIVO_remote_server LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Network WebSockets Gui)
find_package(
        VTK 9.1.0 REQUIRED QUIET
        COMPONENTS
        CommonCore
        CommonDataModel
        FiltersCore
        FiltersGeneral
        ImagingCore
        ImagingMath
        RenderingCore
        RenderingOpenGL2
        RenderingVolumeOpenGL2
        RenderingImage
        InteractionStyle
        IOImage)
find_package(CFITSIO 4.1.0 QUIET REQUIRED)
find_package(MPI REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
        libavcodec
        libavutil
        libswscale)

add_executable(VisIVO_remote_server
        main.cpp
        rpc_server.cpp
        rpc_server.h
        render_service.cpp
        render_service.h
        distributed_service.cpp
        distributed_service.h
        worker_loop.cpp
        worker_loop.h
        scene_manager.cpp
        scene_manager.h
        video_encoder.cpp
        video_encoder.h
        ../src/vtkfitsreader2.cpp
        ../src/luteditor.cpp
        ../src/color.cpp)

target_include_directories(VisIVO_remote_server PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(VisIVO_remote_server PRIVATE
        Qt6::Core Qt6::Network Qt6::WebSockets Qt6::Gui
        ${VTK_LIBRARIES}
        CFITSIO::CFITSIO
        MPI::MPI_CXX
        PkgConfig::FFMPEG)

if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(VisIVO_remote_server PRIVATE -stdlib=libc++)
    target_link_options(VisIVO_remote_server PRIVATE -stdlib=libc++)
endif()

set_target_properties(VisIVO_remote_server PROPERTIES
        OUTPUT_NAME VisIVO_remote_server)

vtk_module_autoinit(TARGETS VisIVO_remote_server MODULES ${VTK_LIBRARIES})
