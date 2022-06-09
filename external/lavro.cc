/*
 * lavro.c
 * Copyright (C) 2022 tinyproxy <tinyproxy@hschens-MacBook-Pro.local>
 *
 * Distributed under terms of the MIT license.
 */

#include "lavro.h"
#include <cstring>
#include <filesystem>
#include <iostream>

#include "avro/DataFile.hh"
#include "avro/Decoder.hh"
#include "avro/Encoder.hh"
#include "avro/Generic.hh"
#include "avro/GenericDatum.hh"
#include "avro/Stream.hh"

typedef avro::DataFileReader<avro::GenericDatum> AvroReader;

typedef struct {
  AvroReader* delegation;
  avro::GenericDatum* datum;
} LAvroReader;

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

const static char* const LUA_AVROREADER_TYPE = "AvroDataFileReader*";
}

static int lavro_open(lua_State* L) {
  const char* filename = lua_tostring(L, 1);
  assert(filename != NULL);
  bool exists = std::filesystem::exists(filename);
  if (!exists) {
    lua_pushnil(L);
    return 1;
  }
  LAvroReader* reader = (LAvroReader*)lua_newuserdata(L, sizeof(LAvroReader));
  reader->delegation = new AvroReader(filename);
  reader->datum = new avro::GenericDatum(reader->delegation->dataSchema());
  luaL_setmetatable(L, LUA_AVROREADER_TYPE);
  return 1;
}

static int lavro_mm_tostring(lua_State* L) {
  char buffer[128] = {0};
  lua_touserdata(L, 1);
  LAvroReader* reader = (LAvroReader*)lua_touserdata(L, 1);
  sprintf(buffer, "%s: %p", LUA_AVROREADER_TYPE, reader->delegation);
  lua_pushstring(L, buffer);
  return 1;
}

static int lavro_mm_gc(lua_State* L) {
  LAvroReader* reader = (LAvroReader*)lua_touserdata(L, 1);
  delete reader->delegation;
  delete reader->datum;
  return 0;
}

static int lavro_next(lua_State* L) {
  LAvroReader* reader = (LAvroReader*)lua_touserdata(L, 1);
  const auto& schema = reader->delegation->dataSchema();
  bool has_more = false;
  try {
    has_more = reader->delegation->read(*reader->datum);
  } catch (const avro::Exception& e) {
      lua_pushboolean(L, 0);
      lua_pushstring(L, e.what());
      lua_pushnil(L);
      return 3;
  }
  avro::GenericRecord record = reader->datum->value<avro::GenericRecord>();

  // bool has_more = reader->delegation->read(*reader->datum);
  lua_pushboolean(L, has_more);
  if (!has_more) {
    lua_pushnil(L);
    lua_pushnil(L);
    return 3;
  }
  lua_pushnil(L);
  lua_newtable(L);
  const auto& schema_root = schema.root();
  for (size_t i = 0; i < schema_root->names(); i++) {
    const std::string& name = schema_root->nameAt(i);
    const auto& field = record.fieldAt(i);
    switch (field.type()) {
      case avro::AVRO_UNKNOWN:
        lua_pushstring(L, "[[UNKNOWN]]");
        break;
      case avro::AVRO_NULL:
        lua_pushnil(L);
        break;
      case avro::AVRO_STRING: {
        const std::string& value = field.value<std::string>();
        lua_pushlstring(L, value.c_str(), value.length());
        break;
      }
      case avro::AVRO_INT:
        lua_pushinteger(L, field.value<int>());
        break;
      case avro::AVRO_LONG:
        lua_pushinteger(L, field.value<long>());
        break;
      case avro::AVRO_FLOAT:
        lua_pushnumber(L, field.value<float>());
        break;
      case avro::AVRO_DOUBLE:
        lua_pushnumber(L, field.value<double>());
        break;
      case avro::AVRO_BOOL:
        lua_pushboolean(L, field.value<bool>());
        break;
      default:
        lua_pushstring(L, "[[NOT HANDLED DATA TYPE]]");
        break;
    }
    lua_setfield(L, -2, name.c_str());
  }

  return 3;
}

const static luaL_Reg avro_meta_libs[] = {{"__index", NULL},
                                          {"__gc", lavro_mm_gc},
                                          {"__tostring", lavro_mm_tostring},
                                          {NULL, NULL}};

const static luaL_Reg avro_libs[] = {{"next", lavro_next},
                                     {"close", lavro_mm_gc},
                                     {NULL, NULL}};

const static luaL_Reg libs[] = {{"open", lavro_open}, {NULL, NULL}};

extern "C" {

LUAMOD_API int luaopen_avro(lua_State* L) {
  luaL_newlib(L, libs);
  if (luaL_newmetatable(L, LUA_AVROREADER_TYPE) > 0) {
    luaL_setfuncs(L, avro_meta_libs, 0);
    luaL_newlibtable(L, avro_libs);
    luaL_setfuncs(L, avro_libs, 0);
    lua_setfield(L, -2, "__index");
  }
  lua_pop(L, 1);  // remove metatable
  return 1;
}
}
