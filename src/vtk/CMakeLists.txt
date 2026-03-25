add_library(visivo_shared_vtk STATIC
        ${PROJECT_SOURCE_DIR}/src/vtk/ColorMaps.cpp
        ${PROJECT_SOURCE_DIR}/src/vtk/ColorMaps.h
        ${PROJECT_SOURCE_DIR}/src/vtk/ImageLayer.cpp
        ${PROJECT_SOURCE_DIR}/src/vtk/ImageLayer.h
        ${PROJECT_SOURCE_DIR}/src/vtk/ImageLayerSet.cpp
        ${PROJECT_SOURCE_DIR}/src/vtk/ImageLayerSet.h
        ${PROJECT_SOURCE_DIR}/src/vtk/MomentMapComputeTask.cpp
        ${PROJECT_SOURCE_DIR}/src/vtk/MomentMapComputeTask.h
        ${PROJECT_SOURCE_DIR}/src/vtk/MomentProcessingService.cpp
        ${PROJECT_SOURCE_DIR}/src/vtk/MomentProcessingService.h
        ${PROJECT_SOURCE_DIR}/src/vtk/vtkFITSReader.cpp
        ${PROJECT_SOURCE_DIR}/src/vtk/vtkFITSReader.h
        ${PROJECT_SOURCE_DIR}/src/vtk/vtkFITSWriter.cpp
        ${PROJECT_SOURCE_DIR}/src/vtk/vtkFITSWriter.h
        ${PROJECT_SOURCE_DIR}/src/vtk/vtkInteractorStyleProfile.cpp
        ${PROJECT_SOURCE_DIR}/src/vtk/vtkInteractorStyleProfile.h
        ${PROJECT_SOURCE_DIR}/src/vtk/vtkLegendScaleActorWCS.cpp
        ${PROJECT_SOURCE_DIR}/src/vtk/vtkLegendScaleActorWCS.h
        ${PROJECT_SOURCE_DIR}/src/vtk/vtkMomentMapFilter.cpp
        ${PROJECT_SOURCE_DIR}/src/vtk/vtkMomentMapFilter.h
)

target_link_libraries(visivo_shared_vtk
        PUBLIC
        ${VTK_LIBRARIES}
        PRIVATE
        visivo_shared_core
        wcs
)

target_include_directories(visivo_shared_vtk
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/vtk
)
