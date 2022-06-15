#!/usr/bin/env doctor
local impala = require'impala'
local logging = require'logging'


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
    for i=1,tree:nodes_length() do
        local node = tree:node_at(i)
        local row = {
            index = i,
            num_children = node:num_children(),
            name = node:name(),
            counters = node:counters(),
            event_sequences = node:event_sequences(),
        }
        print(cjson.encode(row))
    end
end


main()
