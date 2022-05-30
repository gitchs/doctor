#!/usr/bin/env lua
impala = require'impala'


function main()
    local filename = arg[1]
    if filename == nil then
        print('no input file')
        return
    end
    local b64_profile = io.open(filename, 'r'):read('*all')
    local ok, err, tree = impala.parse_profile(b64_profile)
    if not ok then
        print(ok, err)
        return
    end
    local profile = tree:debug()
    profile = profile:gsub('\\n', '\n')
    print(profile)
end


main()

