project(lua)
cmake_minimum_required(VERSION 3.22)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Boost 1.38 REQUIRED
    COMPONENTS filesystem iostreams program_options regex system)


include_directories(
    src/
    third_party/avro-cpp-1.11.0/include/
    ${Boost_INCLUDE_DIRS}
)
add_subdirectory(third_party/snappy-1.1.9)
add_subdirectory(third_party/avro-cpp-1.11.0)



set(LUA_EXTERNAL_LIB_SRC
    external/lavro.cc
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
    snappy
    avrocpp_s
    lua_external
)

add_library(lua_s STATIC ${LUA_LIB_SRC})

add_executable(doctor src/lua.c)
target_link_libraries(doctor lua_s lua_external ${LUA_LINK_LIBS} ${Boost_LIBRARIES})

add_executable(luac src/luac.c)
target_link_libraries(luac lua_s ${LUA_LINK_LIBS})
