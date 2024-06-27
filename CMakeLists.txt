cmake_minimum_required(VERSION 3.16)

# project_source_dir has not been set yet
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/include/toml11/version.hpp" TOML11_MAJOR_VERSION_STRING
    REGEX "#define TOML11_VERSION_MAJOR ([0-9]+)")
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/include/toml11/version.hpp" TOML11_MINOR_VERSION_STRING
    REGEX "#define TOML11_VERSION_MINOR ([0-9]+)")
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/include/toml11/version.hpp" TOML11_PATCH_VERSION_STRING
    REGEX "#define TOML11_VERSION_PATCH ([0-9]+)")

string(REGEX REPLACE "#define TOML11_VERSION_MAJOR ([0-9]+)" "\\1" TOML11_VERSION_MAJOR "${TOML11_MAJOR_VERSION_STRING}")
string(REGEX REPLACE "#define TOML11_VERSION_MINOR ([0-9]+)" "\\1" TOML11_VERSION_MINOR "${TOML11_MINOR_VERSION_STRING}")
string(REGEX REPLACE "#define TOML11_VERSION_PATCH ([0-9]+)" "\\1" TOML11_VERSION_PATCH "${TOML11_PATCH_VERSION_STRING}")

project(toml11 LANGUAGES CXX VERSION "${TOML11_VERSION_MAJOR}.${TOML11_VERSION_MINOR}.${TOML11_VERSION_PATCH}")

include(CMakeDependentOption)
include(CTest)

option(TOML11_PRECOMPILE "precompile toml11 library" OFF)

cmake_dependent_option(TOML11_INSTALL "install toml11 library" ON
    "${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME}" OFF)
cmake_dependent_option(TOML11_BUILD_EXAMPLES "build toml11 examples" OFF
    "${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME}" OFF)
cmake_dependent_option(TOML11_BUILD_TESTS "build toml11 unit tests" OFF
    "${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME}; ${BUILD_TESTING}" OFF)
cmake_dependent_option(TOML11_BUILD_TOML_TESTS "build toml11 toml-test encoder & decoder" OFF
    "${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME}" OFF)
cmake_dependent_option(TOML11_TEST_WITH_ASAN  "build toml11 unit tests with asan" OFF
    "${TOML11_BUILD_TESTS}" OFF)
cmake_dependent_option(TOML11_TEST_WITH_UBSAN "build toml11 unit tests with ubsan" OFF
    "${TOML11_BUILD_TESTS}" OFF)

if(${TOML11_TEST_WITH_ASAN} AND ${TOML11_TEST_WITH_UBSAN})
    message(FATAL_ERROR "trying to build tests with BOTH asan and ubsan")
endif()

include(GNUInstallDirs)

set(TOML11_INSTALL_CMAKE_DIR   ${CMAKE_INSTALL_LIBDIR}/cmake/toml11)
set(TOML11_INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR})
set(TOML11_CONFIG_DIR          ${CMAKE_CURRENT_BINARY_DIR}/cmake)
set(TOML11_CONFIG              ${TOML11_CONFIG_DIR}/toml11Config.cmake)
set(TOML11_CONFIG_VERSION      ${TOML11_CONFIG_DIR}/toml11ConfigVersion.cmake)

# root project?
if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    if(NOT DEFINED CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD 11)
    endif()
    if(NOT DEFINED CMAKE_CXX_EXTENSIONS)
        set(CMAKE_CXX_EXTENSIONS OFF)
    endif()
    if(NOT DEFINED CMAKE_CXX_STANDARD_REQUIRED)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
    endif()

    if(${TOML11_BUILD_TESTS} OR ${TOML11_BUILD_TOML_TESTS})
        add_subdirectory(tests)
    endif()

    if(${TOML11_BUILD_EXAMPLES})
        add_subdirectory(examples)
    endif()
endif()

add_subdirectory(src)
