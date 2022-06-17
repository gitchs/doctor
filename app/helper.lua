#!/usr/bin/env doctor
local io = require'io'
local impala = require'impala'
local missing = require'missing'
local profileutils = require'profileutils'

local libs = {}

function libs.list_operators(filename)
    local fd = io.open(filename)
    assert(fd ~= nil)
    local raw = fd:read('*all')
    local ok, err, tree = impala.parse_profile(raw)
    assert(ok)

    local tree2 = profileutils.build_tree(tree)
    local gops = profileutils.group_operators(tree2)
    local retval = {}
    for oid, ops in pairs(gops) do
        retval[oid] = {}
        for _, op in ipairs(ops) do
            local row = {}
            for k, v in pairs(op) do
                if k == 'counters' then
                    for sk, sv in pairs(v) do
                        row[sk] = sv
                    end
                else
                    row[k] = v
                end
            end
            table.insert(retval[oid], row)
        end
    end
    return retval
end

function libs.fn(filename, oid)
    -- preprocess filename
    if missing.stat(filename).errno ~= nil then
        local filename2 = string.format('/Volumes/HKVSN/views/%s.txt', filename)
        if missing.stat(filename2).errno == nil then
            filename = filename2
        else
            assert(false, string.format([[file both "%s" and "%s" not exists]], filename, filename2))
        end
    end
    local ops = libs.list_operators(filename)
    if oid ~= nil then
        oid = tostring(oid)
        return cjson.encode(ops[oid])
    end
    return ops
end

return libs
