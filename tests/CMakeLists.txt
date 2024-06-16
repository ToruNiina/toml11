set(TOML11_TEST_NAMES
    test_comments
    test_datetime
    test_find
    test_find_or
    test_format_integer
    test_format_floating
    test_format_table
    test_get
    test_get_or
    test_location
    test_literal
    test_parse_null
    test_parse_boolean
    test_parse_integer
    test_parse_floating
    test_parse_string
    test_parse_datetime
    test_parse_array
    test_parse_inline_table
    test_parse_table_keys
    test_parse_table
    test_result
    test_scanner
    test_syntax_boolean
    test_syntax_integer
    test_syntax_floating
    test_syntax_string
    test_syntax_datetime
    test_syntax_key
    test_syntax_comment
    test_spec
    test_storage
    test_traits
    test_types
    test_utility
    test_user_defined_conversion
    test_value
    test_visit
    )

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-Wall"      COMPILER_SUPPORTS_WALL)
check_cxx_compiler_flag("-Wextra"    COMPILER_SUPPORTS_WEXTRA)
check_cxx_compiler_flag("-Wpedantic" COMPILER_SUPPORTS_WPEDANTIC)
check_cxx_compiler_flag("-Werror"    COMPILER_SUPPORTS_WERROR)

check_cxx_compiler_flag("-Wsign-conversion"     COMPILER_SUPPORTS_WSIGN_CONVERSION)
check_cxx_compiler_flag("-Wconversion"          COMPILER_SUPPORTS_WCONVERSION)
check_cxx_compiler_flag("-Wduplicated-cond"     COMPILER_SUPPORTS_WDUPLICATED_COND)
check_cxx_compiler_flag("-Wduplicated-branches" COMPILER_SUPPORTS_WDUPLICATED_BRANCHES)
check_cxx_compiler_flag("-Wlogical-op"          COMPILER_SUPPORTS_WLOGICAL_OP)
check_cxx_compiler_flag("-Wdouble-promotion"    COMPILER_SUPPORTS_WDOUBLE_PROMOTION)
check_cxx_compiler_flag("-Wrange-loop-analysis" COMPILER_SUPPORTS_WRANGE_LOOP_ANALYSIS)
check_cxx_compiler_flag("-Wundef"               COMPILER_SUPPORTS_WUNDEF)
check_cxx_compiler_flag("-Wshadow"              COMPILER_SUPPORTS_WSHADOW)

if(BUILD_TESTING)
    add_library(toml11_test_utility STATIC utility.cpp)
    target_include_directories(toml11_test_utility
        PRIVATE ${PROJECT_SOURCE_DIR}/tests/extlib/doctest/doctest/
        )
    target_link_libraries(toml11_test_utility PUBLIC toml11)

    foreach(TEST_NAME ${TOML11_TEST_NAMES})
        add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
        target_include_directories(${TEST_NAME}
            PRIVATE ${PROJECT_SOURCE_DIR}/tests/extlib/doctest/doctest/
            )
        target_link_libraries(${TEST_NAME} PUBLIC toml11 toml11_test_utility)
        if(MSVC)
            target_compile_options(${TEST_NAME} PRIVATE /W3)
        else()
            target_compile_options(${TEST_NAME} PRIVATE
                $<$<BOOL:${COMPILER_SUPPORTS_WALL}>:                 -Wall                >
                $<$<BOOL:${COMPILER_SUPPORTS_WEXTRA}>:               -Wextra              >
                $<$<BOOL:${COMPILER_SUPPORTS_WPEDANTIC}>:            -Wpedantic           >
                $<$<BOOL:${COMPILER_SUPPORTS_WSIGN_CONVERSION}>:     -Wsign-conversion    >
                $<$<BOOL:${COMPILER_SUPPORTS_WCONVERSION}>:          -Wconversion         >
                $<$<BOOL:${COMPILER_SUPPORTS_WDUPLICATED_COND}>:     -Wduplicated-cond    >
                $<$<BOOL:${COMPILER_SUPPORTS_WDUPLICATED_BRANCHES}>: -Wduplicated-branches>
                $<$<BOOL:${COMPILER_SUPPORTS_WLOGICAL_OP}>:          -Wlogical-op         >
                $<$<BOOL:${COMPILER_SUPPORTS_WDOUBLE_PROMOTION}>:    -Wdouble-promotion   >
                $<$<BOOL:${COMPILER_SUPPORTS_WRANGE_LOOP_ANALYSIS}>: -Wrange-loop-analysis>
                $<$<BOOL:${COMPILER_SUPPORTS_WUNDEF}>:               -Wundef              >
                $<$<BOOL:${COMPILER_SUPPORTS_WSHADOW}>:              -Wshadow             >
                $<$<BOOL:${TOML11_TEST_WITH_ASAN}>:                  -fsanitize=address   >
                $<$<BOOL:${TOML11_TEST_WITH_UBSAN}>:                 -fsanitize=undefined >
                )
            target_link_options(${TEST_NAME} PRIVATE
                $<$<BOOL:${TOML11_TEST_WITH_ASAN}>:                  -fsanitize=address   >
                $<$<BOOL:${TOML11_TEST_WITH_UBSAN}>:                 -fsanitize=undefined >
                )
        endif()
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach(TEST_NAME)
endif(BUILD_TESTING)

