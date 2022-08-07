#!/usr/bin/env doctor
local io = require'io'
local re2 = require're2'
local string = require'string'
local libs = {}


local logging = {}

function logging.info(format, ...)
    local message = string.format(format, ...)
    print(message)
end


local Wrapper = {
}
Wrapper['__index'] = Wrapper

function Wrapper.new(o)
    o = o or {
        modules = {},
        cmodules = {},
    }
    setmetatable(o, Wrapper)
    return o
end


local function bytes2carray(raw)
    local buffer = {}
    local bytes = {raw:byte(1, -1)}
    for _, b in ipairs(bytes) do
        table.insert(buffer, tostring(b))
    end
    return table.concat(buffer, ', ')
end


function Wrapper:add_module(module_name, filename)
    local fd = io.open(filename, 'rb')
    assert(fd ~= nil)
    local module_cname = string.format('LUA_BUILTIN_MODULE_%s', module_name:upper())
    local code = string.format('static const unsigned char %s[] = {', module_cname)
    local first_line = fd:read()

    code = code .. bytes2carray('-- \n')


    if first_line:byte(1, 1) ~= 35 then
        code = string.format('%s, %s', code, bytes2carray(first_line))
    end

    while true do
        local chunk = fd:read(65536)
        if chunk == nil then
            break
        end
        code = string.format('%s, %s', code, bytes2carray(chunk))
    end
    code = string.format('%s, 0};\n', code)
    fd:close()
    table.insert(self.modules, {
        name = module_name,
        cname = module_cname,
        code = code,
    })
end


function Wrapper:codegen(output_filename)
    local fd = nil
    if output_filename ~= nil then
        fd = io.open(output_filename, 'w')
    end
    fd = fd or io.stdout
    local prefix <const> = [[
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>


typedef struct LuaExternalBuiltinModule {
    char* name;
    char* lua_source;
    const unsigned char* script;
} LuaExternalBuiltinModule;


]]
    local suffix <const> = [[


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

]]
    fd:write(prefix)
    if #self.modules == 0 then
        fd:write('// empty modules \n')
        fd:write('static LuaExternalBuiltinModule modules[] = {{NULL, NULL}};\n')
    else 
        for _, module in pairs(self.modules) do
            fd:write(module.code)
            fd:write('\n')
        end
        fd:write([[static LuaExternalBuiltinModule modules[] = {
]])
        for _, module in pairs(self.modules) do
            fd:write(string.format('    {"%s", "builtin/%s", %s},\n', module.name, module.name, module.cname))
        end
        fd:write('    {NULL, NULL}\n};\n')
    end
    fd:write(suffix)
    fd:close()
end

local function main()
    local wrapper = Wrapper.new()
    local lua_suffix = ".lua"
    for _, filename in ipairs(arg) do
        local suffix = filename:sub(#filename - #lua_suffix + 1, -1)
        if suffix:lower() ~= lua_suffix then
            goto NEXT_FILE
        end
        local module_name = re2.match(filename, '\\/(\\w+)\\.lua$')[1]
        wrapper:add_module(module_name, filename)
        ::NEXT_FILE::
    end
    wrapper:codegen()
end


local debug = require'debug'
if not pcall(debug.getlocal, 4, 1) then
    main()
end

return libs

