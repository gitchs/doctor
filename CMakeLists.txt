project(lua)
cmake_minimum_required(VERSION 3.22)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SNAPPY_INSTALL OFF)


find_package(Boost 1.38 REQUIRED
    COMPONENTS filesystem iostreams program_options regex system)

set(OPENSSL_USE_STATIC_LIBS ON)
find_package(OpenSSL REQUIRED)


include_directories(
    src/
    third_party/avro-cpp-1.11.0/include/
    third_party/thrift-0.16.0/lib/cpp/src/
    third_party/impala-codegen/
    third_party/cjson/
    third_party/luasql/
    build/third_party/thrift-0.16.0/
    third_party/mariadb-connector-c-3.3.1-src/include
    third_party/mariadb-connector-c-3.3.1-src/copygen
    ${OPENSSL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)
add_subdirectory(third_party/snappy-1.1.9)
add_subdirectory(third_party/avro-cpp-1.11.0)
add_subdirectory(third_party/thrift-0.16.0)
add_subdirectory(third_party/impala-codegen)
add_subdirectory(third_party/cjson)
add_subdirectory(third_party/mariadb-connector-c-3.3.1-src)
add_subdirectory(third_party/luasql)




set(LUA_EXTERNAL_LIB_SRC
    external/lavro.cc
    external/limpala.cc
    external/lmissing.c
    external/lerrors.c
)
add_library(lua_external STATIC ${LUA_EXTERNAL_LIB_SRC})



set(LUA_LIB_SRC
  src/lapi.c
  src/lauxlib.c
  src/lbaselib.c
  src/lcode.c
  src/lcorolib.c
  src/lctype.c
  src/ldblib.c
  src/ldebug.c
  src/ldo.c
  src/ldump.c
  src/lfunc.c
  src/lgc.c
  src/linit.c
  src/liolib.c
  src/llex.c
  src/lmathlib.c
  src/lmem.c
  src/loadlib.c
  src/lobject.c
  src/lopcodes.c
  src/loslib.c
  src/lparser.c
  src/lstate.c
  src/lstring.c
  src/lstrlib.c
  src/ltable.c
  src/ltablib.c
  src/ltm.c
  src/lundump.c
  src/lutf8lib.c
  src/lvm.c
  src/lzio.c
)

add_compile_definitions(
  LUA_USE_READLINE=ON
  LUA_USE_LINUX=ON
)
set(LUA_LINK_LIBS
    m
    readline
    dl
    cjson
)
set(DOCTOR_LINK_LIBS
    ${LUA_LINK_LIBS}
    impala_thrift
    thriftz
    avrocpp_s
    snappy
    lua_external
    ${OPENSSL_LIBRARIES}
)

add_library(lua_s STATIC ${LUA_LIB_SRC})

add_executable(doctor src/lua.c)
target_link_libraries(doctor lua_s lua_external ${DOCTOR_LINK_LIBS} LUASQL_SQLITE3 sqlite3 ${Boost_LIBRARIES})

add_executable(luac src/luac.c)
target_link_libraries(luac lua_s ${LUA_LINK_LIBS})


install(TARGETS
    doctor
)


if (DEBUG_VARIABLES)

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()

print_all_variables()
endif(DEBUG_VARIABLES)

