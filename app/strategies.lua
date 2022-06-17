#!/usr/bin/env doctor
local logging = require'logging'
local profileutils = require'profileutils'
local gsl = require'gsl'

local libs = {}

function libs.is_slow(tree2)
    -- -1 test if not suitable for this profile
    -- 0  not slow
    -- 1  slow
    local summary = tree2.children[1]
    assert(summary.name == 'Summary', 'first children should be [SUMMARY]')
    local raw_summary = getmetatable(summary).raw
    assert(raw_summary ~= nil, 'getmetatable(summary).raw should not be nil')
    local query_state = raw_summary:info_strings('Query State')
    if query_state ~= 'FINISHED' then
        return -2
    end
    local query_type = raw_summary:info_strings('Query Type')
    if query_type ~= 'QUERY' and query_type ~= 'DML' then
        return -1
    end
    local duration = tonumber(raw_summary:info_strings('Duration(ms)'))
    if duration < 5 * 60 * 1000 then
        return 0
    end
    return 1
end

function libs.operator_skew_detection(tree2)
    local retval = {}
    local summary = tree2.children[1]
    assert(summary.name == 'Summary', 'first children should be [SUMMARY]')
    local raw_summary = getmetatable(summary).raw
    assert(raw_summary ~= nil, 'getmetatable(summary).raw should not be nil')
    local duration_sec = tonumber(raw_summary:info_strings('Duration(ms)')) / 1000
    if duration_sec < 20 then
        -- 执行时间很短，倾斜分析没有必要
        return retval
    end

    local gops = profileutils.group_operators(tree2)
    for oid, ops in pairs(gops) do
        assert(ops ~= nil, 'ops could not be nil')
        if #ops <= 1 then
            goto op_skew_continue
        end
        local total_times = {}
        for i, op in ipairs(ops) do
            table.insert(total_times, op.counters['TotalTime'])
        end
        local min, max, mean, std = gsl.status(total_times)
        local min_sec = min/1e9
        local max_sec = max/1e9
        if max_sec < 20 then
            -- 这种太小的值，没有检查的必要
            goto op_skew_continue
        end
        if (max_sec - min_sec) < 30 then
            -- range足够小的时候，不根据mean、std进行异常值的判断（意义很小）
            goto op_skew_continue
        end
        local up_bound = mean + 2 * std
        for i, op in ipairs(ops) do
            if (op.counters['TotalTime'] > up_bound) then
                table.insert(
                    retval,
                    string.format(
                        [[OP{%s} host(%s) is slow: host: %f, min %f]],
                        op.name, op.host, op.counters['TotalTime']/1e9, min_sec))
            end
        end

        local duration_delta_sec = (max - min)/1e9 -- convert ns to second
        if duration_delta_sec > 90 and next(retval) == nil then
            -- 左右对称分布的话，mean、std不能起到检查作用，这个时候需要检查数据分布的宽度
            -- TODO: 这里用一个常数不太合适，临时先这么处理
            table.insert(
                retval,
                string.format(
                    [[OP{%s} 执行速度两极分化, query duration %.1f, op TotalTime min/max ==> %.1f/%.1f]],
                    ops[1].name,
                    duration_sec,
                    min/1e9,
                    max/1e9))
            -- TODO: 检查是否存在多实例竞争
            -- 比如3个主机，开了双实例，一共6个impala，如果出现这种情况，调度上需要做优化
            --  node01-instance0: 慢
            --  node01-instance1: 快
            --  node02-instance0: 慢
            --  node02-instance1: 快
            --  node03-instance0: 慢
            --  node03-instance1: 快
        end
        ::op_skew_continue::
    end
    return retval
end

function libs.test_profile(tree)
    local retval = {}
    local tree2 = profileutils.build_tree(tree)
    local r = libs.is_slow(tree2)
    retval.is_slow = r
    if r < 0 then
        return retval
    end
    retval.skew_ops = libs.operator_skew_detection(tree2)
    return retval
end

return libs
