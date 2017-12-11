set(TEST_NAMES
    test_traits
    test_utility
    test_value
    test_to_toml
    test_from_toml
    test_get
    test_get_or
    test_value_operator
    test_datetime
    test_acceptor
    test_parser
    test_parse_file
    test_parse_unicode
    )

CHECK_CXX_COMPILER_FLAG("-Wall" COMPILER_SUPPORTS_WALL)
CHECK_CXX_COMPILER_FLAG("-Wpedantic" COMPILER_SUPPORTS_WPEDANTIC)

if(COMPILER_SUPPORTS_WALL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

if(COMPILER_SUPPORTS_WPEDANTIC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")
endif()

# Disable some MSVC warnings
if(MSVC)
    # conversion from 'double' to 'unsigned int', possible loss of data
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244")
    # conversion from 'int' to 'unsigned int', signed/unsigned mismatch
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4365")
    # layout of class may have changed from a previous version of the compiler
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4371")
    # enumerator in switch of enum is not explicitly handled by a case label
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4061")
    # unreferenced inline function has been removed
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4514")
    # constructor is not implicitly called
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4582")
    # destructor is not implicitly called
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4583")
    # pragma warning: there is no warning number <x>
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4619")
    # default constructor was implicitly defined as deleted
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4623")
    # copy constructor was implicitly defined as deleted
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4625")
    # assignment operator was implicitly defined as deleted
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4626")
    # move assignment operator was implicitly defined as deleted
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4627")
    # <X> is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4668")
    # function not inlined
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4710")
    # function selected for automatic inlining
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4711")
    # <x> bytes padding added after data member
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4820")
    # pragma warning(pop): likely mismatch, popping warning state pushed in different file
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5031")
endif()

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
add_definitions(-DBOOST_TEST_DYN_LINK)
add_definitions(-DUNITTEST_FRAMEWORK_LIBRARY_EXIST)

foreach(TEST_NAME ${TEST_NAMES})
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    target_include_directories(${TEST_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

    # Set the PATH to be able to find Boost DLL
    if(WIN32)
        STRING(REPLACE ";" "\\;" PATH_STRING "$ENV{PATH}")
        set_tests_properties(${TEST_NAME}
            PROPERTIES ENVIRONMENT "PATH=${PATH_STRING}\;${Boost_LIBRARY_DIRS}"
        )
    endif()
endforeach(TEST_NAME)
