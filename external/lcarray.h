/*
 * carray.h
 * Copyright (C) 2022 tinyproxy <tinyproxy@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __LCARRAY_H__
#define __LCARRAY_H__
#include <lua.h>
#include <stddef.h>

const static char* const LUA_STRING_ARRAY_TYPE = "StringArray*";

struct StringArray {
  char** data;
  size_t* item_sizes;
  size_t data_size;
};

LUAMOD_API int luaopen_carray(lua_State* L);

#endif  // __LCARRAY_H__
