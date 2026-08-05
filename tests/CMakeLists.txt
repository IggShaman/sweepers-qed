add_executable(neighborhood_test neighborhood_test.cpp)
target_include_directories(neighborhood_test PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(neighborhood_test PRIVATE Catch2::Catch2WithMain)
set_target_properties(neighborhood_test PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

add_executable(aligned_allocator_test aligned_allocator_test.cpp)
target_include_directories(aligned_allocator_test PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(aligned_allocator_test PRIVATE Catch2::Catch2WithMain)
set_target_properties(aligned_allocator_test PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

add_executable(byte_field_test byte_field_test.cpp)
target_include_directories(byte_field_test PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(byte_field_test PRIVATE Catch2::Catch2WithMain)
set_target_properties(byte_field_test PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

add_executable(generate_minefield_test generate_minefield_test.cpp)
target_include_directories(generate_minefield_test PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(generate_minefield_test PRIVATE Catch2::Catch2WithMain)
set_target_properties(generate_minefield_test PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

add_executable(glpk_solver_test glpk_solver_test.cpp)
target_include_directories(glpk_solver_test PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(glpk_solver_test PRIVATE Catch2::Catch2WithMain qed_core)
set_target_properties(glpk_solver_test PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

add_executable(benchmark_test benchmark_test.cpp)
target_include_directories(benchmark_test PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(benchmark_test PRIVATE Catch2::Catch2WithMain benchmarks_base Qt6::Core Qt6::Sql)
set_target_properties(benchmark_test PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

catch_discover_tests(neighborhood_test)
catch_discover_tests(aligned_allocator_test)
catch_discover_tests(byte_field_test)
catch_discover_tests(generate_minefield_test)
catch_discover_tests(glpk_solver_test)
catch_discover_tests(benchmark_test)

add_library(
  benchmarks_base
  field_config.cpp
  benchmark_stats.cpp
)
target_include_directories(benchmarks_base PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(
  benchmarks_base
  PRIVATE
  qed_core
  qed_debug
  tomlplusplus::tomlplusplus
  Qt6::Sql
)
set_target_properties(benchmarks_base PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

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
  benchmarks_base
  CLI11::CLI11
  Qt6::Sql
)
set_target_properties(solver_benchmark PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

add_executable(
  make_sample_fields
  make_sample_fields.cpp
)
target_include_directories(make_sample_fields PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(make_sample_fields PRIVATE benchmarks_base CLI11::CLI11 qed_core)
set_target_properties(make_sample_fields PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
