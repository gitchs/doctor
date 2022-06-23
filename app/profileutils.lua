#!/usr/bin/env doctor
local io = require'io'
local re2 = require're2'
local strutils = require'strutils'
local missing = require'missing'
local impala = require'impala'
local logging = require'logging'

local libs = {}

local node2methods = {
    get_fragment = function (self)
        local re = [[(^Fragment F\d+$)]]
        local node = self
        while node ~= nil do
            if node.fragment ~= nil then
                self.fragment = node.fragment
                goto get_fragment__return
            end
            local name = node.name
            local m = re2.match(name, re)
            if #m > 0 then
                self.fragment = m[1]
                goto get_fragment__return
            end
            node = getmetatable(node).parent
        end
        ::get_fragment__return::
        return self.fragment
    end,
    get_host = function (self)
        -- Instance a046e786ae5a7c22:ad580b3500000013 (host=node01.akulaku.sa:20021)
        local re = [[^Instance [a-f:0-9]+ \(host=([^)]+)\)]]
        local node = self
        while node ~= nil do
            if node.host ~= nil then
                self.host = node.host
                goto get_host_return
            end
            local name = node.name
            local m = re2.match(name, re)
            if #m > 0 then
                self.host = m[1]
                goto get_host_return
            end
            node = getmetatable(node).parent
        end
        ::get_host_return::
        return self.host
    end,
    get_operator_id = function(self)
        local name = self.name
        local m = re2.match(self.name, [[\(id=(\d+)\)$]])
        if #m > 0 then
            return m[#m]
        end
        return nil
    end,
    normalize = function (self)
        local retval = {
            fragment = self:get_fragment(),
            host = self:get_host(),
        }
        for k, v in pairs(self) do
            if k ~= 'children' and k ~= 'num_children' then
                retval[k] = v
            end
        end
        setmetatable(retval, getmetatable(self))
        return retval
    end,
}

local function build_tree_implementation(parent, runtime_profile_tree, index)
    local node1 = runtime_profile_tree:node_at(index)
    local offset = 1
    local node2 = {
        index = index,
        name = node1:name(),
        num_children = node1:num_children(),
        counters = node1:counters(),
        children = nil,
        fragment = nil,
        host = nil,
    }
    local node2meta = {
        raw = node1,
        parent = parent,
        __index = node2methods,
    }
    setmetatable(node2, node2meta)
    if node2.num_children > 0 then
        node2.children = {}
        for i=1,node2.num_children do
            local delta, _ = build_tree_implementation(node2, runtime_profile_tree, index + offset)
            offset = offset + delta
        end
    end
    if parent ~= nil then
        table.insert(parent.children, node2)
    end
    return offset, node2
end

function libs.build_tree(runtime_profile_tree)
    local offset, tree2 = build_tree_implementation(nil, runtime_profile_tree, 1)
    assert(offset == runtime_profile_tree:nodes_length(), 'offset should equal runtime_profile_tree:nodes_length()')
    tree2.query_id = runtime_profile_tree:query_id()
    return tree2
end

function libs.instance_operators(instance)
    local index = 1
    local cit = nil
    return function ()
        if cit ~= nil then
            local retval = cit()
            if retval ~= nil then
                return retval
            end
            cit = nil
        end

        if instance.num_children == 0 or index > instance.num_children then
            return nil
        end

        while index <= instance.num_children do
            local child = instance.children[index]
            index = index + 1
            local m = re2.match(child.name, [[(\(id=\d+\)$)]])
            if #m > 0 then
                if child.num_children > 0 then
                    cit = libs.instance_operators(child)
                end
                return child:normalize()
            end
        end
        return nil
    end
end

function libs.group_operators(tree2)
    local execution_profile = tree2.children[tree2.num_children]
    if not strutils.startswith(execution_profile.name, 'Execution Profile') then
        return nil
    end
    local gops = {}
    for _, fragment in ipairs(execution_profile.children) do
        if not (strutils.startswith(fragment.name, 'Fragment F') or strutils.startswith(fragment.name, 'Coordinator Fragment F')) then
            goto continue
        end

        for _, instance in ipairs(fragment.children) do
            for operator in libs.instance_operators(instance) do
                local oid = operator:get_operator_id()
                if gops[oid] == nil then
                    gops[oid] = {}
                end
                table.insert(gops[oid], operator)
            end
        end
        ::continue::
    end
    return gops
end

function libs.parse_datetime(dt)
    local s = missing.strptime(dt:sub(1, 19), '%Y-%m-%d %H:%M:%S')
    local ms = tonumber(dt:sub(21))/1e6
    return s, ms
end

return libs
