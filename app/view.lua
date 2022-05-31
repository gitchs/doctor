#!/usr/bin/env doctor
missing = require'missing'
impala = require'impala'


function main()
    local b64_profile = nil
    if missing.isatty(0) then
        local filename = arg[1]
        if filename == nil then
            print('no input file')
            return
        end
        b64_profile = io.open(filename, 'r'):read('*all')
    else
        b64_profile = io.stdin:read('*all')
    end
    if b64_profile == nil then
        print('b64_profile input is empty')
        return -1
    end
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

