#!/usr/bin/env doctor
io = require'io'
impala = require'impala'
cjson = require'cjson'
missing = require'missing'


local FRAGMENT_INSTANCE_LIFECYCLE_TIMINGS_KEY = 'Fragment Instance Lifecycle Timings'

local strutils = {}

function strutils.startswith(s, prefix)
    if #s < #prefix then
        return false
    end
    return s:sub(1, #prefix) == prefix
end


local profileutils = {}
function profileutils.is_fragment(name)
    if not name then
        return false
    end
    return strutils.startswith(name, 'Fragment F') or strutils.startswith(name, 'Coordinator Fragment F')
end


function build_tree(profile_tree, index, parent_index)
    local offset = 1
    local profile_node = profile_tree:node_at(index)
    local root = {
        index = index,
        parent_index = parent_index,
        num_children = profile_node:num_children(),
        name = profile_node:name(),
        counters = profile_node:counters(),
        children = nil,
    }

    if root.num_children > 0 then
        root.children = {}
        for i = 1, root.num_children do
            local delta, child = build_tree(profile_tree, index+offset, offset)
            table.insert(root.children, child)
            offset = offset + delta
        end
    end

    local meta = {
        profile_node = profile_node,
    }
    setmetatable(root, meta)
    return offset, root
end


function main()

    local filename = arg[1]
    local raw = io.open(filename, 'r'):read('*all')
    local ok, err, profile_tree = impala.parse_profile(raw)
    if not ok then
        print(ok, err)
        return 1
    end
    local rows = {}

    local offset, tree = build_tree(profile_tree, 1, nil)
    assert(offset == profile_tree:nodes_length())
    ep_node = tree.children[3]
    assert(strutils.startswith(ep_node.name, 'Execution Profile'))
    for _, fragment in ipairs(ep_node.children) do
        if profileutils.is_fragment(fragment.name) then
            for _, instance in ipairs(fragment.children) do
                local imeta = getmetatable(instance)
                local instance_counters = imeta.profile_node:counters()
                instance_counters['type'] = 'instance'
                instance_counters['fragment'] = fragment.name
                instance_counters['instance'] = instance.name
                -- table.insert(rows, instance_counters)

                for _, instance_child in ipairs(instance.children) do
                    if instance_child.name == FRAGMENT_INSTANCE_LIFECYCLE_TIMINGS_KEY then
                        local meta = getmetatable(instance_child)
                        local row = meta.profile_node:counters()
                        row['type'] = 'lifecycle'
                        row['fragment'] = fragment.name
                        row['instance'] = instance.name
                        table.insert(rows, row)
                    end
                end
            end
        end
    end
    -- print(cjson.encode(rows))
end


print(cjson.encode(missing.getrusage(missing.RUSAGE_SELF)))
main()
print(cjson.encode(missing.getrusage(missing.RUSAGE_SELF)))

