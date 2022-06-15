#!/usr/bin/env doctor
local impala = require'impala'
local logging = require'logging'
local strutils = require'strutils'
local profileutils = require'profileutils'
local strategies = require'strategies'

local function test_profile(raw_profile)
    local retval = {}
    local ok, err, tree = impala.parse_profile(raw_profile)
    assert(ok)
    local tree2 = profileutils.build_tree(tree)
    local r = strategies.is_slow(tree2)
    retval.is_slow = r
    if r < 0 then
        return retval
    end
    retval.skew_ops = strategies.operator_skew_detection(tree2)
    return retval
end

local function main()
    local filename = arg[1]
    if filename == nil then
        logging.error('no input file')
        return
    end
    local fd = io.open(filename, 'r')
    assert(fd ~= nil, 'failed to open file')
    local raw = fd:read('*all')
    fd:close()
    local result = test_profile(raw)
    print(cjson.encode(result))
end

main()
