project(lua)
cmake_minimum_required(VERSION 3.22)


SET(LUA_LIB_SRC 
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
set(LUA_LINK_LIBS m readline dl)

add_library(lua STATIC ${LUA_LIB_SRC})

add_executable(luashell src/lua.c)
target_link_libraries(luashell lua ${LUA_LINK_LIBS})
set_target_properties(luashell luashell PROPERTIES OUTPUT_NAME lua)

add_executable(luac src/luac.c)
target_link_libraries(luac lua ${LUA_LINK_LIBS})