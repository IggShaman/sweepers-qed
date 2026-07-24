cmake_minimum_required(VERSION 4.2.0)
project(LANDMINE CXX)

SET(CMAKE_CXX_STANDARD 26)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_AUTORCC ON)

qt_add_executable(
  landmine
  solver.cc
  field.cc
  board.cc
  game_board_widget.cc
  main_window.cc
  main.cpp
  glpk_solver.cc
  glpk_lp_problem.cc
  landmine.qrc
)

target_link_libraries(
  landmine
  PRIVATE
  Qt6::Core
  Qt6::Widgets
  Threads::Threads
  glpk
)
