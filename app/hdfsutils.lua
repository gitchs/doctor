#!/usr/bin/env doctor
local os = require'os'
local cjson = require'cjson'
local missing = require'missing'
local strutils = require'strutils'
local gsl = require'gsl'

local libs = {}


function libs.list_hdfs_nodes(tree)
    local function skip_branch(tree, branch_index)
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


function libs.list_hdfs_node_counters(tree)
    local nodes = libs.list_hdfs_nodes(tree)
    local rows = {}
    for i, node in ipairs(nodes) do
        local row = node.delegation:counters()
        row['fragment'] = node.fragment:name()
        row['instance'] = node.instance:name()
        rows[i] = row
    end
    return rows
end


local function naive_skew(ns)
    if #ns == 0 or type(ns[1]) ~= 'number' then
        return false
    end
    -- not an ideal implementation
    local mean, std, skew, kurt = gsl.status(ns)
    local up = mean + 2 * std
    for _, n in ipairs(ns) do
        if n > up then
            return true
        end
    end
    return false
end


local function is_skew(ns)
    return naive_skew(ns)
end

function libs.skew_counters(counters)
    -- counters ==> list_hdfs_node_counters
    local groups = {}
    local retval = {}

    for i, counter in ipairs(counters) do
        groups[counter.fragment] = groups[counter.fragment] or {}
        table.insert(groups[counter.fragment], counter)
    end

    for fragment, fragment_counters in pairs(groups) do
        local ns = {}
        for _, counter in ipairs(fragment_counters) do
            for ckey, cval in pairs(counter) do
                ns[ckey] = ns[ckey] or {}
                table.insert(ns[ckey], cval)
            end
        end
        for ckey, cns in pairs(ns) do
            local result = is_skew(cns)
            if result then
                retval[fragment] = retval[fragment] or {}
                retval[fragment][ckey] = true
            end
        end
    end
    return retval
end


return libs
