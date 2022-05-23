/*
 * lmissing.c
 * Copyright (C) 2022 archlinux <archlinux@arch>
 *
 * Distributed under terms of the MIT license.
 */
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

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

static int lmissing_strptime(lua_State* L) {
  size_t dtlen = 0;
  size_t flen = 0;
  const char* const dt = lua_tolstring(L, 1, &dtlen);
  const char* const dtformat = lua_tolstring(L, 2, &flen);
  if (dt == NULL || dtformat == NULL) {
    lua_pushnil(L);
    return 1;
  }
  struct tm t;
  bzero(&t, sizeof(struct tm));
  strptime(dt, dtformat, &t);
  time_t ts = mktime(&t);
  lua_pushinteger(L, ts);
  return 1;
}

const static luaL_Reg libs[] = {{"mkdir", lmissing_mkdir},
                                {"strptime", lmissing_strptime},
                                {NULL, NULL}};

int luaopen_missing(lua_State* L) {
  luaL_newlib(L, libs);
  return 1;
}
