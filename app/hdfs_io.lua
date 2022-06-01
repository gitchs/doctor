#!/usr/bin/env lua
os = require'os'
cjson = require'cjson'
missing = require'missing'
strutils = require'strutils'
local hdfsutils = require'hdfsutils'

local CASE_NAME = 'HDFS_IO'


local libs = function(tree)
    local query_id = tree:query_id()
    local summary = tree:node_at(2)
    local query_type = summary:info_strings('Query Type')
    local retval = {
        query_state = summary:info_strings('Query State'),
        tested = false,
        match = false,
    }
    if query_type ~= 'QUERY' and query_type ~= 'DML' then
        retval.report = string.format('invalid query type "%s"', query_type)
        return CASE_NAME, retval
    end
    local nodes = hdfsutils.list_hdfs_nodes(tree)
    retval.hdfs_node_num = #nodes
    if #nodes == 0 then
        retval.report = string.format('query has no hdfs nodes')
        return CASE_NAME, retval
    end

    retval.match = true
    local short_circuit_disabled_instances = {}
    local read_local_disable_instances = {}
    local zero_io_instances = {}
    local remote_read_instances = {}
    local remote_scan_instances = {}

    local sum_rows_return = 0
    local sum_rows_read = 0

    for _, node in ipairs(nodes) do
        -- hdfs counters
        --   https://github.com/apache/impala/blob/3.4.0/be/src/exec/hdfs-scan-node-base.cc#L104-L111
        local counters = node.delegation:counters()
        if counters['BytesRead'] == 0 then
            table.insert(zero_io_instances, node.instance)
            goto continue
        end

        sum_rows_return = sum_rows_return + (counters['RowsReturned'] or 0)
        sum_rows_read = sum_rows_read + (counters['RowsRead'] or 0)

        if counters['BytesRead'] > counters['BytesReadLocal'] then
            table.insert(remote_read_instances, node.instance)
        end

        if counters['RemoteScanRanges'] > 0 then
            table.insert(remote_scan_instances, node.instance)
        end

        if counters['BytesReadLocal'] == 0 then
            table.insert(read_local_disable_instances, node.instance)
            goto continue
        end

        if counters['BytesReadLocal'] > 0 and counters['BytesReadShortCircuit'] == 0 then
            table.insert(short_circuit_disabled_instances, node.instance)
            goto continue
        end
        ::continue::
    end
    retval.tested = true
    retval.short_circuit_disabled = #short_circuit_disabled_instances
    retval.read_local_disable = #read_local_disable_instances
    retval.zero_io = #zero_io_instances
    retval.remote_scan = #remote_scan_instances
    retval.remote_read = #remote_read_instances
    retval.sum_rows_return = sum_rows_return
    retval.sum_rows_read = sum_rows_read
    return CASE_NAME, retval
end


if os.getenv('DEBUG_HDFS_IO_MAIN') == nil then
    return libs
end


function main()
    if arg[1] == nil then
        print('no input file')
        return 1
    end
    local files = {
        '/Users/tinyproxy/Documents/data/workload_query.avro',
    }

    for _, filename in ipairs(files) do
        local avro_reader = avro.open(filename)
        while true do
            -- local b64_profile = io.open(arg[1]):read('*all')
            local has_more, row = avro_reader:next()
            if not has_more then
                break
            end
            local b64_profile = row['profile']
            local ok, err, tree = impala.parse_profile(b64_profile)
            assert(ok)
            local summary = tree:node_at(2)
            assert(summary:name() == 'Summary')
            local query_type = summary:info_strings('Query Type')
            local case_name, result = libs(tree)
            result['query_id'] = tree:query_id()
            if result.match then
                print(cjson.encode(result))
                if result.hdfs_node_num > (result.zero_io + result.short_circuit_disabled) then
                    local ofilename = string.format('profiles/%s.txt', tree:query_id())
                    local fd = io.open(ofilename, 'w')
                    fd:write(b64_profile)
                    fd:close()
                end
            end
        end
    end
end

io.stderr:write(string.format('%s\n', cjson.encode(missing.getrusage())))
main()
io.stderr:write(string.format('%s\n', cjson.encode(missing.getrusage())))


