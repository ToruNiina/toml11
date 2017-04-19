cmake_minimum_required(VERSION 2.8)
enable_testing()
project(toml11)

include_directories(${PROJECT_SOURCE_DIR})
add_definitions(-std=c++11)

add_subdirectory(tests)
