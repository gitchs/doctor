#!/usr/bin/env doctor
avro = require'avro'

local libs = function(filename)
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


return libs
