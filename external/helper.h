/*
 * helper.h
 * Copyright (C) 2022 tinyproxy <tinyproxy@hschens-MacBook-Pro.local>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __HELPER_H__
#define __HELPER_H__
#include <assert.h>
#include "lua.h"

#define DEFINE_LUA_CONST_INTEGER(L, name, i) \
  do {                                       \
    assert(lua_istable(L, -1));              \
    lua_pushinteger(L, i);                   \
    lua_setfield(L, -2, name);               \
  } while (0)

#define DEFINE_LUA_CONST_STRING(L, name, s) \
  do {                                      \
    assert(lua_istable(L, -1));             \
    lua_pushstring(L, s);                   \
    lua_setfield(L, -2, name);              \
  } while (0)

#endif  // __HELPER_H__
