#!/usr/bin/env doctor
string = require'string'


local libs = {}

function libs.startswith(s, prefix)
    if #s < #prefix then
        return false
    end
    return s:sub(1, #prefix) == prefix
end

function libs.endswith(s, suffix)
    if #s < #suffix then
        return false
    end
    return s:sub(#s-#suffix+1, -1) == suffix
end

return libs

