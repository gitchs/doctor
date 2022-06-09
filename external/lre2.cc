extern "C" {
#include "lre2.h"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}
#include <iostream>
#include <string>
#include <vector>
#include "re2/re2.h"

static int lre2_replace(lua_State* L) {
  if (lua_isnil(L, 1) || lua_isnil(L, 2) || lua_isnil(L, 3)) {
    lua_pushnil(L);
    return 1;
  }
  std::string s(lua_tostring(L, 1));
  const char* const re = lua_tostring(L, 2);
  const char* const rewrite = lua_tostring(L, 3);
  re2::RE2::GlobalReplace(&s, re, rewrite);
  lua_pushstring(L, s.c_str());
  return 1;
}

static int lre2_match(lua_State* L) {
  std::string content(lua_tostring(L, 1));
  re2::StringPiece sp(content);
  re2::RE2 re(lua_tostring(L, 2));

  std::string val;

  int index = 1;
  lua_newtable(L);
  while (re2::RE2::FindAndConsume(&sp, re, &val)) {
    lua_pushinteger(L, index++);
    lua_pushstring(L, val.c_str());
    lua_settable(L, -3);
  }
  return 1;
}

static luaL_Reg libs[] = {{"replace", lre2_replace},
                          {"match", lre2_match},
                          {NULL, NULL}};

extern "C" {
LUAMOD_API int luaopen_re2(lua_State* L) {
  luaL_newlib(L, libs);
  return 1;
}
}
