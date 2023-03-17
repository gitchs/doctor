#include "limpala.h"
#include <memory>
#include "RuntimeProfile_types.h"
#include "thrift/protocol/TBase64Utils.h"
#include "thrift/protocol/TCompactProtocol.h"
#include "thrift/protocol/TDebugProtocol.h"
#include "thrift/protocol/TJSONProtocol.h"
#include "thrift/protocol/TProtocol.h"
#include "thrift/transport/TBufferTransports.h"
#include "thrift/transport/TZlibTransport.h"


#include "utils.h"


extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lua_cjson.h"
#include "lualib.h"

const static char* const LUA_TRUNTIMEPROFILETREE_TYPE = "TRuntimeProfileTree*";
const static char* const LUA_TRUNTIMEPROFILENODE_TYPE = "TRuntimeProfileNode*";
const static int32_t TDEBUG_STRING_MAX_SIZE = 128 * 1024 * 1024;
}

typedef struct {
  impala::TRuntimeProfileTree* delegation;
} LRuntimeProfileTree;

static int limpala_runtime_profile_tree_mm_tostring(lua_State* L) {
  LRuntimeProfileTree* tree = (LRuntimeProfileTree*)lua_touserdata(L, 1);
  if (tree == nullptr) {
    lua_pushnil(L);
    return 1;
  }
  char buffer[128] = {0};
  sprintf(buffer, "%s %p", LUA_TRUNTIMEPROFILETREE_TYPE, tree->delegation);
  lua_pushstring(L, buffer);
  return 1;
}

static int limpala_runtime_profile_tree_debug(lua_State* L) {
  LRuntimeProfileTree* tree = (LRuntimeProfileTree*)lua_touserdata(L, 1);
  if (tree == nullptr) {
    lua_pushnil(L);
    return 1;
  }
  std::shared_ptr<apache::thrift::transport::TMemoryBuffer> buffer =
      std::make_shared<apache::thrift::transport::TMemoryBuffer>();
  std::shared_ptr<apache::thrift::protocol::TProtocol> protocol =
      apache::thrift::protocol::TDebugProtocolFactory().getProtocol(buffer);
  dynamic_cast<apache::thrift::protocol::TDebugProtocol*>(protocol.get())
      ->setStringSizeLimit(TDEBUG_STRING_MAX_SIZE);
  tree->delegation->write(protocol.get());
  const std::string& retval = buffer->readAsString(buffer->getBufferSize());
  lua_pushstring(L, retval.c_str());
  return 1;
}

static int limpala_runtime_profile_tree_mm_gc(lua_State* L) {
  LRuntimeProfileTree* tree = (LRuntimeProfileTree*)lua_touserdata(L, 1);
  if (tree != nullptr) {
    delete tree->delegation;
  }
  return 0;
}

static int limpala_runtime_profile_tree_nodes_length(lua_State* L) {
  LRuntimeProfileTree* tree = (LRuntimeProfileTree*)lua_touserdata(L, 1);
  lua_pushinteger(L, tree == nullptr ? 0 : tree->delegation->nodes.size());
  return 1;
}

static int limpala_runtime_profile_tree_query_id(lua_State* L) {
  LRuntimeProfileTree* tree = (LRuntimeProfileTree*)lua_touserdata(L, 1);
  if (tree == nullptr) {
    lua_pushnil(L);
    return 1;
  }
  auto& node = tree->delegation->nodes.at(0);
  // query_id: Query (id=[33 chars])
  lua_pushlstring(L, node.name.c_str() + 10, 33);
  return 1;
}

static int limpala_runtime_profile_tree_node_at(lua_State* L) {
  LRuntimeProfileTree* tree = (LRuntimeProfileTree*)lua_touserdata(L, 1);
  if (tree == nullptr) {
    lua_pushnil(L);
    return 1;
  }
  size_t index = lua_tointeger(L, 2);
  if (index == 0 || index > tree->delegation->nodes.size()) {
    lua_pushnil(L);
    return 1;
  }
  const auto& node = tree->delegation->nodes.at(index - 1);
  lua_pushlightuserdata(L, (void*)&node);
  luaL_setmetatable(L, LUA_TRUNTIMEPROFILENODE_TYPE);
  return 1;
}

