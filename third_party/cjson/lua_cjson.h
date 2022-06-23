/*
 * lua_cjson.h
 * Copyright (C) 2022 archlinux <archlinux@arch>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __LUA_CJSON_H__
#define __LUA_CJSON_H__

#include "lua.h"

const static char* const LUA_ARRAY_METATABLE = "LuaArray";

int luaopen_cjson(lua_State* L);

#endif // __LUA_CJSON_H__
