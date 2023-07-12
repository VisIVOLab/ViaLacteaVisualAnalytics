# FindCFITSIO.cmake
# Version 1.0
#[=======================================================================[.rst:
FindCFITSIO
-----------

Finds the CFITSIO library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``CFITSIO::CFITSIO``
  The CFITSIO library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``CFITSIO_FOUND``
  True if the system has the CFITSIO library.
``CFITSIO_VERSION``
  The version of the CFITSIO library which was found.
``CFITSIO_INCLUDE_DIRS``
  Include directories needed to use CFITSIO.
``CFITSIO_LIBRARIES``
  Libraries needed to link to CFITSIO.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``CFITSIO_INCLUDE_DIR``
  The directory containing ``fitsio.h``.
``CFITSIO_LIBRARY``
  The path to the CFITSIO library.

#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_CFITSIO QUIET cfitsio)

find_path(CFITSIO_INCLUDE_DIR
  NAMES fitsio.h
  PATHS ${PC_CFITSIO_INCLUDE_DIRS} ${CFITSIO_ROOT_DIR}
  PATH_SUFFIXES include include/cfitsio
  DOC "Include directory for CFITSIO."
)
find_library(CFITSIO_LIBRARY
  NAMES cfitsio
  PATHS ${PC_CFITSIO_LIBRARY_DIRS} ${CFITSIO_ROOT_DIR}
  PATH_SUFFIXES lib
  DOC "Path to CFITSIO library."
)

if(PC_CFITSIO_VERSION)
  set(CFITSIO_VERSION ${PC_CFITSIO_VERSION})
elseif(CFITSIO_INCLUDE_DIR)
  file(READ "${CFITSIO_INCLUDE_DIR}/fitsio.h" ver)
  string(REGEX MATCH "CFITSIO_VERSION ([0-9\.]*)" _ ${ver})
  set(CFITSIO_VERSION ${CMAKE_MATCH_1})
endif()
set(CFITSIO_VERSION_STRING ${CFITSIO_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CFITSIO
  FOUND_VAR CFITSIO_FOUND
  REQUIRED_VARS
    CFITSIO_LIBRARY
    CFITSIO_INCLUDE_DIR
  VERSION_VAR CFITSIO_VERSION
  FAIL_MESSAGE "Could NOT find CFITSIO. You can specify a non-standard installation using CFITSIO_ROOT_DIR."
)

if(CFITSIO_FOUND)
  set(CFITSIO_LIBRARIES ${CFITSIO_LIBRARY})
  set(CFITSIO_INCLUDE_DIRS ${CFITSIO_INCLUDE_DIR})
  set(CFITSIO_DEFINITIONS ${PC_CFITSIO_CFLAGS_OTHER})
endif()

if(CFITSIO_FOUND AND NOT TARGET CFITSIO::CFITSIO)
  add_library(CFITSIO::CFITSIO UNKNOWN IMPORTED)
  set_target_properties(CFITSIO::CFITSIO PROPERTIES
    IMPORTED_LOCATION "${CFITSIO_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${PC_CFITSIO_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${CFITSIO_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(
  CFITSIO_INCLUDE_DIR
  CFITSIO_LIBRARY
)
