/*
 * carray.c
 * Copyright (C) 2022 tinyproxy <tinyproxy@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "lcarray.h"
#include <stdlib.h>
#include <string.h>
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

static int lcarry_string_array_gc(lua_State* L) {
  struct StringArray* array = lua_touserdata(L, 1);
  if (array->data_size > 0 && array->data != NULL) {
    for (size_t i = 0; i < array->data_size; i++) {
      if (array->data[i] == NULL) {
        break;
      }
      free(array->data[i]);
    }
  }
  if (array->item_sizes != NULL) {
    free(array->item_sizes);
  }
  if (array->data != NULL) {
    free(array->data);
  }
  return 0;
}

static int lcarray_string_array_size(lua_State* L) {
  struct StringArray* array = lua_touserdata(L, 1);
  lua_pushinteger(L, array->data_size);
  return 1;
}

static int lcarray_string_array_set(lua_State* L) {
  if (!lua_isinteger(L, 2)) {
    lua_pushboolean(L, 0);
    return 1;
  }
  struct StringArray* array = lua_touserdata(L, 1);
  size_t index = lua_tointeger(L, 2);
  if (index > array->data_size || index == 0) {
    // lua API index start from 1
    lua_pushboolean(L, 0);
    return 1;
  }
  index--;
  const char* value = lua_tolstring(L, 3, array->item_sizes + index);
  array->data[index] = malloc(sizeof(char) * (array->item_sizes[index] + 1));
  strncpy(array->data[index], value, array->item_sizes[index]);
  lua_pushboolean(L, 1);
  return 1;
}

static int lcarray_string_array_at(lua_State* L) {
  struct StringArray* array = lua_touserdata(L, 1);
  size_t index = lua_tointeger(L, 2);
  if (index == 0 || index > array->data_size) {
    lua_pushnil(L);
    return 1;
  }
  index--;
  lua_pushlstring(L, array->data[index], array->item_sizes[index]);
  return 1;
}

static int lcarray_string_array_raw(lua_State* L) {
  struct StringArray* array = lua_touserdata(L, 1);
  lua_pushlightuserdata(L, array->data);
  return 1;
}

static int lcarray_string_array_item_sizes(lua_State* L) {
  lua_newtable(L);
  struct StringArray* array = lua_touserdata(L, 1);
  for (size_t i = 0; i < array->data_size; i++) {
    lua_pushinteger(L, array->item_sizes[i]);
    lua_seti(L, -2, i + 1);
  }
  return 1;
}

static int lcarray_string_array_new(lua_State* L) {
  if (!lua_isinteger(L, 1)) {
    lua_pushnil(L);
    return 1;
  }
  struct StringArray* retval =
      (struct StringArray*)lua_newuserdata(L, sizeof(struct StringArray));
  luaL_setmetatable(L, LUA_STRING_ARRAY_TYPE);
  retval->data_size = lua_tointeger(L, 1);
  retval->data = (char**)malloc(sizeof(char*) * retval->data_size);
  memset(retval->data, 0, sizeof(char*) * retval->data_size);
  retval->item_sizes = (size_t*)malloc(sizeof(size_t) * retval->data_size);
  memset(retval->item_sizes, 0, sizeof(size_t) * retval->data_size);
  return 1;
}

const static luaL_Reg string_array_meta_libs[] = {
    {"__gc", lcarry_string_array_gc},
    {NULL, NULL}};

const static luaL_Reg string_array_libs[] = {
    {"size", lcarray_string_array_size},
    {"set", lcarray_string_array_set},
    {"at", lcarray_string_array_at},
    {"raw", lcarray_string_array_raw},
    {"item_sizes", lcarray_string_array_item_sizes},
    {NULL, NULL},
};

const static luaL_Reg libs[] = {{"StringArray", lcarray_string_array_new},
                                {NULL, NULL}};

LUAMOD_API int luaopen_carray(lua_State* L) {
  luaL_newlib(L, libs);

  if (luaL_newmetatable(L, LUA_STRING_ARRAY_TYPE) > 0) {
    luaL_setfuncs(L, string_array_meta_libs, 0);
    luaL_newlib(L, string_array_libs);
    lua_setfield(L, -2, "__index");
  }
  lua_pop(L, 1);
  return 1;
}
