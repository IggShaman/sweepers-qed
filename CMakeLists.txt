cmake_minimum_required(VERSION 4.2.0)
project(SWEEPERS_QED CXX)

SET(CMAKE_CXX_STANDARD 26)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
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
# qed_core: model + solvers, no Qt or UI
#
add_library(
  qed_core
  solver.cpp
  glpk_solver.cpp
  glpk_lp_problem.cpp
)

target_include_directories(qed_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(qed_core PUBLIC Threads::Threads glpk)
target_compile_features(qed_core PUBLIC cxx_std_26)
set_target_properties(qed_core PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

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
  Qt6::Widgets)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
