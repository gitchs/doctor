#!/usr/bin/env doctor
os = require'os'
math = require'math'
cjson = require'cjson'
missing = require'missing'
strutils = require'strutils'

local libs = {}

function log(level, format, ...)
    local message = nil
    if type(format) ~= 'string' then
        message = tostring(format)
        if strutils.startswith(message, 'table:') and type(format) == 'table' then
            message = ''
            for k, v in pairs(format) do
                message = string.format('%s\n%s=%s', message, k, v)
            end
        end
    else
        message = string.format(format, ...)
    end
    local sec, usec = missing.gettimeofday()
    local now = os.date('%Y-%m-%d %H:%M:%S', sec)
    io.stderr:write(string.format('[%s %s.%06d] %s\n', level, now, usec, message))
end

libs.info = function(format, ...)
    log('INFO', format, ...)
end

libs.error = function(format, ...)
    log('ERROR', format, ...)
end


return libs

