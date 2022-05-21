/*
 * limpala.h
 * Copyright (C) 2022 archlinux <archlinux@arch>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __LIMPALA_H__
#define __LIMPALA_H__

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

LUAMOD_API int luaopen_impala(lua_State* L);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // __LIMPALA_H__
