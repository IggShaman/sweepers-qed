set(GIT_SHA "unknown")
set(GIT_DESCRIBE "unknown")
set(GIT_DIRTY "false")

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${SRC_DIR}/.git")
  execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
    WORKING_DIRECTORY "${SRC_DIR}" OUTPUT_VARIABLE _sha
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET RESULT_VARIABLE _rc)
  if(_rc EQUAL 0)
    set(GIT_SHA "${_sha}")
  endif()

  execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
    WORKING_DIRECTORY "${SRC_DIR}" OUTPUT_VARIABLE _desc
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET RESULT_VARIABLE _rc2)
  if(_rc2 EQUAL 0)
    set(GIT_DESCRIBE "${_desc}")
  endif()

  execute_process(COMMAND ${GIT_EXECUTABLE} diff --quiet HEAD --
    WORKING_DIRECTORY "${SRC_DIR}" RESULT_VARIABLE _dirty ERROR_QUIET)
  if(NOT _dirty EQUAL 0)
    set(GIT_DIRTY "true")
  endif()
endif()

# configure_file only rewrites when the content differs -> no spurious rebuilds
configure_file("${IN_FILE}" "${OUT_FILE}" @ONLY)
