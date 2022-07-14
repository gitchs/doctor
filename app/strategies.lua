#!/usr/bin/env doctor
local strutils = require'strutils'
local profileutils = require'profileutils'
local gsl = require'gsl'


local libs = {}


libs.ProfileContext = {};
libs.ProfileContext['__index'] = libs.ProfileContext;

function libs.ProfileContext.new(o)
    o = o or {
        max_ignored_query_duration_sec = 20, -- 执行时间很短，倾斜分析没有必要
        skew_op = {
            max_ignored_op_duration_sec = 20, -- 算子时间很短，倾斜分析没有必要
            max_allowed_skew_range_sec = 60, -- 同一个算子，各个实例，最长的执行时间 - 最短执行时间，不超过这个范围视为不倾斜
            min_symmetry_op_range_sec = 90, -- 算子的时间，都在[mean - 2 * std, mean + 2 * std]区间，但可能是对称的分布
        },
    }
    setmetatable(o, libs.ProfileContext)
    return o
end

local default_profile_context = libs.ProfileContext.new()

function libs.hdfs_statics(tree2)
    local gops = profileutils.group_operators(tree2)
    if gops == nil then
        return {}
    end
    local retval = {}
    for oid, ops in pairs(gops) do
        if #ops == 0 then 
            goto NEXT_OP_GROUP
        end
        if not strutils.startswith(ops[1].name, 'HDFS_SCAN_NODE') then
            goto NEXT_OP_GROUP
        end
        retval[oid] = {}
        retval[oid].sum_rows_read = ops[1].counters['RowsRead']
        retval[oid].sum_rows_returned = ops[1].counters['RowsReturned']
        retval[oid].rows_filter_rate = 0
        retval[oid].sum_bytes_read = ops[1].counters['BytesRead']
        retval[oid].sum_bytes_read_local = ops[1].counters['BytesReadLocal']
        retval[oid].sum_bytes_read_shortcircuit = ops[1].counters['BytesReadShortCircuit']
        retval[oid].sum_bytes_read_remote = 0
        retval[oid].sum_bytes_read_remote_unexpected = ops[1].counters['BytesReadRemoteUnexpected']
        retval[oid].sum_throughput = ops[1].counters['TotalReadThroughput']
        retval[oid].max_throughput = ops[1].counters['TotalReadThroughput']
        retval[oid].min_throughput = ops[1].counters['TotalReadThroughput']
        for index = 2, #ops do
            local op = ops[index]
            local counters = op.counters
            local throughput = counters['TotalReadThroughput'] or 0
            if throughput < retval[oid].min_throughput then
                retval[oid].min_throughput = throughput
            end
            if throughput > retval[oid].max_throughput then
                retval[oid].max_throughput = throughput
            end
            retval[oid].sum_rows_read = retval[oid].sum_rows_read + (counters['RowsRead'] or 0)
            retval[oid].sum_rows_returned = retval[oid].sum_rows_returned + (counters['RowsReturned'] or 0)
            retval[oid].sum_throughput = retval[oid].sum_throughput + (counters['TotalReadThroughput'] or 0)
            retval[oid].sum_bytes_read = retval[oid].sum_bytes_read + (counters['BytesRead'] or 0)
            retval[oid].sum_bytes_read_local = retval[oid].sum_bytes_read_local + (counters['BytesReadLocal'] or 0)
            retval[oid].sum_bytes_read_shortcircuit = retval[oid].sum_bytes_read_shortcircuit + (counters['BytesReadShortCircuit'] or 0)
            retval[oid].sum_bytes_read_remote_unexpected = retval[oid].sum_bytes_read_remote_unexpected + (counters['BytesReadRemoteUnexpected'] or 0)
        end
        retval[oid].sum_bytes_read_remote = retval[oid].sum_bytes_read - retval[oid].sum_bytes_read_local
        retval[oid].rows_filter_rate = retval[oid].sum_rows_returned / retval[oid].sum_rows_read * 100

        -- 注意，这里开始，单位变成了MB，前面不处理是为了减少"除法操作"
        -- 注意，这里开始，单位变成了MB，前面不处理是为了减少"除法操作"
        -- 注意，这里开始，单位变成了MB，前面不处理是为了减少"除法操作"
        retval[oid].sum_bytes_read = retval[oid].sum_bytes_read/1048576.0
        retval[oid].sum_bytes_read_local = retval[oid].sum_bytes_read_local/1048576.0
        retval[oid].sum_bytes_read_shortcircuit = retval[oid].sum_bytes_read_shortcircuit/1048576.0
        retval[oid].sum_bytes_read_remote = retval[oid].sum_bytes_read_remote/1048576.0
        retval[oid].sum_bytes_read_remote_unexpected = retval[oid].sum_bytes_read_remote_unexpected/1048576.0
        retval[oid].sum_throughput = retval[oid].sum_throughput/1048576.0
        retval[oid].max_throughput = retval[oid].max_throughput/1048576.0
        retval[oid].min_throughput = retval[oid].min_throughput/1048576.0
        ::NEXT_OP_GROUP::
    end
    return retval
end

function libs.is_slow(tree2, _)
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

function libs.operator_skew_detection(tree2, ctx)
    if ctx == nil then
        ctx = default_profile_context
    end
    assert(ctx ~= nil)

    local retval = {}
    local summary = tree2.children[1]
    assert(summary.name == 'Summary', 'first children should be [SUMMARY]')
    local raw_summary = getmetatable(summary).raw
    assert(raw_summary ~= nil, 'getmetatable(summary).raw should not be nil')
    local duration_sec = tonumber(raw_summary:info_strings('Duration(ms)')) / 1000
    if duration_sec < ctx.max_ignored_query_duration_sec then
        -- 执行时间很短，倾斜分析没有必要
        return retval
    end

    local gops = profileutils.group_operators(tree2)
    for _, ops in pairs(gops) do
        assert(ops ~= nil, 'ops could not be nil')
        if #ops <= 1 then
            goto op_skew_continue
        end
        local total_times = {}
        for _, op in ipairs(ops) do
            table.insert(total_times, op.counters['TotalTime'])
        end
        local min, max, mean, std = gsl.status(total_times)
        local min_sec = min/1e9
        local max_sec = max/1e9
        if max_sec < ctx.skew_op.max_ignored_op_duration_sec then
            -- 这种太小的值，没有检查的必要
            goto op_skew_continue
        end
        if (max_sec - min_sec) < ctx.skew_op.max_allowed_skew_range_sec then
            -- range足够小的时候，不根据mean、std进行异常值的判断（意义很小）
            goto op_skew_continue
        end
        local up_bound = mean + 2 * std
        for _, op in ipairs(ops) do
            if (op.counters['TotalTime'] > up_bound) then
                table.insert(
                    retval,
                    string.format(
                        [[OP{%s} host(%s) is slow: host: %f, min %f]],
                        op.name, op.host, op.counters['TotalTime']/1e9, min_sec))
            end
        end

        local duration_delta_sec = (max - min)/1e9 -- convert ns to second
        if duration_delta_sec > ctx.skew_op.min_symmetry_op_range_sec and next(retval) == nil then
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

function libs.test_profile(tree2, ctx)
    local retval = {}
    local r = libs.is_slow(tree2, ctx)
    retval.is_slow = r
    if r < 0 then
        return retval
    end
    retval.skew_ops = libs.operator_skew_detection(tree2, ctx)
    return retval
end

return libs
