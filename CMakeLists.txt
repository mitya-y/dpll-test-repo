cmake_minimum_required(VERSION 3.20)

# set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE Release)

project("dpll")

add_executable(${CMAKE_PROJECT_NAME} utils/dimacs_loader.cpp utils/dpll.cpp main.cpp)
add_subdirectory(tests)

set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 23)
