/*
 * lmissing.c
 * Copyright (C) 2022 archlinux <archlinux@arch>
 *
 * Distributed under terms of the MIT license.
 */
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>

#include "helper.h"
#include "lauxlib.h"
#include "lmissing.h"
#include "lua.h"
#include "lualib.h"

#define __USE_XOPEN
#define _GNU_SOURCE

static int lmissing_gettimeofday(lua_State* L) {
  // int gettimeofday(struct timeval *tp, void *tzp);
  struct timeval tv;
  gettimeofday(&tv, NULL);  // The gettimeofday() function returns 0 and no
                            // value is reserved to indicate an error.
  lua_pushinteger(L, tv.tv_sec);
  lua_pushinteger(L, tv.tv_usec);
  return 2;
}

static int lmissing_stat(lua_State* L) {
  if (!lua_isstring(L, 1)) {
    lua_pushnil(L);
    return 1;
  }
  lua_newtable(L);
  const char* const filename = lua_tostring(L, 1);
  struct stat s;
  bzero(&s, sizeof(s));
  int rc = stat(filename, &s);
  if (rc != 0) {
    lua_pushinteger(L, errno);
    lua_setfield(L, -2, "errno");
    lua_pushstring(L, strerror(errno));
    lua_setfield(L, -2, "error");
    return 1;
  }
  lua_pushinteger(L, s.st_dev);
  lua_setfield(L, -2, "st_dev");
  lua_pushinteger(L, s.st_mode);
  lua_setfield(L, -2, "st_mode");
  lua_pushinteger(L, s.st_nlink);
  lua_setfield(L, -2, "st_nlink");
  lua_pushinteger(L, s.st_uid);
  lua_setfield(L, -2, "st_uid");
  lua_pushinteger(L, s.st_gid);
  lua_setfield(L, -2, "st_gid");
  lua_pushinteger(L, s.st_rdev);
  lua_setfield(L, -2, "st_rdev");
  lua_pushinteger(L, s.st_blocks);
  lua_setfield(L, -2, "st_blocks");
  lua_pushinteger(L, s.st_blksize);
  lua_setfield(L, -2, "st_blksize");

#ifdef __linux__
  lua_pushinteger(L, s.st_atime);
  lua_setfield(L, -2, "st_atime");
  lua_pushinteger(L, s.st_ctime);
  lua_setfield(L, -2, "st_ctime");
  lua_pushinteger(L, s.st_mtime);
  lua_setfield(L, -2, "st_mtime");
#endif  // __linux__

#ifdef __APPLE__
  lua_pushinteger(L, s.st_atimespec.tv_sec);
  lua_setfield(L, -2, "st_atime");
  lua_pushinteger(L, s.st_ctimespec.tv_sec);
  lua_setfield(L, -2, "st_ctime");
  lua_pushinteger(L, s.st_mtimespec.tv_sec);
  lua_setfield(L, -2, "st_mtime");
  lua_pushinteger(L, s.st_birthtimespec.tv_sec);
  lua_setfield(L, -2, "st_birthtime");
  lua_pushinteger(L, s.st_flags);
  lua_setfield(L, -2, "st_flags");
  lua_pushinteger(L, s.st_gen);
  lua_setfield(L, -2, "st_gen");
#endif  // __APPLE__

  lua_pushinteger(L, s.st_size);
  lua_setfield(L, -2, "st_size");
  return 1;
}

static int lmissing_readdir(lua_State* L) {
  if (!lua_isstring(L, 1)) {
    goto FAILED;
  }
  const char* const dirname = lua_tostring(L, 1);
  DIR* dir = opendir(dirname);
  if (dir == NULL) {
    goto FAILED;
  }

  lua_pushboolean(L, 1);
  lua_newtable(L);
  int i = 1;

  while (1) {
    struct dirent* f = readdir(dir);
    if (f == NULL) {
      break;
    }
    if (strcmp(f->d_name, ".") == 0 || strcmp(f->d_name, "..") == 0) {
      // ignore . and ..
      continue;
    }
    lua_pushinteger(L, i++);
    lua_pushstring(L, f->d_name);
    lua_settable(L, -3);
  }
  closedir(dir);
  return 2;
FAILED:
  lua_pushboolean(L, 0);
  lua_pushnil(L);
  return 2;
}

