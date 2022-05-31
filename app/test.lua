#!/usr/bin/env doctor
io = require'io'
cjson = require'cjson'
impala = require'impala'
hdfsutils = require'hdfsutils'
local sqlite3_env = require'luasql.sqlite3'.sqlite3()
local db = sqlite3_env:connect('/Users/tinyproxy/Downloads/profile2/profile.db')
local cursor = db:execute[[
    select profile from profile
]]


function c2s(counter)
    local retval = ''
    for k, v in pairs(counter) do
        retval = retval .. string.format('%40s = %s\n', k, v)
    end
    return retval
end

m = {
    __tostring = c2s,
}


function main()
    for b64_profile in function() local profile = cursor:fetch(); return profile;end do
        local ok, err, tree = impala.parse_profile(b64_profile)
        local counters = hdfsutils.list_hdfs_node_counters(tree)
        local summary = tree:node_at(2)
        local query_id = tree:query_id()
        local duration = tonumber(summary:info_strings('Duration(ms)'))
        for _, counter in ipairs(counters) do
            counter.query_id = query_id
            counter.duration = duration
            io.stdout:write(cjson.encode(counter) .. '\n')
        end
    end
end


main()

