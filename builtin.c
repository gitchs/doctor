#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>


typedef struct LuaExternalBuiltinModule {
    char* name;
    char* lua_source;
    const unsigned char* script;
} LuaExternalBuiltinModule;


// empty modules 
static LuaExternalBuiltinModule modules[] = {{NULL, NULL}};


LUAMOD_API int lexternal_searchers_builtin(lua_State* L) {
  const char* const module_name = lua_tostring(L, 1);
  LuaExternalBuiltinModule* module = modules;
  for (;module->name != NULL;module++) {
    if (strcmp(module_name, module->name) == 0) {
      luaL_loadbuffer(L, (const char*)module->script, strlen((const char*)module->script), module->name);
      lua_pushstring(L, module->lua_source);
      return 2;
    }
  }
  lua_pushnil(L);
  return 0;
}

