#!/usr/bin/env doctor
local string = require'string'
local logging = require'logging'
local missing = require'missing'
local carray = require'carray'

local libs = {}
libs.Parser = {}
libs.Parser['__index'] = libs.Parser
libs.Parser['__name'] = 'optlib.Parser'

function libs.Parser:push_argument(v)
    local ok = self.values:set(self.used+1, v)
    if ok then
        self.used = self.used + 1
    end
    return ok
end

function libs.Parser:feed(args, optstr)
    for _, arg in ipairs(args) do
        self:push_argument(arg)
    end
    return self:getopt(optstr)
end


function libs.Parser:getopt(optstr)
    local done = false
    missing.optreset();
    return function()
        if not done then
            local index, code, value = missing.getopt(self.used, self.values:raw(), optstr)
            if code == -1 then
                if index >= self.used then
                    done = true
                    return nil, nil, nil
                end
                missing.inc_opt_index()
                return false, -1, self.values:at(index+1)
            end
            code = string.char(code)
            return true, code, value
        end
        return nil
    end
end

function libs.Parser.new(o, size)
    o = o or {
        used = 0,
        values = carray.StringArray(size or 128),
    }
    setmetatable(o, libs.Parser);
    return o
end



return libs

