cmake_minimum_required (VERSION 3.10)

project(sqlite3)
set(CMAKE_C_STANDARD 99)


set(SQLITE3_SRCS
    sqlite3.c
)

add_library(sqlite3_a STATIC ${SQLITE3_SRCS})
target_compile_definitions(sqlite3_a PRIVATE 
    SQLITE_CORE
    SQLITE_ENABLE_RTREE
    SQLITE_OMIT_LOAD_EXTENSION
    SQLITE_ENABLE_FTS5)

