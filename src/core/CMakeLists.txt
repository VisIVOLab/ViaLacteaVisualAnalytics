add_library(visivo_shared_core STATIC
        ${PROJECT_SOURCE_DIR}/src/AstroUtils.cpp
        ${PROJECT_SOURCE_DIR}/src/AstroUtils.h
        ${PROJECT_SOURCE_DIR}/src/app/DatasetOpenService.cpp
        ${PROJECT_SOURCE_DIR}/src/app/DatasetOpenService.h
        ${PROJECT_SOURCE_DIR}/src/app/DatasetOpenTypes.h
)

target_link_libraries(visivo_shared_core
        PUBLIC
        Qt::Core
        CFITSIO::cfitsio
        PRIVATE
        wcs
)

target_include_directories(visivo_shared_core
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/app
)
