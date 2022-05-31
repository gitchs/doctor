cmake_minimum_required (VERSION 3.10)

# projectname is the same as the main-executable
project(luasql)
set(CMAKE_C_STANDARD 99)


set(LUASQL_SQLITE3_SRCS
    ls_sqlite3.c
    luasql.c
)
add_compile_definitions(LUASQL_VERSION_NUMBER="2.6.0")

add_library(LUASQL_SQLITE3 STATIC ${LUASQL_SQLITE3_SRCS})
target_link_libraries(LUASQL_SQLITE3 sqlite3)

