/*
 * lerrors.c
 * Copyright (C) 2022 archlinux <archlinux@arch>
 *
 * Distributed under terms of the MIT license.
 */

#include "lerrors.h"
#include <errno.h>
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

#define ldefine_error(L_, name_, errno_) \
  do {                                   \
    lua_pushinteger((L_), (errno_));     \
    lua_setfield((L_), -2, (name_));     \
  } while (0)

static int lerrors_errno(lua_State* L) {
  lua_pushinteger(L, errno);
  return 1;
}

const static luaL_Reg libs[] = {{"errno", lerrors_errno}, {NULL, NULL}};

int luaopen_errors(lua_State* L) {
  lua_newtable(L);

  // FROM https://linux.die.net/man/2/mkdir
  ldefine_error(L, "EACCES", EACCES);
  ldefine_error(L, "EDQUOT", EDQUOT);
  ldefine_error(L, "EEXIST", EEXIST);
  ldefine_error(L, "EFAULT", EFAULT);
  ldefine_error(L, "ELOOP", ELOOP);
  ldefine_error(L, "EMLINK", EMLINK);
  ldefine_error(L, "ENAMETOOLONG", ENAMETOOLONG);
  ldefine_error(L, "ENOENT", ENOENT);
  ldefine_error(L, "ENOMEM", ENOMEM);
  ldefine_error(L, "ENOSPC", ENOSPC);
  ldefine_error(L, "ENOTDIR", ENOTDIR);
  ldefine_error(L, "EPERM", EPERM);
  ldefine_error(L, "EROFS", EROFS);
  // FROM https://linux.die.net/man/2/mkdir
  return 1;
}
