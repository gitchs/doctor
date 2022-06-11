#!/usr/bin/env doctor
logging = require'logging'
iterators = require'iterators'
hdfsutils = require'hdfsutils'

function main()
    if #arg < 1 then
        logging.error('afilter.lua AVRO_FILENAME QUERY_ID')
        return
    end
    local filename = arg[1]
    local fd = io.open(filename)
    local profile = fd:read('*all')

    local ok, err, tree = impala.parse_profile(profile)
    assert(ok, 'failed to parse profile')

    local counters = hdfsutils.list_hdfs_node_counters(tree)
    print(cjson.encode(counters))

end


main()
