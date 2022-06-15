#!/usr/bin/env doctor
local logging = require'logging'
local avro = require'avro'
local re2 = require're2'

local libs = {}
function libs.avro(filename)
    local reader = avro.open(filename)
    assert(reader ~= nil, string.format('failed to open avro file "%s"', filename))
    return function()
        local has_more, err, row = reader:next()
        if err ~= nil then
            logging.error('avro reader error %s', err)
            return nil
        end
        if not has_more then
            return nil
        else
            return row
        end
    end
end

return libs
