#!/usr/bin/env lua
os = require'os'
cjson = require'cjson'

local libs = {}

function log(level, message)
    local now = os.date('%Y-%m-%d %H:%M:%S')
    io.stderr:write(string.format('[%s] %s %s\n', level, now, message))
end

libs.info = function(format, ...)
    local message = string.format(format, ...)
    log('INFO', message)
end


return libs

