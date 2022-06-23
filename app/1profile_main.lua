#!/usr/bin/env doctor
local impala = require'impala'
local logging = require'logging'
local strategies = require'strategies'
local cjson = require'cjson'

local function main()
    local filename = arg[1]
    if filename == nil then
        logging.error('no input file')
        return
    end
    local fd = io.open(filename, 'r')
    assert(fd ~= nil, 'failed to open file')
    local raw = fd:read('*all')
    fd:close()
    local ok, err, tree = impala.parse_profile(raw)
    local result = strategies.test_profile(tree)
    print(cjson.encode(result))
end

main()
