cmake_minimum_required(VERSION 3.5)
enable_testing()

project(toml11 VERSION 3.8.1)

option(toml11_BUILD_TEST "Build toml tests" OFF)
option(toml11_INSTALL "Install CMake targets during install step." ON)
option(toml11_TEST_WITH_ASAN  "use LLVM address sanitizer" OFF)
option(toml11_TEST_WITH_UBSAN "use LLVM undefined behavior sanitizer" OFF)

option(TOML11_USE_UNRELEASED_TOML_FEATURES
    "use features in toml-lang/toml master while testing" OFF)

if(MSVC)
#     add_definitions("/Zc:__cplusplus")  # define __cplusplus value correctly
    add_definitions("/utf-8")           # enable to use u8"" literal
    if(MSVC_VERSION LESS 1910)
        message(STATUS "MSVC < 1910. DEFINE_CONVERSION_NON_INTRUSIVE is disabled")
        add_definitions(-DTOML11_WITHOUT_DEFINE_NON_INTRUSIVE)
    elseif(MSVC_VERSION LESS 1920)
        add_definitions("/experimental:preprocessor") # MSVC 2017
    else()
        add_definitions("/Zc:preprocessor") # MSVC 2019
    endif()
endif()

# Set some common directories
include(GNUInstallDirs)
set(toml11_install_cmake_dir         ${CMAKE_INSTALL_LIBDIR}/cmake/toml11)
set(toml11_install_include_dir       ${CMAKE_INSTALL_INCLUDEDIR})
set(toml11_config_dir                ${CMAKE_CURRENT_BINARY_DIR}/cmake/)
set(toml11_config                    ${toml11_config_dir}/toml11Config.cmake)
set(toml11_config_version            ${toml11_config_dir}/toml11ConfigVersion.cmake)

add_library(toml11 INTERFACE)
target_include_directories(toml11 INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${toml11_install_include_dir}>
)
add_library(toml11::toml11 ALIAS toml11)

# Write config and version config files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${toml11_config_version}
    VERSION ${toml11_VERSION}
    COMPATIBILITY SameMajorVersion
)

if (toml11_INSTALL)
    configure_package_config_file(
        cmake/toml11Config.cmake.in
        ${toml11_config}
        INSTALL_DESTINATION ${toml11_install_cmake_dir}
        PATH_VARS toml11_install_cmake_dir
    )

    # Install config files
    install(FILES ${toml11_config} ${toml11_config_version}
        DESTINATION ${toml11_install_cmake_dir}
    )

    # Install header files
    install(
        FILES toml.hpp
        DESTINATION "${toml11_install_include_dir}"
    )
    install(
        DIRECTORY "toml"
        DESTINATION "${toml11_install_include_dir}"
        FILES_MATCHING PATTERN "*.hpp"
    )

    # Export targets and install them
    install(TARGETS toml11
        EXPORT toml11Targets
    )
    install(EXPORT toml11Targets
        FILE toml11Targets.cmake
        DESTINATION ${toml11_install_cmake_dir}
        NAMESPACE toml11::
    )
endif()

if (toml11_BUILD_TEST)
    add_subdirectory(tests)
endif ()
