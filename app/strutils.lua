#!/usr/bin/env lua
string = require'string'


local libs = {}

function libs.startswith(s, prefix)
    if #s < #prefix then
        return false
    end
    return s:sub(1, #prefix) == prefix
end


return libs

