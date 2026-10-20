cmake_minimum_required(VERSION 3.20)
project(pwgen VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(PWGEN_BUILD_TESTS "Build unit tests" ON)

add_library(pwgen_warnings INTERFACE)
target_compile_options(pwgen_warnings INTERFACE -Wall -Wextra -Wpedantic -Werror)

add_library(pwgen_core STATIC src/generator.cpp src/options.cpp)
target_include_directories(pwgen_core PUBLIC src)
target_link_libraries(pwgen_core PRIVATE pwgen_warnings)

add_executable(pwgen src/main.cpp)
target_link_libraries(pwgen PRIVATE pwgen_core pwgen_warnings)

if(PWGEN_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.12
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(doctest)

    enable_testing()
    add_executable(generator_tests tests/generator_tests.cpp)
    target_link_libraries(generator_tests PRIVATE pwgen_core doctest::doctest)
    target_compile_options(generator_tests PRIVATE -Wall -Wextra)
    add_test(NAME generator_tests COMMAND generator_tests)
endif()
