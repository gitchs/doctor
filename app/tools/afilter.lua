#!/usr/bin/env doctor
local logging = require'logging'
local iterators = require'iterators'

local function main()
    if #arg < 2 then
        logging.error('afilter.lua AVRO_FILENAME QUERY_ID')
        return
    end
    local query_id = arg[1]

    local profile = nil
    for findex = 2, #arg do
        local filename = arg[findex]
        for row in iterators.avro(filename) do
            if row.query_id == query_id then
                profile = row.profile
                break
            end
        end
        if profile ~= nil then
            print(profile)
            return
        end
    end
    logging.error('could not search query "%s" from "%s"', query_id, filename)


end


main()