static int limpala_runtime_profile_node_debug(lua_State* L) {
  impala::TRuntimeProfileNode* node =
      (impala::TRuntimeProfileNode*)lua_touserdata(L, 1);
  if (node == nullptr) {
    lua_pushnil(L);
    return 1;
  }
  std::shared_ptr<apache::thrift::transport::TMemoryBuffer> buffer =
      std::make_shared<apache::thrift::transport::TMemoryBuffer>();
  std::shared_ptr<apache::thrift::protocol::TProtocol> protocol =
      apache::thrift::protocol::TDebugProtocolFactory().getProtocol(buffer);
  dynamic_cast<apache::thrift::protocol::TDebugProtocol*>(protocol.get())
      ->setStringSizeLimit(TDEBUG_STRING_MAX_SIZE);
  node->write(protocol.get());
  const std::string& retval = buffer->readAsString(buffer->getBufferSize());
  lua_pushstring(L, retval.c_str());
  return 1;
}

static int limpala_runtime_profile_node_counters(lua_State* L) {
  impala::TRuntimeProfileNode* node =
      (impala::TRuntimeProfileNode*)lua_touserdata(L, 1);
  if (node == nullptr) {
    lua_pushnil(L);
    return 1;
  }
  lua_newtable(L);
  auto it = node->counters.begin();
  for (; it != node->counters.end(); ++it) {
    lua_pushinteger(L, it->value);
    lua_setfield(L, -2, it->name.c_str());
  }
  return 1;
}

static int limpala_runtime_profile_node_info_strings(lua_State* L) {
  impala::TRuntimeProfileNode* node =
      (impala::TRuntimeProfileNode*)lua_touserdata(L, 1);
  if (node == nullptr) {
    lua_pushnil(L);
    return 1;
  }

  const char* const key = lua_tostring(L, 2);
  if (key != nullptr) {
    auto it = node->info_strings.find(key);
    if (it != node->info_strings.end()) {
      lua_pushstring(L, it->second.c_str());
    } else {
      lua_pushnil(L);
    }
    return 1;
  }

  lua_newtable(L);
  for (auto it = node->info_strings.begin(); it != node->info_strings.end();
       ++it) {
    lua_pushstring(L, it->second.c_str());
    lua_setfield(L, -2, it->first.c_str());
  }
  return 1;
}

static int limpala_runtime_profile_node_name(lua_State* L) {
  impala::TRuntimeProfileNode* node =
      (impala::TRuntimeProfileNode*)lua_touserdata(L, 1);
  if (node == nullptr) {
    lua_pushnil(L);
    return 1;
  }
  lua_pushstring(L, node->name.c_str());
  return 1;
}

static int limpala_runtime_profile_node_num_children(lua_State* L) {
  impala::TRuntimeProfileNode* node =
      (impala::TRuntimeProfileNode*)lua_touserdata(L, 1);
  if (node == nullptr) {
    lua_pushnil(L);
    return 1;
  }
  lua_pushinteger(L, node->num_children);
  return 1;
}

static int limpala_runtime_profile_node_event_sequences(lua_State* L) {
  impala::TRuntimeProfileNode* node =
      (impala::TRuntimeProfileNode*)lua_touserdata(L, 1);
  if (node == nullptr) {
    lua_pushnil(L);
    return 1;
  }

  const char* const name = lua_tostring(L, 2);
  if (name != nullptr) {
    auto it = node->event_sequences.begin();
    for (; it != node->event_sequences.end(); ++it) {
      if (it->name == name) {
        break;
      }
    }
    if (it == node->event_sequences.end()) {
      lua_pushnil(L);
      return 1;
    }
    lua_newtable(L);
    luaL_setmetatable(L, LUA_ARRAY_METATABLE);
    for (size_t i = 0; i < it->labels.size(); ++i) {
      lua_pushinteger(L, i + 1);
      lua_newtable(L);
      luaL_setmetatable(L, LUA_ARRAY_METATABLE);
      lua_pushinteger(L, 1);
      lua_pushstring(L, it->labels.at(i).c_str());
      lua_settable(L, -3);

      lua_pushinteger(L, 2);
      lua_pushinteger(L, it->timestamps.at(i));
      lua_settable(L, -3);

      lua_settable(L, -3);
    }
    return 1;
  }

  // fetch all event_sequences
  lua_newtable(L);
  luaL_setmetatable(L, LUA_ARRAY_METATABLE);
  int es_index = 1;
  for (auto it = node->event_sequences.begin();
       it != node->event_sequences.end(); ++it) {
    lua_pushinteger(L, es_index++);
    lua_newtable(L);
    lua_pushstring(L, it->name.c_str());
    lua_setfield(L, -2, "name");
    lua_newtable(L);
    for (size_t i = 0; i < it->labels.size(); ++i) {
      lua_pushinteger(L, i + 1);
      lua_newtable(L);
      lua_pushinteger(L, 1);
      lua_pushstring(L, it->labels.at(i).c_str());
      lua_settable(L, -3);
      lua_pushinteger(L, 2);
      lua_pushinteger(L, it->timestamps.at(i));
      lua_settable(L, -3);
      lua_settable(L, -3);
    }
    lua_setfield(L, -2, "events");
    lua_settable(L, -3);
  }
  return 1;
}

