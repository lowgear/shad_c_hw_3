cmake_minimum_required(VERSION 3.13)
project(shad_c_hw_3 C CXX)

set(CMAKE_C_FLAGS "-fprofile-arcs -ftest-coverage ${CMAKE_C_FLAGS} -std=c11")
set(CMAKE_CXX_FLAGS "-fprofile-arcs -ftest-coverage ${CMAKE_CXX_FLAGS} -std=c++17 -g")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(GTEST googletest-release-1.8.1/googletest)

include_directories(${GTEST} ${GTEST}/include .)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(shad_c_hw_3
        main.c
        evaluation.c
        builtins.c
        lisp_io.c
        models.c
        utils/iotools.c
        utils/strtools.c)

aux_source_directory(tests TEST_SRCS)

add_executable(Check
        test_main.cpp
        ${GTEST}/src/gtest-all.cc
        evaluation.c
        builtins.c
        lisp_io.c
        models.c
        utils/iotools.c
        utils/strtools.c

        ${TEST_SRCS})

target_link_libraries(Check Threads::Threads)
