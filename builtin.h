/*
 * builtin.h
 * Copyright (C) 2022 tinyproxy <tinyproxy@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __BUILTIN_H__
#define __BUILTIN_H__


#include "lua.h"

LUAMOD_API int lexternal_searchers_builtin(lua_State* L);

#define LUA_ENABLE_BUILTIN(L) do { \
  int sidx = lua_gettop(L); \
  lua_getglobal(L, "package"); \
  lua_getfield(L, -1, "searchers"); \
  lua_len(L, -1); \
  int tlen = lua_tointeger(L, -1); \
  lua_pop(L, 1); /* pop tlen */ \
  lua_pushcfunction(L, lexternal_searchers_builtin); \
  lua_seti(L, -2, tlen+1); \
  lua_settop(L, sidx); \
} while(0)


#endif // __BUILTIN_H__

