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
    local unexpected_remote_fingerprint = 'of data across network that was expected to be local'
    for b64_profile in function() local profile = cursor:fetch(); return profile;end do
        local ok, err, tree = impala.parse_profile(b64_profile)
        local summary = tree:node_at(2)
        local query_type = summary:info_strings('Query Type')
        if query_type ~= 'QUERY' and query_type ~= 'DML' then
            goto continue
        end
        local counters = hdfsutils.list_hdfs_node_counters(tree)
        local iserror = summary:info_strings('Errors')
        local unexpected_remote = false
        if iserror ~= nil and iserror:match(unexpected_remote_fingerprint) ~= nil then
            unexpected_remote = true
        end
        local query_id = tree:query_id()
        local duration = tonumber(summary:info_strings('Duration(ms)'))
        local duration_min = duration/60000
        local query_state = summary:info_strings('Query State')
        local session_type = summary:info_strings('Session Type')
        if session_type ~= 'BEESWAX' then
            for _, counter in ipairs(counters) do
                counter.query_type = query_type
                counter.session_type = session_type
                counter.query_state = query_state
                counter.query_id = query_id
                counter.duration = duration
                counter.duration_min = duration_min
                counter.unexpected_remote = unexpected_remote
                io.stdout:write(cjson.encode(counter) .. '\n')
            end
        end
        ::continue::
    end
end


main()

