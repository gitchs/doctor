#!/usr/bin/env doctor
io = require'io'
impala = require'impala'
cjson = require'cjson'
avro = require'avro'
missing = require'missing'
strutils = require'strutils'
logutils = require'logutils'

local sqlite3 = require'luasql.sqlite3'.sqlite3


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
    logutils.info(help)
end


function analyze_profile(b64_profile)
    local ok, err, tree = impala.parse_profile(b64_profile)
    if not ok then
        return ok, err, nil
    end
    local reports = {
        query_id = tree:query_id(),
    }
    for _, fn in ipairs(analyze_chains) do
        local case_name, report = fn(tree)
        reports[case_name] = report
    end
    return true, nil, reports
end


function avro_wrapper(filename)
    local reader = avro.open(filename)
    return function()
        local has_more, row = reader:next()
        if not has_more then
            return nil
        else
            return row
        end
    end
end

function sqlite3_it(filename)
    local env = sqlite3()
    local db = env:connect(filename)
    local cursor = db:execute[[select profile from profile]]
    return function()
        local profile = cursor:fetch()
        return profile
    end
end


function main()
    local filename = arg[1]
    if filename == nil then
        print_usage()
        return 1
    end
    -- for row in avro_wrapper(filename) do
    for profile in sqlite3_it(filename) do
        local ok, err, reports = analyze_profile(profile)
        if not ok then
            logutils.info(ok, err)
            return 1
        end
        print(cjson.encode(reports))
    end
    return 0
end


os.exit(main() or 0)

