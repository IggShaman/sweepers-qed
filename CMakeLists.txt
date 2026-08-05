cmake_minimum_required(VERSION 4.2.0)
project(SWEEPERS_QED CXX)

SET(CMAKE_CXX_STANDARD 26)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(Threads REQUIRED)
find_package(GLPK 5.0 REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)
qt_standard_project_setup()

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_AUTORCC ON)

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.15.3
)
FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(CLI11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG        v2.7.2)
FetchContent_MakeAvailable(CLI11)

FetchContent_Declare(tomlplusplus
  GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
  GIT_TAG        v3.4.0)
FetchContent_MakeAvailable(tomlplusplus)

#
# build info
#
string(TOUPPER "${CMAKE_BUILD_TYPE}" _bt_upper)
set(BUILD_INFO_HPP ${CMAKE_BINARY_DIR}/generated/build_info.hpp)

add_custom_target(generate_build_info ALL
  COMMAND ${CMAKE_COMMAND}
    -D "SRC_DIR=${CMAKE_SOURCE_DIR}"
    -D "IN_FILE=${CMAKE_SOURCE_DIR}/cmake/build_info.hpp.in"
    -D "OUT_FILE=${BUILD_INFO_HPP}"
    -D "CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}"
    -D "CXX_COMPILER_VERSION=${CMAKE_CXX_COMPILER_VERSION}"
    -D "BUILD_TYPE=${CMAKE_BUILD_TYPE}"
    -D "CXX_FLAGS=${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${_bt_upper}}"
    -D "CXX_STANDARD=${CMAKE_CXX_STANDARD}"
    -D "SYSTEM_NAME=${CMAKE_SYSTEM_NAME}"
    -D "SYSTEM_PROC=${CMAKE_SYSTEM_PROCESSOR}"
    -P "${CMAKE_SOURCE_DIR}/cmake/GenerateBuildInfo.cmake"
  BYPRODUCTS ${BUILD_INFO_HPP}
  COMMENT "Capturing build info"
  VERBATIM)

#
# qed_core: model + solvers, no Qt or UI
#
add_library(
  qed_core
  solver.cpp
  glpk_solver.cpp
  glpk_lp_problem.cpp
)

target_include_directories(qed_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(qed_core PUBLIC Threads::Threads GLPK::GLPK)
target_compile_features(qed_core PUBLIC cxx_std_26)
set_target_properties(qed_core PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
# qed_core hosts build_info
add_dependencies(qed_core generate_build_info)
target_include_directories(qed_core PUBLIC ${CMAKE_BINARY_DIR}/generated)

#
# qed_debug: helpers for debugging, unit tests, benchmarks
#
add_library(
  qed_debug
  byte_field_image_saver.cpp
)

target_include_directories(qed_debug PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(qed_debug qed_core Qt6::Gui)
target_compile_features(qed_debug PUBLIC cxx_std_26)
set_target_properties(qed_debug PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

#
# make_sample_fields
#
add_executable(
  make_sample_fields
  make_sample_fields.cpp
)
target_include_directories(make_sample_fields PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(make_sample_fields PRIVATE benchmark_base CLI11::CLI11 qed_core)
set_target_properties(make_sample_fields PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

#
# benchmarking
#

add_library(
  benchmark_base
  field_config.cpp
  benchmark_stats.cpp
)
target_include_directories(benchmark_base PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(
  benchmark_base
  PRIVATE
  qed_core
  qed_debug
  tomlplusplus::tomlplusplus
  Qt6::Sql
)
set_target_properties(benchmark_base PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

add_executable(
  solver_benchmark
  solver_benchmark.cpp
  benchmark_options.cpp
  benchmark_runner.cpp
)
target_include_directories(solver_benchmark PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(
  solver_benchmark
  PRIVATE
  benchmark_base
  CLI11::CLI11
  Qt6::Sql
)
set_target_properties(solver_benchmark PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

#
# The GUI application
#
qt_add_executable(
  sweepers-qed
  main.cpp
  main_window.cpp
  game_board_widget.cpp
  sweeper.qrc
)

target_link_libraries(
  sweepers-qed
  PRIVATE
  qed_core
  Qt6::Core
  Qt6::Widgets
)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
