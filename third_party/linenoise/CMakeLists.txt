cmake_minimum_required (VERSION 3.10)

project(linenoise)
set(CMAKE_C_STANDARD 99)


set(LINENOISE_SRC
    linenoise.c
)

add_library(linenoise STATIC ${LINENOISE_SRC})

