include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.15.3
)
FetchContent_MakeAvailable(Catch2)

add_executable(neighborhood_test neighborhood_test.cpp)
target_include_directories(neighborhood_test PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(neighborhood_test PRIVATE Catch2::Catch2WithMain)
set_target_properties(neighborhood_test PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

add_executable(aligned_allocator_test aligned_allocator_test.cpp)
target_include_directories(aligned_allocator_test PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(aligned_allocator_test PRIVATE Catch2::Catch2WithMain)
set_target_properties(aligned_allocator_test PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

catch_discover_tests(neighborhood_test)
catch_discover_tests(aligned_allocator_test)
