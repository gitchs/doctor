/*
 * LLIMITS.h
 * Copyright (C) 2022 archlinux <archlinux@arch>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __LLIMITS_H__
#define __LLIMITS_H__

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

LUAMOD_API int luaopen_limits(lua_State* L);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // __LLIMITS_H__
