if(WIN32)
    set(win_headers win_time.h win_fixes.h)
endif()

add_library(wcs STATIC
    actread.c
    binread.c
    catutil.c
    cel.c
    ctgread.c
    daoread.c
    dateutil.c
    distort.c
    dsspos.c
    fileutil.c
    findstar.c
    fitsfile.c
    fitsfile.h
    fitshead.h
    fitswcs.c
    fitswcs.h
    gsc2read.c
    gscread.c
    hget.c
    hput.c
    iget.c
    imgetwcs.c
    imhfile.c
    imio.c
    imio.h
    imrotate.c
    imsetwcs.c
    imutil.c
    lin.c
    lwcs.h
    matchstar.c
    platefit.c
    platepos.c
    poly.c
    proj.c
    sdssread.c
    skybotread.c
    sortstar.c
    sph.c
    tabread.c
    tmcread.c
    tnxpos.c
    ty2read.c
    uacread.c
    ubcread.c
    ucacread.c
    ujcread.c
    wcs.c
    wcs.h
    wcscat.h
    wcscon.c
    wcsinit.c
    wcslib.c
    wcslib.h
    wcstrig.c
    webread.c
    worldpos.c
    zpxpos.c
    ${win_headers}
)

set_target_properties(wcs PROPERTIES
    AUTOMOC OFF
    AUTOUIC OFF
)

target_compile_definitions(wcs PRIVATE _FILE_OFFSET_BITS=64)

target_include_directories(wcs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
