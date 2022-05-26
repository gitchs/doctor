#!/usr/bin/env lua
os = require'os'
cjson = require'cjson'
missing = require'missing'
strutils = require'strutils'


function log(message)
    io.stderr:write(message .. '\n')
end


local BaseReport = {}
function BaseReport:new(o)
    o = o or {}
    o.name = 'HDFS_IO'
    setmetatable(o, BaseReport)
    return o
end


function BaseReport:to_markdown(self)
    print(self)
end



local list_hdfs_nodes = function(tree)
    function skip_branch(tree, branch_index)
        local node = tree:node_at(branch_index)
        local retval = branch_index
        local num_children = node:num_children()
        while num_children > 0 do
            retval = retval + 1
            node = tree:node_at(retval)
            num_children = num_children + node:num_children() - 1
        end
        return retval + 1
    end

    -- general speaking
    --   nodes[1] is Query node
    --   nodes[2] is Summary node
    -- let's start index from 3
    local index = 3
    local node = nil
    while true do
        node = tree:node_at(index)
        if node == nil then
            return {}
        end
        index = index + 1
        -- jump to execution profile node
        if strutils.startswith(node:name(), 'Execution Profile ') then
            break
        end
    end
    index = index + 1

    local nodes_num = tree:nodes_length()
    local fragment = nil
    local instance = nil
    local instance_lifecycle = nil
    local rows = {}
    while index <= nodes_num do
        node = tree:node_at(index)
        if node:name() == 'Per Node Profiles' then
            index = skip_branch(tree, index)
            goto continue
        end
        local node_name = node:name()
        if strutils.startswith(node_name, 'Averaged Fragment F') then
            index = skip_branch(tree, index)
            goto continue
        end
        if strutils.startswith(node_name, 'Fragment F') or strutils.startswith(node_name, 'Coordinator Fragment F') then
            fragment = node
            instance = nil -- reset instance when switch fragment
            goto add_index_then_continue
        end
        if strutils.startswith(node_name, 'Instance ') then
            instance = node
            goto add_index_then_continue
        end
        if node_name == 'Fragment Instance Lifecycle Timings' then
            instance_lifecycle = node
            goto add_index_then_continue
        end

        if strutils.startswith(node_name, 'HDFS_SCAN_NODE ') then
            local row = {
                delegation = node,
                index = index,
                fragment = fragment,
                instance = instance,
                instance_lifecycle = instance_lifecycle,
            }
            table.insert(rows, row)
            goto add_index_then_continue
        end

        ::add_index_then_continue::
        index = index + 1
        ::continue::
    end
    return rows

end

local libs = function(tree)
    local query_id = tree:query_id()
    local summary = tree:node_at(2)
    local query_type = summary:info_strings('Query Type')
    local retval = {
        case_name = 'HDFS_IO',
    }
    if query_type ~= 'QUERY' and query_type ~= 'DML' then
        retval.tested = false
        retval.report = string.format('invalid query type "%s"', query_type)
        return retval
    end
    local nodes = list_hdfs_nodes(tree)
    retval.hdfs_node_num = #nodes
    if #nodes == 0 then
        retval.tested = false
        retval.report = string.format('query has no hdfs nodes')
        return retval
    end


    local short_circuit_disabled_instances = {}
    local read_local_disable_instances = {}
    local zero_io_instances = {}
    local remote_scan_instances = {}

    for _, node in ipairs(nodes) do
        -- hdfs counters
        --   https://github.com/apache/impala/blob/3.4.0/be/src/exec/hdfs-scan-node-base.cc#L104-L111
        local counters = node.delegation:counters()
        if counters['BytesRead'] == 0 then
            table.insert(zero_io_instances, node.instance)
            goto continue
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
    return retval
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
            local result = libs(tree)
            print(cjson.encode(result))
        end
    end
end

print(cjson.encode(missing.getrusage()))
main()
print(cjson.encode(missing.getrusage()))

-- io.stderr:write(cjson.encode(missing.getrusage()))