const static luaL_Reg runtime_profile_node_mm_libs[] = {{NULL, NULL}};

const static luaL_Reg runtime_profile_node_libs[] = {
    {"debug", limpala_runtime_profile_node_debug},
    {"name", limpala_runtime_profile_node_name},
    {"num_children", limpala_runtime_profile_node_num_children},
    {"counters", limpala_runtime_profile_node_counters},
    {"event_sequences", limpala_runtime_profile_node_event_sequences},
    {"info_strings", limpala_runtime_profile_node_info_strings},
    {NULL, NULL}};

const static luaL_Reg runtime_profile_tree_mm_libs[] = {
    {"__tostring", limpala_runtime_profile_tree_mm_tostring},
    {"__gc", limpala_runtime_profile_tree_mm_gc},
    {NULL, NULL}};

const static luaL_Reg runtime_profile_tree_libs[] = {
    {"query_id", limpala_runtime_profile_tree_query_id},
    {"nodes_length", limpala_runtime_profile_tree_nodes_length},
    {"node_at", limpala_runtime_profile_tree_node_at},
    {"debug", limpala_runtime_profile_tree_debug},
    {NULL, NULL}};

static int limpala_parse_profile(lua_State* L) {
  size_t b64_plen = 0;
  const char* const b64_profile = lua_tolstring(L, 1, &b64_plen);
  if (!b64_plen) {
    lua_pushboolean(L, 0);
    lua_pushstring(L, "empty input");
    lua_pushnil(L);
    return 3;
  }

  size_t plen = b64_plen * 3 / 4;
  std::string raw_profile = Base64Decode(b64_profile, b64_plen);

  lua_pushboolean(L, 1);
  lua_pushnil(L);
  std::shared_ptr<apache::thrift::transport::TMemoryBuffer> buffer =
      std::make_shared<apache::thrift::transport::TMemoryBuffer>(plen);
  buffer->write((unsigned char*)raw_profile.c_str(), raw_profile.size());
  apache::thrift::protocol::TCompactProtocol protocol(
      apache::thrift::transport::TZlibTransportFactory().getTransport(buffer));
  LRuntimeProfileTree* tree =
      (LRuntimeProfileTree*)lua_newuserdata(L, sizeof(LRuntimeProfileTree));
  tree->delegation = new impala::TRuntimeProfileTree();
  tree->delegation->read(&protocol);
  luaL_setmetatable(L, LUA_TRUNTIMEPROFILETREE_TYPE);
  return 3;
}

extern "C" {

const static luaL_Reg libs[] = {{"parse_profile", limpala_parse_profile},
                                {NULL, NULL}};

LUAMOD_API int luaopen_impala(lua_State* L) {
  luaL_newlib(L, libs);
  if (luaL_newmetatable(L, LUA_TRUNTIMEPROFILETREE_TYPE) > 0) {
    luaL_setfuncs(L, runtime_profile_tree_mm_libs, 0);
    lua_newtable(L);
    luaL_setfuncs(L, runtime_profile_tree_libs, 0);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
  }

  if (luaL_newmetatable(L, LUA_TRUNTIMEPROFILENODE_TYPE) > 0) {
    luaL_setfuncs(L, runtime_profile_node_mm_libs, 0);
    lua_newtable(L);
    luaL_setfuncs(L, runtime_profile_node_libs, 0);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
  }
  return 1;
}
}
