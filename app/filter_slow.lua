#!/usr/bin/env doctor
os = require'os'
cjson = require'cjson'
impala = require'impala'
avro_wrapper = require'avro_wrapper'
is_slow_query = require'is_slow_query'


function main()
    local filename = arg[1]
    if filename == nil then
        print('no input file')
        os.exit(0)
    end
    local reader = avro_wrapper(filename)
    local rows = {}
    for row in reader do
        local ok, err, tree = impala.parse_profile(row.profile)
        local _, result = is_slow_query(tree)
        if result.is_slow then
            local summary = tree:node_at(2)
            local duration = tonumber(summary:info_strings("Duration(ms)"))
            local row = {
                duration=duration,
                profile=row.profile,
                query_id=query_id,
            }
            table.insert(rows, row)
        end
    end
    table.sort(rows,
        function(v1, v2)
            return v1.duration > v2.duration
        end)
    local output = io.open('slow_queries.txt', 'w')
    for _, row in ipairs(rows) do
        output:write(row.profile)
    end
    output:close()
end


main()

