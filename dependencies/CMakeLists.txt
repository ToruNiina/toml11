include(FetchContent)

if (toml11_BUILD_TEST)
    FetchContent_Declare(toml
                         GIT_REPOSITORY https://github.com/toml-lang/toml
                         GIT_TAG v0.5.0)

    FetchContent_MakeAvailable(toml)
endif ()
