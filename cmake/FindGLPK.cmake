#[=======================================================================[.rst:
FindGLPK
--------
Finds the GNU Linear Programming Kit.

Imported target:  ``GLPK::GLPK``
Result variables: ``GLPK_FOUND`` ``GLPK_VERSION``
                  ``GLPK_INCLUDE_DIRS`` ``GLPK_LIBRARIES``
Hint variable:    ``GLPK_ROOT``
#]=======================================================================]

find_path(GLPK_INCLUDE_DIR
  NAMES glpk.h
  HINTS ${GLPK_ROOT} ENV GLPK_ROOT
  PATH_SUFFIXES include)

find_library(GLPK_LIBRARY
  NAMES glpk
  HINTS ${GLPK_ROOT} ENV GLPK_ROOT
  PATH_SUFFIXES lib lib64)

if(GLPK_INCLUDE_DIR AND EXISTS "${GLPK_INCLUDE_DIR}/glpk.h")
  file(STRINGS "${GLPK_INCLUDE_DIR}/glpk.h" _glpk_major
    REGEX "^#define[ \t]+GLP_MAJOR_VERSION[ \t]+[0-9]+")
  file(STRINGS "${GLPK_INCLUDE_DIR}/glpk.h" _glpk_minor
    REGEX "^#define[ \t]+GLP_MINOR_VERSION[ \t]+[0-9]+")
  string(REGEX REPLACE "[^0-9]" "" _glpk_major "${_glpk_major}")
  string(REGEX REPLACE "[^0-9]" "" _glpk_minor "${_glpk_minor}")
  if(_glpk_major MATCHES "^[0-9]+$" AND _glpk_minor MATCHES "^[0-9]+$")
    set(GLPK_VERSION "${_glpk_major}.${_glpk_minor}")
  endif()
  unset(_glpk_major)
  unset(_glpk_minor)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLPK
  REQUIRED_VARS GLPK_LIBRARY GLPK_INCLUDE_DIR
  VERSION_VAR   GLPK_VERSION)

if(GLPK_FOUND)
  set(GLPK_INCLUDE_DIRS ${GLPK_INCLUDE_DIR})
  set(GLPK_LIBRARIES    ${GLPK_LIBRARY})
  if(NOT TARGET GLPK::GLPK)
    add_library(GLPK::GLPK UNKNOWN IMPORTED)
    set_target_properties(GLPK::GLPK PROPERTIES
      IMPORTED_LOCATION             "${GLPK_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${GLPK_INCLUDE_DIR}")
  endif()
endif()

mark_as_advanced(GLPK_INCLUDE_DIR GLPK_LIBRARY)
