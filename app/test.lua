#!/usr/bin/env lua
hdfsutils = require'hdfsutils'

filename = 'profiles/9f41dd368a419ced:822a9ba300000000.txt'
raw = io.open(filename):read('*all')
ok, err, tree = impala.parse_profile(raw)
assert(ok)

counters = hdfsutils.list_hdfs_node_counters(tree)


return counters