# =============================================================================
# toml-test encoder/decoder

if(TOML11_BUILD_TOML_TESTS)

    add_executable(toml11_decoder to_json.cpp)
    target_include_directories(toml11_decoder
        PRIVATE ${PROJECT_SOURCE_DIR}/tests/extlib/doctest/doctest/
        PRIVATE ${PROJECT_SOURCE_DIR}/tests/extlib/json/include/
        )
    target_link_libraries(toml11_decoder PRIVATE toml11)
    if(MSVC)
        target_compile_options(${TEST_NAME} PRIVATE /W3)
    else()
        target_compile_options(toml11_decoder PRIVATE
            $<$<BOOL:${COMPILER_SUPPORTS_WALL}>:                 -Wall                >
            $<$<BOOL:${COMPILER_SUPPORTS_WEXTRA}>:               -Wextra              >
            $<$<BOOL:${COMPILER_SUPPORTS_WPEDANTIC}>:            -Wpedantic           >
            $<$<BOOL:${COMPILER_SUPPORTS_WSIGN_CONVERSION}>:     -Wsign-conversion    >
            $<$<BOOL:${COMPILER_SUPPORTS_WCONVERSION}>:          -Wconversion         >
            $<$<BOOL:${COMPILER_SUPPORTS_WDUPLICATED_COND}>:     -Wduplicated-cond    >
            $<$<BOOL:${COMPILER_SUPPORTS_WDUPLICATED_BRANCHES}>: -Wduplicated-branches>
            $<$<BOOL:${COMPILER_SUPPORTS_WLOGICAL_OP}>:          -Wlogical-op         >
            $<$<BOOL:${COMPILER_SUPPORTS_WDOUBLE_PROMOTION}>:    -Wdouble-promotion   >
            $<$<BOOL:${COMPILER_SUPPORTS_WRANGE_LOOP_ANALYSIS}>: -Wrange-loop-analysis>
            $<$<BOOL:${COMPILER_SUPPORTS_WUNDEF}>:               -Wundef              >
            $<$<BOOL:${COMPILER_SUPPORTS_WSHADOW}>:              -Wshadow             >
            $<$<BOOL:${TOML11_TEST_WITH_ASAN}>:                  -fsanitize=address   >
            $<$<BOOL:${TOML11_TEST_WITH_UBSAN}>:                 -fsanitize=undefined >
            )
        target_link_options(toml11_decoder PRIVATE
            $<$<BOOL:${TOML11_TEST_WITH_ASAN}>:                  -fsanitize=address   >
            $<$<BOOL:${TOML11_TEST_WITH_UBSAN}>:                 -fsanitize=undefined >
            )
    endif()

    add_executable(toml11_decoder_v1_1_0 to_json.cpp)
    target_include_directories(toml11_decoder_v1_1_0
        PRIVATE ${PROJECT_SOURCE_DIR}/tests/extlib/doctest/doctest/
        PRIVATE ${PROJECT_SOURCE_DIR}/tests/extlib/json/include/
        )
    target_compile_definitions(toml11_decoder_v1_1_0 PRIVATE -DTOML11_TO_JSON_USE_V1_1_0)
    target_link_libraries(toml11_decoder_v1_1_0 PRIVATE toml11)
    if(MSVC)
        target_compile_options(${TEST_NAME} PRIVATE /W3)
    else()
        target_compile_options(toml11_decoder_v1_1_0 PRIVATE
            $<$<BOOL:${COMPILER_SUPPORTS_WALL}>:                 -Wall                >
            $<$<BOOL:${COMPILER_SUPPORTS_WEXTRA}>:               -Wextra              >
            $<$<BOOL:${COMPILER_SUPPORTS_WPEDANTIC}>:            -Wpedantic           >
            $<$<BOOL:${COMPILER_SUPPORTS_WSIGN_CONVERSION}>:     -Wsign-conversion    >
            $<$<BOOL:${COMPILER_SUPPORTS_WCONVERSION}>:          -Wconversion         >
            $<$<BOOL:${COMPILER_SUPPORTS_WDUPLICATED_COND}>:     -Wduplicated-cond    >
            $<$<BOOL:${COMPILER_SUPPORTS_WDUPLICATED_BRANCHES}>: -Wduplicated-branches>
            $<$<BOOL:${COMPILER_SUPPORTS_WLOGICAL_OP}>:          -Wlogical-op         >
            $<$<BOOL:${COMPILER_SUPPORTS_WDOUBLE_PROMOTION}>:    -Wdouble-promotion   >
            $<$<BOOL:${COMPILER_SUPPORTS_WRANGE_LOOP_ANALYSIS}>: -Wrange-loop-analysis>
            $<$<BOOL:${COMPILER_SUPPORTS_WUNDEF}>:               -Wundef              >
            $<$<BOOL:${COMPILER_SUPPORTS_WSHADOW}>:              -Wshadow             >
            $<$<BOOL:${TOML11_TEST_WITH_ASAN}>:                  -fsanitize=address   >
            $<$<BOOL:${TOML11_TEST_WITH_UBSAN}>:                 -fsanitize=undefined >
            )
        target_link_options(toml11_decoder_v1_1_0 PRIVATE
            $<$<BOOL:${TOML11_TEST_WITH_ASAN}>:                  -fsanitize=address   >
            $<$<BOOL:${TOML11_TEST_WITH_UBSAN}>:                 -fsanitize=undefined >
            )
    endif()

    add_executable(toml11_encoder to_toml.cpp)
    target_include_directories(toml11_encoder
        PRIVATE ${PROJECT_SOURCE_DIR}/tests/extlib/doctest/doctest/
        PRIVATE ${PROJECT_SOURCE_DIR}/tests/extlib/json/include/
        )
    target_link_libraries(toml11_encoder PRIVATE toml11)
    if(MSVC)
        target_compile_options(${TEST_NAME} PRIVATE /W3)
    else()
        target_compile_options(toml11_encoder PRIVATE
            $<$<BOOL:${COMPILER_SUPPORTS_WALL}>:                 -Wall                >
            $<$<BOOL:${COMPILER_SUPPORTS_WEXTRA}>:               -Wextra              >
            $<$<BOOL:${COMPILER_SUPPORTS_WPEDANTIC}>:            -Wpedantic           >
            $<$<BOOL:${COMPILER_SUPPORTS_WSIGN_CONVERSION}>:     -Wsign-conversion    >
            $<$<BOOL:${COMPILER_SUPPORTS_WCONVERSION}>:          -Wconversion         >
            $<$<BOOL:${COMPILER_SUPPORTS_WDUPLICATED_COND}>:     -Wduplicated-cond    >
            $<$<BOOL:${COMPILER_SUPPORTS_WDUPLICATED_BRANCHES}>: -Wduplicated-branches>
            $<$<BOOL:${COMPILER_SUPPORTS_WLOGICAL_OP}>:          -Wlogical-op         >
            $<$<BOOL:${COMPILER_SUPPORTS_WDOUBLE_PROMOTION}>:    -Wdouble-promotion   >
            $<$<BOOL:${COMPILER_SUPPORTS_WRANGE_LOOP_ANALYSIS}>: -Wrange-loop-analysis>
            $<$<BOOL:${COMPILER_SUPPORTS_WUNDEF}>:               -Wundef              >
            $<$<BOOL:${COMPILER_SUPPORTS_WSHADOW}>:              -Wshadow             >
            $<$<BOOL:${TOML11_TEST_WITH_ASAN}>:                  -fsanitize=address   >
            $<$<BOOL:${TOML11_TEST_WITH_UBSAN}>:                 -fsanitize=undefined >
            )
        target_link_options(toml11_encoder PRIVATE
            $<$<BOOL:${TOML11_TEST_WITH_ASAN}>:                  -fsanitize=address   >
            $<$<BOOL:${TOML11_TEST_WITH_UBSAN}>:                 -fsanitize=undefined >
            )
    endif()
endif(TOML11_BUILD_TOML_TESTS)
