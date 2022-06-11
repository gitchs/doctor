#!/usr/bin/env doctor
local logging = require'logging'
local iterators = require'iterators'

local function main()
    if #arg < 2 then
        logging.error('afilter.lua AVRO_FILENAME QUERY_ID')
        return
    end
    local filename = arg[1]
    local query_id = arg[2]

    local profile = nil
    for row in iterators.avro(filename) do
        if row.query_id == query_id then
            profile = row.profile
            break
        end
    end
    if profile == nil then
        logging.error('could not search query "%s" from "%s"', query_id, filename)
        return
    end

    print(profile)

end


main()
