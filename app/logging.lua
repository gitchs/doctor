#!/usr/bin/env doctor
local os = require'os'
local debug = require'debug'
local missing = require'missing'
local strutils = require'strutils'

local libs = {
    DEBUG=0,
    INFO=1,
    WARN=2,
    ERROR=3,
}

local log_level = 0

function libs.set_level(level)
    assert(level>=0 and level <= libs.ERROR, 'invalid log level')
    log_level = level
end

local function log(lnum, level, format, ...)
    if lnum < log_level then
        return
    end
    local message = nil
    local _, dinfo = pcall(debug.getinfo, 4);
    dinfo = dinfo or {
        source = '[UNKNOWN]',
        currentline = -1,
    };
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
    io.stderr:write(string.format('[%s %s.%06d %s:%d] %s\n', level, now, usec, dinfo.source, dinfo.currentline, message))
end

libs.debug = function (format, ...)
    log(libs.DEBUG, 'DEBUG', format, ...)
end

libs.info = function(format, ...)
    log(libs.INFO, 'INFO', format, ...)
end

libs.error = function(format, ...)
    log(libs.ERROR, 'ERROR', format, ...)
end


return libs

