/*
 * lerrors.h
 * Copyright (C) 2022 archlinux <archlinux@arch>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __LERRORS_H__
#define __LERRORS_H__

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

LUAMOD_API int luaopen_errors(lua_State* L);

#ifdef __cplusplus
}

#endif  // __cplusplus
#endif  // __LERRORS_H__
