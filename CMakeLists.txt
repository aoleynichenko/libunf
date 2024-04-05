cmake_minimum_required(VERSION 3.14)
project(run_tests C)
project(run_tests Fortran)
project(run_tests CXX)
set(CMAKE_C_STANDARD 11)

find_library(GFORTRAN_LIBRARY NAMES gfortran)
message("libgfortran library: ${GFORTRAN_LIBRARY}")

#
# GoogleTest
#
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
        run_tests
        libunf.c
        run_tests.cpp
        gen_tests.f90
)

target_link_libraries(
        run_tests
        -L ${GFORTRAN_LIBRARY}
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(run_tests)
