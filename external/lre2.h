/*
 * LRE2.h
 * Copyright (C) 2022 archlinux <archlinux@arch>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __LRE2_H__
#define __LRE2_H__

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

LUAMOD_API int luaopen_re2(lua_State* L);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // __LRE2_H__
