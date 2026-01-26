message(STATUS "Using QCustomPlot v2.1.1")

add_library(QCustomPlot STATIC
        qcustomplot.cpp
        qcustomplot.h
)

set_target_properties(QCustomPlot PROPERTIES
        AUTOUIC OFF
)

target_include_directories(QCustomPlot INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(QCustomPlot
        PRIVATE Qt::Core
        INTERFACE Qt::PrintSupport
)
