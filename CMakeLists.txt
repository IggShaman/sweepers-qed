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

qt_add_executable(
  sweepers-qed
  solver.cpp
  field.cpp
  board.cpp
  game_board_widget.cpp
  main_window.cpp
  main.cpp
  glpk_solver.cpp
  glpk_lp_problem.cpp
  sweeper.qrc
)

target_link_libraries(
  sweepers-qed
  PRIVATE
  Qt6::Core
  Qt6::Widgets
  Threads::Threads
  glpk
)
