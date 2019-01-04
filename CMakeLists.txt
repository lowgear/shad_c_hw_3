cmake_minimum_required(VERSION 3.13)
project(shad_c_hw_3 C)

set(CMAKE_C_STANDARD 99)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=memory -fPIE -fno-omit-frame-pointer -g -O1")

add_executable(shad_c_hw_3
        main.c
        evaluation.c
        builtins.c
        lisp_io.c
        models.c
        utils/iotools.c
        utils/strtools.c)