static int lmissing_day2date(lua_State* L) {
  if (!lua_isinteger(L, 1)) {
    lua_pushnil(L);
    return 1;
  }
  int day = lua_tointeger(L, 1);
  time_t ts = day * 86400 + 8 * 3600;
  lua_pushinteger(L, ts);
  return 1;
}

static int lmissing_isatty(lua_State* L) {
  int fd = lua_tonumber(L, 1);
  lua_pushboolean(L, isatty(fd));
  return 1;
}

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

static int lmissing_uname(lua_State* L) {
  struct utsname name;
  bzero(&name, sizeof(name));
  int rc = uname(&name);
  lua_newtable(L);
  if (rc == -1) {
    lua_pushinteger(L, errno);
    lua_setfield(L, -2, "errno");
    lua_pushstring(L, strerror(errno));
    lua_setfield(L, -2, "error");
    return 1;
  }
  lua_pushstring(L, name.sysname);
  lua_setfield(L, -2, "sysname");
  lua_pushstring(L, name.nodename);
  lua_setfield(L, -2, "nodename");
  lua_pushstring(L, name.machine);
  lua_setfield(L, -2, "machine");
  lua_pushstring(L, name.version);
  lua_setfield(L, -2, "version");
  lua_pushstring(L, name.release);
  lua_setfield(L, -2, "release");
  return 1;
}

const static luaL_Reg libs[] = {
    {"mkdir", lmissing_mkdir},         {"stat", lmissing_stat},
    {"getrusage", lmissing_getrusage}, {"strptime", lmissing_strptime},
    {"getpid", lmissing_getpid},       {"getcwd", lmissing_getcwd},
    {"isatty", lmissing_isatty},       {"readdir", lmissing_readdir},
    {"day2date", lmissing_day2date},   {"gettimeofday", lmissing_gettimeofday},
    {"uname", lmissing_uname},         {NULL, NULL}};

int luaopen_missing(lua_State* L) {
  luaL_newlib(L, libs);

  DEFINE_LUA_CONST_INTEGER(L, "S_IFMT", S_IFMT);
  DEFINE_LUA_CONST_INTEGER(L, "S_IFIFO", S_IFIFO);
  DEFINE_LUA_CONST_INTEGER(L, "S_IFCHR", S_IFCHR);
  DEFINE_LUA_CONST_INTEGER(L, "S_IFDIR", S_IFDIR);
  DEFINE_LUA_CONST_INTEGER(L, "S_IFBLK", S_IFBLK);
  DEFINE_LUA_CONST_INTEGER(L, "S_IFREG", S_IFREG);
  DEFINE_LUA_CONST_INTEGER(L, "S_IFREG", S_IFREG);
  DEFINE_LUA_CONST_INTEGER(L, "S_IFLNK", S_IFLNK);
  DEFINE_LUA_CONST_INTEGER(L, "S_IFSOCK", S_IFSOCK);

#ifdef S_IFWHT
  // only works on macos
  DEFINE_LUA_CONST_INTEGER(L, "S_IFWHT", S_IFWHT);
#endif

  DEFINE_LUA_CONST_INTEGER(L, "RUSAGE_SELF", RUSAGE_SELF);
  DEFINE_LUA_CONST_INTEGER(L, "RUSAGE_CHILDREN", RUSAGE_CHILDREN);
#ifdef RUSAGE_THREAD
  // only works on Linux(since 2.6.26)
  // https://linux.die.net/man/2/getrusage
  DEFINE_LUA_CONST_INTEGER(L, "RUSAGE_THREAD", RUSAGE_THREAD);
#endif  // RUSAGE_THREAD
  return 1;
}
