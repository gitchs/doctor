#!/usr/bin/env doctor
local math = require'math'
local table = require'table'
local strutils = require'strutils'
local libs = {}


local callable = function(t)
  if t == nil then
    return false
  end
  if type(t) == 'function' then
    return true
  end
  if type(t) == 'table' then
    local mt = getmetatable(t)
    return type(mt['__call']) == 'function'
  end
  return false
end

local options = {}
function libs.define(...)
  local option = {
    short_name = '',
    long_name = '',
    value = nil,
    nargs = 1,
    type = nil,
  }
  local configure = nil
  for _, item in ipairs({...}) do
    local itype = type(item)
    if itype == 'table' then
      configure = item
    elseif itype == 'string' then
      if strutils.startswith(item, '--') then
        option.long_name = item
      elseif strutils.startswith(item, '-') then
        option.short_name = item
      else
        assert(false, 'invalid flag value')
      end
    else
      assert(false, 'only accept string/table')
    end
  end

  if configure ~= nil then
    if configure.nargs ~= nil and configure.nargs ~= 1 then
      assert(configure.nargs == 0, 'nargs only accept 0/1')
      option.nargs = configure.nargs
      option.value = false
    end
    if configure.default ~= nil then
      option.vlaue = configure.default
    end
    if callable(configure.type) then
      option.type = configure.type
    end
  end

  assert(option.long_name ~= '', 'option should have long_name')
  table.insert(options, option)
end


function libs.parse(args)
  local idx = 1
  while idx <= #args do
    local arg = args[idx]
    for _, option in ipairs(options) do
      local nargs = option.nargs
      if arg == option.short_name then
        if nargs == 0 then
          option.value = true
        else
          option.value = args[idx+1]
        end
        idx = idx + nargs + 1
        break
      elseif arg == option.long_name then
        if nargs == 0 then
          option.value = true
        else
          option.value = args[idx+1]
        end
        idx = idx + nargs + 1
        break
      elseif strutils.startswith(arg, option.long_name .. '=') then
        local i = arg:find('=')
        local optarg = arg:sub(i+1, #arg)
        print('optarg: ', optarg)
        if narg == 0 then
          if optarg == 'false' or optarg == '0' then
            option.value = false
          elseif optarg == 'true' or optarg == '1' then
            option.value = true
          else
            assert(false, 'invalid optarg: ' .. optarg)
          end
        else
          option.value = optarg
        end
        idx = idx + 1
        break
      end
    end
  end
end



local search_cache = {}
function libs.get(name)
  if search_cache[name] ~= nil then
    return search_cache[name]
  end
  local key = name
  if not strutils.startswith(key, '--') then
    key = '--' .. key
  end
  for _, option in ipairs(options) do
    if option.long_name == key then
      local retval = option.value
      if option.type ~= nil then
        retval = option.type(retval)
      end
      search_cache[name] = retval
      return retval
    end
  end
end

libs.options = options

libs.tointeger = function(v)
  return math.floor(tonumber(v))
end

return libs

