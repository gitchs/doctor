/*
 * lmissing.c
 * Copyright (C) 2022 archlinux <archlinux@arch>
 *
 * Distributed under terms of the MIT license.
 */
#include <errno.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "lauxlib.h"
#include "lmissing.h"
#include "lua.h"
#include "lualib.h"

static int lmissing_getcwd(lua_State* L) {
    const char* const cwd = getcwd(NULL, 0);
    lua_pushstring(L, cwd);
    return 1;
}

static int lmissing_getpid(lua_State* L) {
  lua_pushinteger(L, getpid());
  return 1;
}

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

static int lmissing_getrusage(lua_State* L) {
  struct rusage usage;
  int who = RUSAGE_SELF;
  if (lua_isinteger(L, 1)) {
    who = lua_tointeger(L, 1);
  }
  bzero(&usage, sizeof(struct rusage));
  getrusage(who, &usage);
  lua_newtable(L);
  lua_pushnumber(L, (float)usage.ru_utime.tv_sec * 1000 +
                        (float)(usage.ru_utime.tv_usec) / 1000);
  lua_setfield(L, -2, "ru_utime");
  lua_pushnumber(L, (float)usage.ru_stime.tv_sec * 1000 +
                        (float)(usage.ru_stime.tv_usec) / 1000);
  lua_setfield(L, -2, "ru_stime");

  lua_pushinteger(L, usage.ru_maxrss);
  lua_setfield(L, -2, "ru_maxrss");
  lua_pushinteger(L, usage.ru_ixrss);
  lua_setfield(L, -2, "ru_ixrss");
  lua_pushinteger(L, usage.ru_idrss);
  lua_setfield(L, -2, "ru_idrss");
  lua_pushinteger(L, usage.ru_isrss);
  lua_setfield(L, -2, "ru_isrss");
  lua_pushinteger(L, usage.ru_minflt);
  lua_setfield(L, -2, "ru_minflt");
  lua_pushinteger(L, usage.ru_majflt);
  lua_setfield(L, -2, "ru_majflt");
  lua_pushinteger(L, usage.ru_nswap);
  lua_setfield(L, -2, "ru_nswap");
  lua_pushinteger(L, usage.ru_inblock);
  lua_setfield(L, -2, "ru_inblock");
  lua_pushinteger(L, usage.ru_oublock);
  lua_setfield(L, -2, "ru_oublock");
  lua_pushinteger(L, usage.ru_msgsnd);
  lua_setfield(L, -2, "ru_msgsnd");
  lua_pushinteger(L, usage.ru_msgrcv);
  lua_setfield(L, -2, "ru_msgrcv");
  lua_pushinteger(L, usage.ru_nsignals);
  lua_setfield(L, -2, "ru_nsignals");
  lua_pushinteger(L, usage.ru_nvcsw);
  lua_setfield(L, -2, "ru_nvcsw");
  lua_pushinteger(L, usage.ru_nivcsw);
  lua_setfield(L, -2, "ru_nivcsw");

  return 1;
}

const static luaL_Reg libs[] = {{"mkdir", lmissing_mkdir},
                                {"getrusage", lmissing_getrusage},
                                {"strptime", lmissing_strptime},
                                {"getpid", lmissing_getpid},
                                {"getcwd", lmissing_getpwd},
                                {NULL, NULL}};

int luaopen_missing(lua_State* L) {
  luaL_newlib(L, libs);
  lua_pushinteger(L, RUSAGE_SELF);
  lua_setfield(L, -2, "RUSAGE_SELF");
  lua_pushinteger(L, RUSAGE_CHILDREN);
  lua_setfield(L, -2, "RUSAGE_CHILDREN");
#ifdef RUSAGE_THREAD
  // only works on Linux(since 2.6.26)
  // https://linux.die.net/man/2/getrusage
  lua_pushinteger(L, RUSAGE_THREAD);
  lua_setfield(L, -2, "RUSAGE_THREAD");
#endif  // RUSAGE_THREAD
  return 1;
}
