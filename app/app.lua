#!/usr/bin/env doctor
io = require'io'
impala = require'impala'
cjson = require'cjson'
missing = require'missing'
strutils = require'strutils'


local FRAGMENT_INSTANCE_LIFECYCLE_TIMINGS_KEY = 'Fragment Instance Lifecycle Timings'


local is_slow_query = require'is_slow_query'
local hdfs_io = require'hdfs_io'

local analyze_chains = {
    is_slow_query,
    hdfs_io,
}


function print_usage()
    -- TODO: update usage message
    help = 'app.lua: \n' ..
'Usage:\n' ..
'TO BE CONTINUE'
    print(help)
end


function analyze_profile(b64_profile)
    local ok, err, tree = impala.parse_profile(b64_profile)
    if not ok then
        return ok, err, nil
    end
    local reports = {}
    for _, fn in ipairs(analyze_chains) do
        local report = fn(tree)
        table.insert(reports, report)
    end
    return true, nil, tree:query_id(), reports
end


function main()
    local filename = arg[1]
    if filename == nil then
        print_usage()
        return 1
    end
    local raw = io.open(filename, 'r'):read('*all')
    local ok, err, query_id, reports = analyze_profile(raw)
    if not ok then
        print(ok, err)
        return 1
    end
    print(query_id, cjson.encode(reports))
    return 0
end


os.exit(main() or 0)

