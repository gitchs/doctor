/*
 * lmissing.c
 * Copyright (C) 2022 archlinux <archlinux@arch>
 *
 * Distributed under terms of the MIT license.
 */
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "lauxlib.h"
#include "lmissing.h"
#include "lua.h"
#include "lualib.h"

static int lmissing_mkdir(lua_State* L) {
  const char* const filepath = lua_tostring(L, 1);
  const static mode_t mode = 0755;
  lua_pushinteger(L, mkdir(filepath, mode));
  return 1;
}

const static luaL_Reg libs[] = {{"mkdir", lmissing_mkdir}, {NULL, NULL}};

int luaopen_missing(lua_State* L) {
  luaL_newlib(L, libs);
  return 1;
}
