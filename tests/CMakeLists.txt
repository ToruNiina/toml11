set(TEST_NAMES
    test_traits
    test_utility
    test_value
    test_to_toml
    test_from_toml
    test_get
    test_value_operator
    test_datetime
    test_acceptor
    test_parser
    test_parse_file
    )

CHECK_CXX_COMPILER_FLAG("-Wall" COMPILER_SUPPORTS_WALL)
CHECK_CXX_COMPILER_FLAG("-Wpedantic" COMPILER_SUPPORTS_WPEDANTIC)

if(COMPILER_SUPPORTS_WALL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

if(COMPILER_SUPPORTS_WPEDANTIC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")
endif()

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
add_definitions(-DBOOST_TEST_DYN_LINK)
add_definitions(-DUNITTEST_FRAMEWORK_LIBRARY_EXIST)

foreach(TEST_NAME ${TEST_NAMES})
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    target_include_directories(${TEST_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
endforeach(TEST_NAME)
