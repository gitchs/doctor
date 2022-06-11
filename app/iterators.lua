#!/usr/bin/env doctor
local logging = require'logging'
local avro = require'avro'

local libs = {}
function libs.avro(filename)
    local reader = avro.open(filename)
    assert(reader ~= nil, string.format('failed to open avro file "%s"', filename))
    return function()
        local has_more, err, row = reader:next()
        if err ~= nil then
            logging.error('avro reader error %s', err)
            return nil
        end
        if not has_more then
            return nil
        else
            return row
        end
    end
end


function libs.list_operator_nodes(tree, node_filter_callback)
    if type(node_filter_callback) ~= 'function' then
        error('node_filter_callback should be a function')
        return nil
    end


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
        if node_filter_callback(index, fragment, instance, node) then
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


return libs
