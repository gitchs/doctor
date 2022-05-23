#!/usr/bin/env lua
missing = require'missing'


SLOW_QUERY_TIMEOUT = 3 * 60 * 1000

local libs = function(tree)
    local summary_node = tree:node_at(2)
    assert(summary_node:name() == 'Summary')
    local start_time = os.date(
        '*t', missing.strptime(
            summary_node:info_strings('Start Time'):sub(1, 19),
            '%Y-%m-%d %H:%M:%S'))

    if start_time.wday == 1 or start_time.wday == 7 then
        -- ignore Sun and Sat
        return false
    end
    if start_time.hour >=9 and start_time.hour < 20 then
        -- 09:00 ~ 20:00
        local duration = tonumber(summary_node:info_strings("Duration(ms)"))
        return duration > SLOW_QUERY_TIMEOUT
    end
    return false
end


return libs

