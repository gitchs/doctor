#!/usr/bin/env doctor
local missing = require'missing'

-- 从以下条件进行判断
-- 1. 执行日期不是周末;
-- 1. 执行时间在09:00~20:00，一般可以覆盖大部分客户的上班时间；20:00后继续加班的情况，一般其他人回家也可以把查询资源空出来了；
-- 1. 查询时间在3分钟以上（随意制定的数字，后续可能更改）；
-- 1. 标记处理，不对后续的处理流程产生任何影响；


local SLOW_QUERY_TIMEOUT = 3 * 60 * 1000
local CASE_NAME = 'is_slow_query'

local libs = function(tree)
    local summary_node = tree:node_at(2)
    assert(summary_node:name() == 'Summary')
    local start_time = os.date(
        '*t', missing.strptime(
            summary_node:info_strings('Start Time'):sub(1, 19),
            '%Y-%m-%d %H:%M:%S'))

    local retval = {
        is_slow = false,
    }
    if start_time.wday == 1 or start_time.wday == 7 then
        -- ignore Sun and Sat
        retval.is_slow = false
        return CASE_NAME, retval
    end
    if start_time.hour >=9 and start_time.hour < 20 then
        -- 09:00 ~ 20:00
        local duration = tonumber(summary_node:info_strings("Duration(ms)"))
        retval.is_slow = duration > SLOW_QUERY_TIMEOUT
        return CASE_NAME, retval
    end
    return CASE_NAME, retval
end


return libs

