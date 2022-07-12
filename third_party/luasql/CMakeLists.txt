cmake_minimum_required (VERSION 3.10)

# projectname is the same as the main-executable
project(luasql)
set(CMAKE_C_STANDARD 99)


set(LUASQL_SRCS
    ls_sqlite3.c
    luasql.c
    ls_mysql.c
)
add_compile_definitions(LUASQL_VERSION_NUMBER="2.6.0")

add_library(LUASQL STATIC ${LUASQL_SRCS})
target_link_libraries(LUASQL sqlite3_a libmariadb_s ${OPENSSL_LIBRARIES})

