/*
 * lavro.h
 * Copyright (C) 2022 tinyproxy <tinyproxy@hschens-MacBook-Pro.local>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __LAVRO_H__
#define __LAVRO_H__

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

LUAMOD_API int luaopen_avro(lua_State* L);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // __LAVRO_H__
