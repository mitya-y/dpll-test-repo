cmake_minimum_required(VERSION 3.20)

# project(dpll)

enable_testing()
add_subdirectory(googletest)
# add_executable(TestDPLL test_loading.cpp test_algorithm.cpp)
# file(test_loading.cpp test_algorithm.cpp)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE test_loading.cpp test_algorithm.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME} gtest_main gmock_main)

execute_process(COMMAND "")
